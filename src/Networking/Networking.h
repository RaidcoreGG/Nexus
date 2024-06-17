#pragma once

#include "Packet.h"
#include "Loader/AddonAPI.h"
#include <WinSock2.h>
#include <map>

namespace Networking
{
	typedef u32 UserId;
	typedef u32 SessionId;

	extern std::map<AddonSignature, PacketHandler*> Handlers;

	enum InitState {
		_UNKNOWN           = 0,
		Initializing       = 1,
		WaitingForUserId   = 2,
		ReadyForSession    = 3,
		SessionEstablished = 4
	};
	extern InitState State;
	void Init();

	bool PrepareAndBroadcastPacket(Packet* packet);
	void SendInternalPacket(Packet* packet);
	void JoinSession();
	void LeaveSession();

	PacketHandler* GetPacketHandlerFromApi(int version, AddonAPI* api);
}
