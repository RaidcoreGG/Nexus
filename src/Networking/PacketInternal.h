#pragma once

#include "Networking.h"
#include "Packet.h"

namespace Networking {
namespace Internal {

	enum class PacketType : u16 {
		_UNKNOWN       = 0,
		JoinSession    = 1,
		SessionCreated = 2,
		ChangeUserId   = 3,
		LeaveSession   = 4,

	};

	struct PacketHeader {
		Networking::PacketHeader BaseHeader;
		PacketType Type;
	};
	static_assert(sizeof(PacketHeader) == 8);

	const AddonSignature INTERNAL_PACKET = 0;
	const int MIN_PACKET_SIZE = (sizeof(PacketHeader) + 3) / 4 + (sizeof(PacketChecksum) + 3) / 4; // separate division for separate rounding for alignment requirements
	
	struct JoinSession {
		PacketHeader Header {{ INTERNAL_PACKET, PACKET_LEN(JoinSession) }, PacketType::JoinSession };
		UserId SessionUser;
		UserId Me;
		PacketChecksum CRC;
	};

	struct ChangeUserId {
		PacketHeader Header {{ INTERNAL_PACKET, PACKET_LEN(ChangeUserId) }, PacketType::ChangeUserId };
		UserId OldId;
		UserId NewId;
		PacketChecksum CRC;
	};

	struct LeaveSession {
		PacketHeader Header {{ INTERNAL_PACKET, PACKET_LEN(LeaveSession) }, PacketType::LeaveSession };
		UserId Me;
		PacketChecksum CRC;
	};

	struct SessionCreated {
		PacketHeader Header;
		SessionId Session;
		PacketChecksum Crc;
	};
}
}

