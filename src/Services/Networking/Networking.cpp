//TODO(Rennorb) @bug: When you join into the game already in a squad alone with yourself, join / leave packets will be inverted from what they should be.

#include "Networking.h"
#include "PacketInternal.h"
#include "State.h"
#include "Shared.h" // Logger and EventApi instance
#include "Services/Mumble/Reader.h"
#include "Services/Settings/Settings.h"
#include "Services/AddonShare/AddonShare.h"
#include "Events/EventHandler.h"
#include "Consts.h"
#include "SquadEvents.h"
#include <WinSock2.h>
#include <WS2tcpip.h> // inet_pton
#include <stdint.h>
#include <thread>

#define _LOG(lvl, fmt, ...) Logger->LogMessage(ELogLevel:: lvl, "Networking", fmt, __VA_ARGS__)

namespace Networking
{
	typedef uint32_t u32;
	typedef uint16_t u16;
	
	ModuleState State = ModuleState::_UNKNOWN;
	bool AddonLoaded = false;
	std::map<AddonSignature, PacketHandler*> Handlers;
	UserId MyUserId = INVALID_USER;


	void ReportSystemError(char const* fmt, int error);
	u32 CalculatePacketChecksum(Packet* packet);
	void HandleInternalPacket(Packet* packet);
	void EnterReceiveLoop();
	void SendInternalPacket(Packet* packet);
	void IJoinedSquad(SquadEvents::Squad* squad);
	void ILeftSquad(SquadEvents::LeaveReason reason);
	void AddonStateResponse(SquadEvents::Squad* squad);
	void SomeoneLeftSquad(SquadEvents::SquadMember* member);

	SOCKET Socket = INVALID_SOCKET;
	sockaddr ServerEndpoint;
	SessionId MySessionId = INVALID_SESSION;
	SquadEvents::Squad* CurrentSquad;


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

	bool ParseSettings(nlohmann::json& settings, u16* outlistenPort)
	{
		auto& setting = Settings::Settings[OPT_NETWORKINGTRIPPLE];
		auto settingsTripple = setting.get<std::string>();

		unsigned int listenPort, serverPort;
		char serverHost[128];
		if(_snscanf(settingsTripple.data(), settingsTripple.length(), " %u %127s %u ", &listenPort, &serverHost, &serverPort) != 3) {
			_LOG(WARNING, "Networking Tripple settings was set to '%*s', but failed to parse as '%%u %%127s %%u'. Make sure the setting follows this format, with the fields being 'listen port', 'server host' and 'server port' in that order. Using fallback values.", settingsTripple);
			return false;
		}

		if(listenPort <= 1024 || listenPort > 0xffff) {
			_LOG(WARNING, "Networking Tripple setting was set, but 'listen port' (first part) %d was out of range; 1024 < port < 65535 must hold. Using any available local port.", listenPort);
			*outlistenPort = 0;
		}
		else {
			*outlistenPort = (u16)listenPort;
		}


		if(serverPort <= 1024 || serverPort > 0xffff) {
			_LOG(WARNING, "Networking Tripple setting was set, but 'server port' (third part) %d was out of range; 1024 < port < 65535 must hold. Using fallback values.", serverPort);
			return false;
		}

		if(!SetServerAddress(serverHost, serverPort)) {
			_LOG(WARNING, "Networking Tripple setting was set, but 'server host' (second part) '%s' failed to resolve to any address (see warning above this one). Using fallback values.", serverHost);
			return false;
		}

		return true;
	}

	void Init()
	{
		if(State >= ModuleState::Initializing) return;

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
			bool useFallback = true;
			if(!setting.is_null()) {
				useFallback = !ParseSettings(setting, &listenPort);
			}

			if(useFallback) {
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

		EventApi->Subscribe(SquadEvents::EV_SQUAD_I_JOINED, (EVENT_CONSUME)IJoinedSquad);
		EventApi->Subscribe(SquadEvents::EV_SQUAD_I_LEFT, (EVENT_CONSUME)ILeftSquad);
		EventApi->Subscribe(SquadEvents::EV_SQUAD_SOMEONE_LEFT, (EVENT_CONSUME)SomeoneLeftSquad);

		EventApi->Subscribe(SquadEvents::EV_SQUAD_RESPONSE_STATE, (EVENT_CONSUME)AddonStateResponse);
		//TODO(Rennorb) @cleanup: Would be nice to receive the sender signature so i can send a response back to only this target here.
		EventApi->Raise(SIG_NETWORKING, SquadEvents::EV_SQUAD_REQUEST_STATE);

		KeybindApi->Register("disconnect_session", EKeybindHandlerType::DownOnly, LeaveSession, "Ctrl+Shift+D");

		State = ModuleState::WaitingForJoin;
	}

	void AddonStateResponse(SquadEvents::Squad* squad) {
		if(squad->id != UUID {0})  IJoinedSquad(squad);
	}

	void IJoinedSquad(SquadEvents::Squad* squad)
	{
		CurrentSquad = squad;
		auto newUserId = squad->members[squad->my_index].id;

		if(MyUserId != newUserId || MySessionId == INVALID_SESSION) {
			_LOG(TRACE, "Joining new session");

			Internal::JoinSession joinPacket {};
			joinPacket.Me      = newUserId;
			joinPacket.Session = squad->id;

			State = ModuleState::WaitingForServerResponse;
			
			SendInternalPacket((Packet*)&joinPacket);
		}

		MyUserId = newUserId;
	}

	void ILeftSquad(SquadEvents::LeaveReason reason)
	{
		LeaveSession();
	}

	void SomeoneLeftSquad(SquadEvents::SquadMember* member)
	{
		AddonShare::RemoveSpecificMember(member->id);
	}



	bool PrepareAndBroadcastPacket(Packet* packet)
	{
		if(State != ModuleState::SessionEstablished) return false;

		//TODO(Rennorb): think about the addon id
		if(packet->Header.TargetAddonId == Internal::INTERNAL_PACKET) {
			_LOG(WARNING, "Header.TargetAddonId is 0. Auto ids are not currently supported, please set the id manually. Packet will be discarded.");
			return false;
		}
		auto minSize = 3;
		if(packet->Header.Flags & PacketFlags::ContainsSource) minSize += sizeof(UserId) / 4;
		if(packet->Header.Flags & PacketFlags::ContainsTarget) minSize += sizeof(UserId) / 4;
		if(packet->Header.LengthInU32s < (uint8_t)minSize) {
			_LOG(WARNING, "Header.Length is %d, which less than the minimum plausible length (%d) for the given flags. Packet will be discarded.", packet->Header.LengthInU32s, minSize);
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
		if(State < ModuleState::WaitingForServerResponse) return;

		packet->Header.TargetAddonId = Internal::INTERNAL_PACKET;
		if(packet->Header.LengthInU32s < 3) {
			_LOG(WARNING, "Header.Length is %d, which less than the minimum plausible length (3). Packet will be discarded.", packet->Header.LengthInU32s);
			return;
		}
		if(packet->Header.Flags) {
			_LOG(WARNING, "Header.Flags (was %02x) are ignored for internals packets.", packet->Header.Flags);
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

	void LeaveSession()
	{
		if(State < ModuleState::SessionEstablished || MyUserId == INVALID_USER) return;
		_LOG(TRACE, "Disconnecting from established session");

		Internal::LeaveSession leavePacket;
		leavePacket.Me = MyUserId;
		SendInternalPacket((Packet*)&leavePacket);

		CurrentSquad = 0;
		MySessionId = INVALID_SESSION;
		State = ModuleState::WaitingForJoin;

		AddonShare::ClearOthers();
	}


	void EnterReceiveLoop()
	{
		char buffer[1 << (sizeof(PacketHeader::LengthInU32s) * 8)];
		while(true) {
			int rcvdLength = recv(Socket, buffer, sizeof(buffer), 0);
			if(rcvdLength == SOCKET_ERROR) {
				ReportSystemError("Failed to read from socket:\n%d: %s", WSAGetLastError());
				continue;
			}

			Packet* packet = (Packet*)buffer;

			//NOTE(Rennorb): Only trace these messages because incoming packets might just contain junk.
			// The packets that are sent are already checked with higher log levels.
			int minPacketLength = (sizeof(PacketHeader) + 3) / 4 + (sizeof(PacketChecksum) + 3) / 4;
			if(rcvdLength >= minPacketLength) {
				if(packet->Header.Flags & PacketFlags::ContainsTarget) minPacketLength += sizeof(UserId);
				if(packet->Header.Flags & PacketFlags::ContainsSource) minPacketLength += sizeof(UserId);
			}
			if(rcvdLength % 4 != 0 || rcvdLength < minPacketLength) {
				_LOG(TRACE, "Packet length is %d Bytes, not evenly divisible by 4 or less than the minimum plausible length (%d Bytes). Packet will be discarded.", rcvdLength, minPacketLength);
				continue;
			}

			int lengthInU32s = rcvdLength / 4;
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
				_LOG(TRACE, "Received packet for which the addon (%08x) is not installed or did not register a handler.", packet->Header.TargetAddonId);
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
				if(packet->Header.LengthInU32s * 4 == sizeof(Internal::SessionCreated)) {
					if(AddonLoaded) { // one more safety just in case the networking addon got unloaded between request and response
						State = ModuleState::SessionEstablished;
						MySessionId = ((Internal::SessionCreated*)packet)->Session;
						_LOG(TRACE, "Joined session %08x%08x%08x%08x.", SPLIT_SESSION_ID(MySessionId));

						//NOTE(Rennorb): Spawn this in a separate thread as addons will likely try to immediately fire packets ones this goes out,
						// and we dont want to block the receiver for responses.
						std::thread([]() { EventApi->Raise(EV_NETWORKING_READY); }).detach();

						AddonShare::TransmitAddonStates(0);
						// if we joined a squad with other members, ask them for their addons
						if(CurrentSquad && CurrentSquad->member_count > 1) {
							AddonShare::RequestAddons();
						}
					}
				}
				else
					_LOG(TRACE, "Received SessionCreated packet that is not the correct size (%d instead of the expected %d).", packet->Header.LengthInU32s, sizeof(Internal::SessionCreated));
				break;

			default:
				_LOG(TRACE, FORMAT_DISCARD_MESSAGE(packet));
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


	void ResetHandlerPtr(int version, AddonAPI* api)
	{
		switch(version) {
			case 4: ((AddonAPI4*)api)->HandleIncomingPacket = 0;
		}
	}


	PacketChecksum CalculatePacketChecksum(Packet* packet)
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
