#pragma once

#include <stdint.h>
#include "Networking.h"

namespace Networking {
namespace Internal {

	enum class PacketType : uint16_t {
		_UNKNOWN       = 0,
		JoinSession    = 1,
		SessionCreated = 2,
		LeaveSession   = 4,
	};

	struct PacketHeader : Networking::PacketHeader {
		PacketType Type;
	};
	static_assert(sizeof(PacketHeader) % 4 == 0);

	const AddonSignature INTERNAL_PACKET = 0;
	const int MIN_PACKET_SIZE = (sizeof(PacketHeader) + 3) / 4 + (sizeof(PacketChecksum) + 3) / 4; // separate division for separate rounding for alignment requirements
	
	struct JoinSession {
		PacketHeader Header { INTERNAL_PACKET, PACKET_LEN(JoinSession), PacketType::JoinSession };
		UserId Me;
		SessionId Session;
		PacketChecksum CRC;
	};

	struct LeaveSession {
		PacketHeader Header { INTERNAL_PACKET, PACKET_LEN(LeaveSession), PacketType::LeaveSession };
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

