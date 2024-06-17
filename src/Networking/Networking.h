#pragma once

#include "NetworkingPublic.h"
#include "Loader/AddonAPI.h"
#include <WinSock2.h>
#include <map>

namespace Networking
{
	typedef uint32_t u32;
	typedef uint16_t u16;

	typedef u32 UserId;
	typedef u32 SessionId;

	extern std::map<AddonSignature, PacketHandler*> Handlers;

	/// The source from wich to pull squad join information.
	/// Higher values are more desirable.
	enum SessionSource {
		/// No proper squad information is available, the player can only join a global squad based on his current character name.
		Mumble         = 0,
		/// Arc is available, but ArcBridge is not, will use arc directly for squad events.
		Arc            = 1,
		ArcBridge      = 2,
		/// Custom network addon is loaded. This should provide the best events for this specific purpose.
		NetworkAddon   = 3,
	};
	extern SessionSource CurrentSessionSource;
	/// Will handle required cahnges in the UserID and packet sending depending on initialization state of nexus and the networking module
	void ChangeSessionSource(SessionSource newSource);

	enum ModuleState {
		_UNKNOWN           = 0,
		Initializing       = 1,
		WaitingForUserIds   = 2,
		ReadyForSession    = 3,
		SessionEstablished = 4
	};
	extern ModuleState State;
	void Init();

	bool PrepareAndBroadcastPacket(Packet* packet);
	void SendInternalPacket(Packet* packet);
	void LeaveSession();

	PacketHandler* GetPacketHandlerFromApi(int version, AddonAPI* api);
}
