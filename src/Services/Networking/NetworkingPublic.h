#pragma once
/*

	This header contains the public struct definitions for packages transferred by the routing system,
	as well as the function definitions for related callbacks.

	If you want to use the system, an example usage is available at 
	https://github.com/SaculRennorb/nexus-example-networked-addon/blob/master/src/main.cpp

	A version of this header that has standalone definitions is also available in that repo.

*/

#include "BaseTypes.h"
#include <stdint.h>
#include <guiddef.h>

namespace Networking {

enum PacketFlags : uint8_t {
	None = 0,
	/// This flag indicates that this packet contains a source UserId int the first 32bit aligned 16 bytes of the data.  
	/// The id will be set by the server, just make sure to have enough space in the packet.
	/// This also increases the minimum valid packet size by those 16 bytes.
	ContainsSource = 1 << 0,
	/// This flag indicates that this is not a broadcast packet, and the last 16 bytes in the data contain a destination UserId.  
	/// This is placed at the end of the packet, because in almost all cases a handler doesn't care about it being a broadcast or unicast packet,
	/// and so the handler implementation can be teh same for both versions if the handler doesn't care.
	/// This also increases the minimum valid packet size by 16 bytes.
	ContainsTarget = 1 << 1,
};

#pragma pack(push, 1)
typedef struct {
	/// The addon should not fill this out, the routing will give it the correct id.
	//TODO(Rennorb): endianess?
	AddonSignature TargetAddonId;
	/// The total length of the packet in int32's this includes the header and crc32 at the end, so it should be
	/// 1 + round_up(0.5 + byte_len(data) / 4) + 1
	uint8_t LengthInU32s;
	PacketFlags Flags;
} PacketHeader;

static_assert(sizeof(PacketHeader) == 6);

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


//
// Helpers
//

// Helper for setting up the networking packet len passing a var or type
#define PACKET_LEN(packet) sizeof(packet) / 4

/// A small helper to synthesize a debug "packet discarded" message in the shape of `<fmt>, args...` from ma header.
/// This is intended to be used by the receiving addon, so it doesn't format the target addon id.
#define FORMAT_DISCARD_MESSAGE(packet) "Discarding invalid packet with\n\tLength %uB\n\tFlags: %02x\n\t\tContainsTarget: %c\n\t\tContainsSource: %c\n\t2 Bytes after header: %02x %02x", \
	packet->Header.LengthInU32s * 4, packet->Header.Flags, packet->Header.Flags & PacketFlags::ContainsTarget ? '1' : '0', packet->Header.Flags & PacketFlags::ContainsSource ? '1' : '0', \
	((uint8_t*)packet)[sizeof(PacketHeader)], ((uint8_t*)packet)[sizeof(PacketHeader) + 1] // these always exist because of rounding, so addons are likely to put some header extension into it


//
// Handlers
//

//TODO(Rennorb) @cleanup: change case to match other exposed fns

/// The shape of function that a plugin can register to handle packets destined for it.
/// If you receive a packet from this callback it is already destined for you and the crc has been validated.
typedef void PacketHandler(Packet const* packet);

/// The shape of function that a plugin can register to handle packets destined for it.
/// This function will add addon id and crc into the packet, so there is no need for the addon itself to do it.
typedef bool PacketPrepareAndBroadcast(Packet* packet);

}
