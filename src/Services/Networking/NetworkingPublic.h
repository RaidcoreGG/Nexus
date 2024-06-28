#pragma once
/*

	This header contains the public struct definitions for packages transferred by the routing system,
	as well as the function definitions for related callbacks.

	If you want to use the system, the idea is the following:

*/
#if 0
	#include "nexus.h"
	#include "packet.h"

	#define MY_ADDON_ID  -1234
	#define _LOG(lvl, fmt, ...) Log(ELogLevel::ELogLevel_ ## lvl, "PingPongAddon", fmt, __VA_ARGS__)

	enum PacketType : uint16_t {
		_UNKNOWN = 0,
		Ping = 1,
		Pong = 2,
	};

	// Create a custom Packet struct you want to send:
	#pragma pack(push, 1)
	struct MyPacket {
		PacketHeader Header;
		PacketType PacketType; // if we want to have multiple packets, we need to differentiate them somehow

		uint32_t Number; // additional data we want to send

		PacketChecksum CRC; // just for sizing, we dont touch that
	};
	#pragma pack(pop)

	// Send function: `bool PrepareAndBroadcastPacket(Packet* packet)`
	PacketPrepareAndBroadcast* PrepareAndBroadcastPacket = 0;
	LOGGER_LOG2 Log = 0;

	void HandleIncomingPacket(Packet const* _packet)
	{
		MyPacket const* packet = (MyPacket const*)_packet;
		switch(packet->PacketType) {
		case PacketType::Ping: {
			_LOG(INFO, "ping received");

			MyPacket response = {0};
			response.Header.TargetAddon = MY_ADDON_ID; // TODO: actually make the zero thing work
			response.Header.LengthInU32s = PACKET_LEN(response);
			response.PacketType = PacketType::Pong;

			response.Number = packet->Number;

			PrepareAndBroadcastPacket((Packet*)&response);
			_LOG(TRACE, "pong sent");
		} break;

		case PacketType::Pong: {
			_LOG(INFO, "pong received");
			//NOTE(Rennorb): For the sake of keeping it short I dont format the number into the string here, but you get the idea.
		} break;


		default:
			_LOG(TRACE, "Unknown packet received");
		}
	}


	void ProcessMyKeybinds(const char* aIdentifier, bool aIsRelease)
	{
		if(aIsRelease) return; // only on key press

		MyPacket ping = {0};
		ping.Header.TargetAddon = MY_ADDON_ID; // TODO: actually make the zero thing work
		ping.Header.LengthInU32s = PACKET_LEN(ping);
		ping.PacketType = PacketType::Ping;

		ping.Number = 1;

		PrepareAndBroadcastPacket((Packet*)&ping);
		_LOG(TRACE, "Ping sent");
	}


	// Use the load function to get a function for sending packets, and to install a callback for receiving packets
	void MyAddonLoad(AddonAPI* api)
	{
		Log = api->Log;
		PrepareAndBroadcastPacket = api->PrepareAndBroadcastPacket;
		api->HandleIncomingPacket = &HandleIncomingPacket;
		api->RegisterKeybindWithString("send_pings", &ProcessMyKeybinds, "S");
	}

	// Usual addon stuff, expose the info struct...
	AddonDefinition MyAddonDefinition {
		.Signature   = MY_ADDON_ID,
		.APIVersion  = NEXUS_API_VERSION,
		.Name        = "Test",
		.Author      = "Rennorb",
		.Description = "",
		.Load        = MyAddonLoad,
		.Flags       = EAddonFlags_DisableHotloading, // wired hack because of buggy validity check
	};
	extern "C" __declspec(dllexport) void* __cdecl GetAddonDef() { return &MyAddonDefinition; }

	//required by windows, we don't care about it 
	BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) { return TRUE; }
#endif

// Introduction over, now for the actual definitions:

#include <stdint.h>

#ifdef __cplusplus
namespace Networking {
#endif

typedef int32_t AddonSignature;

#pragma pack(push, 1)
typedef struct {
	/// The addon should not fill this out, the routing will give it the correct id.
	//TODO(Rennorb): endianess?
	int32_t TargetAddonId;
	/// The total length of the packet in int32's this includes teh header and crc32 at the end, so it should be
	/// 1 + round_up(0.5 + byte_len(data) / 4) + 1
	uint16_t LengthInU32s;
} PacketHeader;

#ifdef __cplusplus
static_assert(sizeof(PacketHeader) == 6);
#endif

#define PACKET_LEN(packet) sizeof(packet) / 4

#pragma warning(push)
#pragma warning(disable : 4200) // nonstandard data[]
/// This is the packet that an addon produces.
/// There is a crc32 missing at the end of the structure, which also should not be filled out by the addon - 
/// this is just how end of struct arrays work unfortunately, we cant have fields after it.
typedef struct {
	PacketHeader Header;
	/// Your data. This is just an array of u16 for alignment purposes, its actual content will not be touched by the routing.
	/// This must always come up to a full int32 alignment together with the header, or in other words: It must always contain 2n + 1 int16's.
	uint16_t Data[];
	//TODO(Rennorb): crc endianess?
} Packet;
#pragma warning(pop)
#pragma pack(pop)

typedef uint32_t PacketChecksum;

//TODO(Rennorb) @cleanup: change case to match other exposed fns

/// The shape of function that a plugin can register to handle packets destined for it.
/// If you receive a packet from this callback it is already destined for you and the crc has been validated.
typedef void PacketHandler(Packet const* packet);

/// The shape of function that a plugin can register to handle packets destined for it.
/// This function will add addon id and crc into the packet, so there is no need for the addon itself to do it.
typedef bool PacketPrepareAndBroadcast(Packet* packet);

#ifdef __cplusplus
}
#endif
