#include "Networking.h"
#include "PacketInternal.h"
#include "State.h"
#include "Shared.h" // Logger adn EventApi instance
#include "Services/Mumble/Reader.h"
#include "Services/Settings/Settings.h"
#include "Events/EventHandler.h"
#include "Consts.h"
#include <WinSock2.h>
#include <WS2tcpip.h> // inet_pton
#include <thread>

#define _LOG(lvl, fmt, ...) Logger->LogMessage(ELogLevel:: lvl, "Networking", fmt, __VA_ARGS__)

namespace Networking
{
	void ReportSystemError(char const* fmt, int error);
	u32 CalculatePacketChecksum(Packet* packet);
	void HandleInternalPacket(Packet* packet);
	void EnterReceiveLoop();
	void UpdateUserId(void* ev_args);

	SOCKET Socket = INVALID_SOCKET;
	sockaddr ServerEndpoint;
	std::map<AddonSignature, PacketHandler*> Handlers;
	InitState State = InitState::_UNKNOWN;
	UserId MyUserId;


	bool SetServerAddress(char const* host, u16 port)
	{
		addrinfo hints {};
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;

		addrinfo* addresses;
		int error = getaddrinfo(host, 0, &hints, &addresses);
		if(error) {
			ReportSystemError("Failed to resolve server hostname:\n%d: %s", WSAGetLastError());
			return false;
		}

		char addr_buffer[INET6_ADDRSTRLEN];
		bool acceptedOne = false;
		for(auto current = addresses; current; current = current->ai_next) {
			inet_ntop(current->ai_family, &((sockaddr_in*)current->ai_addr)->sin_addr, addr_buffer, sizeof(addr_buffer));
			_LOG(TRACE, "DNS: pf: %d, addr: %s.", current->ai_family, addr_buffer);

			if((current->ai_family != PF_INET && current->ai_family != PF_INET6)) continue;

			ServerEndpoint = *addresses->ai_addr;
			//NOTE(Rennorb): port is in the same space regardles of address kind
			((sockaddr_in*)&ServerEndpoint)->sin_port = htons(port);

			acceptedOne = true;
			break;
		}
		

		if(acceptedOne) {
			//char addr_buffer[INET6_ADDRSTRLEN];
			inet_ntop(((sockaddr_in*)&ServerEndpoint)->sin_family, &((sockaddr_in*)&ServerEndpoint)->sin_addr, addr_buffer, sizeof(addr_buffer));
			_LOG(TRACE, "Determined server to be (%s) %s, port: %u.", host, addr_buffer, port);
		}
		else {
			_LOG(WARNING, "Failed to resolve acceptable address for host '%s'.", host);
		}

		freeaddrinfo(addresses);
		return acceptedOne;
	}

	u16 ParseSettings(nlohmann::json& settings)
	{
		auto& setting = Settings::Settings[OPT_NETWORKINGTRIPPLE];
		auto settingsTripple = setting.get<std::string>();

		unsigned int listenPort, serverPort;
		char serverHost[128];
		if(_snscanf(settingsTripple.data(), settingsTripple.length(), " %u %127s %u ", &listenPort, &serverHost, &serverPort) != 3) {
			_LOG(WARNING, "Networking Tripple settings was set to '%*s', but failed to parse as '%%u %%127s %%u'. Make sure the setting follows this format, with the fields being 'listen port', 'server host' and 'server port' in that order. Using fallback values.", settingsTripple);
			return 0;
		}

		if(listenPort <= 1024 || listenPort > 0xffff) {
			_LOG(WARNING, "Networking Tripple setting was set, but 'listen port' (first part) %d was out of range; 1024 < port < 65535 must hold. Using fallback values.", listenPort);
			return 0;
		}

		if(serverPort <= 1024 || serverPort > 0xffff) {
			_LOG(WARNING, "Networking Tripple setting was set, but 'server port' (third part) %d was out of range; 1024 < port < 65535 must hold. Using fallback values.", serverPort);
			return 0;
		}

		if(!SetServerAddress(serverHost, serverPort)) {
			_LOG(WARNING, "Networking Tripple setting was set, but 'server host' (second part) '%s' failed to resolve to any address (see warning above this one). Using fallback values.", serverHost);
			return 0;
		}

		return listenPort;
	}

	void Init()
	{
		if(State >= InitState::Initializing) return;

		WSADATA wsa;
		int error = WSAStartup(MAKEWORD(1, 0), &wsa);
		if(error) {
			ReportSystemError("WSAStartup failed:\n%d: %s", error);
			_LOG(CRITICAL, "Networking will be unavailable.");
			return;
		}



		u16 listenPort = 0;
		{
			auto& setting = Settings::Settings[OPT_NETWORKINGTRIPPLE];
			if(!setting.is_null()) {
				listenPort = ParseSettings(setting);
			}

			if(listenPort == 0) {
				listenPort = 1338;
				if(!SetServerAddress("nexus.sacul.dev", 1337)) {
					_LOG(CRITICAL, "Networking will be unavailable.");
					return;
				}
			}
			//TODO find correct network interface for sending
		}


		//NOTE(Rennorb): We don't care about closing the socket. Its going to run for the whole lifetime of the process.
		SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(sock == INVALID_SOCKET) {
			ReportSystemError("Failed to create socket:\n%d: %s", WSAGetLastError());
			_LOG(CRITICAL, "Networking will be unavailable.");
			return;
		}

		Socket = sock;

		

		sockaddr_in bind_addr {};
		bind_addr.sin_family = AF_INET;
		bind_addr.sin_addr.s_addr = INADDR_ANY;
		bind_addr.sin_port = htons(listenPort);
		error = bind(Socket, (SOCKADDR*)&bind_addr, sizeof(bind_addr));
		if(error) {
			ReportSystemError("Failed to bind socket:\n%d: %s", WSAGetLastError());
			_LOG(CRITICAL, "Networking will be unavailable.");
			return;
		}

		auto rcvThread = std::thread(EnterReceiveLoop);
		SetThreadDescription(rcvThread.native_handle(), L"Packet receive and dispatch");
		rcvThread.detach();

		if(true) {
			_LOG(INFO, "Using scuffed user id for now.");
			
			State = InitState::WaitingForUserId;

			EventApi->Subscribe(EV_MUMBLE_IDENTITY_UPDATED, UpdateUserId);
		}
		else {
			//TODO(Rennorb) @completeness: obtain stable id, visible by others in the squad, or obtain a stable squad id
			State = InitState::ReadyForSession;
		}
	}

	void UpdateUserId(void* ev_args)
	{
		auto identity = (Mumble::Identity*)ev_args;

		if(!identity->Name[0]) {
			_LOG(TRACE, "Discarding UserId update event with empty character name");
			return;
		}

		u32* namedData = (u32*)identity->Name;
		u32* end = namedData + sizeof(identity->Name) / 4;
		UserId newUserId = 0;
		while(namedData < end) {
			newUserId = newUserId ^ *namedData++;
		}

		if(MyUserId == newUserId) return;

		switch(State) {
			case InitState::WaitingForUserId: {
				State = InitState::ReadyForSession;
				_LOG(TRACE, "Initial UserId established");

				//TODO(Rennorb) @adhoc @debug
				Internal::JoinSession joinPacket {};
				joinPacket.Me = newUserId;
				joinPacket.SessionUser = 0;
				SendInternalPacket((Packet*)&joinPacket);

				State = InitState::SessionEstablished;
			} break;

			case InitState::SessionEstablished: {
				Internal::ChangeUserId changePacket {};
				changePacket.OldId = MyUserId;
				changePacket.NewId = newUserId;
				SendInternalPacket((Packet*)&changePacket);

				_LOG(TRACE, "Updated UserId");
			} break;
		}

		MyUserId = newUserId;
	}




	bool PrepareAndBroadcastPacket(Packet* packet)
	{
		if(State != InitState::SessionEstablished) return false;

		//TODO(Rennorb): think about the addon id
		if(packet->Header.TargetAddonId == 0) {
			_LOG(WARNING, "Header.TargetAddonId is 0. Auto ids are not currently supported, please set the id manually. Packet will be discarded.");
			return false;
		}
		if(packet->Header.LengthInU32s < 3) {
			_LOG(WARNING, "Header.Length is %d, which less than the minimum plausible length (3). Packet will be discarded.", packet->Header.LengthInU32s);
			return false;
		}

		// packet checksum is the last u32 in the packet, packet has to be aligned
		((u32*)packet)[packet->Header.LengthInU32s - 1] = CalculatePacketChecksum(packet);

		int result = sendto(
			Socket,
			(char*)packet, packet->Header.LengthInU32s * sizeof(u32),
			0,
			(SOCKADDR*)&ServerEndpoint, sizeof(ServerEndpoint)
		);
		if(State::IsDeveloperMode && result == SOCKET_ERROR) {
			ReportSystemError("Failed to send packet:\n%08x: %s", WSAGetLastError());
		}
		return result == SOCKET_ERROR;
	}

	void SendInternalPacket(Packet* packet)
	{
		if(State < InitState::ReadyForSession) return;

		packet->Header.TargetAddonId = 0;
		if(packet->Header.LengthInU32s < 3) {
			_LOG(WARNING, "Header.Length is %d, which less than the minimum plausible length (3). Packet will be discarded.", packet->Header.LengthInU32s);
			return;
		}
		// packet checksum is the last u32 in the packet, packet has to be aligned
		((u32*)packet)[packet->Header.LengthInU32s - 1] = CalculatePacketChecksum(packet);

		int result = sendto(
			Socket,
			(char*)packet, packet->Header.LengthInU32s * sizeof(u32),
			0,
			(SOCKADDR*)&ServerEndpoint, sizeof(ServerEndpoint)
		);
		if(State::IsDeveloperMode && result == SOCKET_ERROR) {
			ReportSystemError("Failed to send packet:\n%08x: %s", WSAGetLastError());
		}
	}

	void JoinSession()
	{
		//TODO
	}

	void LeaveSession() 
	{
		if(State < InitState::SessionEstablished) return;
		_LOG(TRACE, "Disconnecting from established session");

		Internal::LeaveSession leavePacket;
		leavePacket.Me = MyUserId;
		SendInternalPacket((Packet*)&leavePacket);

		State = InitState::ReadyForSession;
	}


	void EnterReceiveLoop()
	{
		char buffer[1024]; //TODO size
		while(true) {
			sockaddr_in from {};
			int fromSize = sizeof(from);
			int rcvdLength = recvfrom(Socket, buffer, sizeof(buffer), 0, (SOCKADDR*)&from, &fromSize);
			if(rcvdLength == SOCKET_ERROR) {
				ReportSystemError("Failed to read from socket:\n%d: %s", WSAGetLastError());
				continue;
			}

			//NOTE(Rennorb): Only trace these messages because incoming packets might just contain junk.
			// The packets that are sent are already checked with higher log levels.
			int lengthInU32s = rcvdLength / 4;
			const int MIN_PACKET_SIZE = (sizeof(PacketHeader) + 3) / 4 + (sizeof(PacketChecksum) + 3) / 4;
			if(rcvdLength % 4 != 0 || lengthInU32s < MIN_PACKET_SIZE) {
				_LOG(TRACE, "Packet length is %d Bytes, not evenly divisible by 4 or less than the minimum plausible length (%d Bytes). Packet will be discarded.", rcvdLength, MIN_PACKET_SIZE * 4);
				continue;
			}

			Packet* packet = (Packet*)buffer;

			if(packet->Header.LengthInU32s != lengthInU32s) {
				_LOG(TRACE, "Actual packet length (%d) does not match length in packet header (%d).", packet->Header.LengthInU32s, lengthInU32s);
				continue;
			}

			PacketChecksum packetChecksum = ((u32*)packet)[packet->Header.LengthInU32s - 1];
			PacketChecksum calculatedChecksum = CalculatePacketChecksum(packet);
			if(packetChecksum != calculatedChecksum) {
				_LOG(TRACE, "Actual packet checksum (%08x) does not match checksum in packet (%08x) trailer.", calculatedChecksum, packetChecksum);
				continue;
			}

			if(packet->Header.TargetAddonId == Internal::INTERNAL_PACKET) {
				HandleInternalPacket(packet);
				continue;
			}

			auto it = Handlers.find(packet->Header.TargetAddonId);
			if(it == Handlers.end()) {
				_LOG(TRACE, "Received packet for which the addon (%08x) is not installed.", packet->Header.TargetAddonId);
				continue;
			}

			it->second(packet);
		}
	}

	void HandleInternalPacket(Packet* packet)
	{
		if(packet->Header.LengthInU32s < Internal::MIN_PACKET_SIZE) {
			_LOG(TRACE, "Received internal packet that's smaller (%d) than the minimum required size (%d). Packet will be discarded.", packet->Header.LengthInU32s, Internal::MIN_PACKET_SIZE);
			return;
		}
		auto header = (Internal::PacketHeader*)packet;
		switch(header->Type) {
			case Internal::PacketType::SessionCreated:
				if(packet->Header.LengthInU32s * 4 == sizeof(Internal::SessionCreated))
					_LOG(TRACE, "Joined session %08x.", ((Internal::SessionCreated*)packet)->Session);
				else
					_LOG(TRACE, "Received SessionCreated packet that is not the correct size (%d instead of the expected %d).", packet->Header.LengthInU32s, sizeof(Internal::SessionCreated));
				break;

			default:
				_LOG(TRACE, "Received unknown internal (type: %d, len: %d Bytes). Packet will be discarded.", header->Type, header->BaseHeader.LengthInU32s * 4);
		}
	}



	//TODO(Rennorb) @cleanup
	PacketHandler* GetPacketHandlerFromApi(int version, AddonAPI* api)
	{
		switch(version) {
			case  4: return ((AddonAPI4*)api)->HandleIncomingPacket;
			default: return 0;
		}
	}




	u32 CalculatePacketChecksum(Packet* packet)
	{
		u32* start = ((u32*)packet);
		u32* end = start + packet->Header.LengthInU32s - 1;
		u32 accumulator = 0;
		while(start < end) {
			accumulator += *start++;
		}

		return accumulator;
	}

	/// `fmt` gets two arguments passed, the numeric error code, and the string representation.
	/// The error will be logged as `WARNING`
	void ReportSystemError(char const* fmt, int error)
	{
		char* system_error = 0;
		FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
			0,
			error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&system_error,
			0, 0
		);
		if(!system_error) system_error = (char*)"Could not obtain error message.";
		_LOG(WARNING, fmt, error, system_error);
		LocalFree(system_error);
	}
}

#undef _LOG
