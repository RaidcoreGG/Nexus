#pragma once

#include "BaseTypes.h"
#include "NetworkingPublic.h"
#include "Loader/AddonAPI.h"
#include "SquadEvents.h"
#include <WinSock2.h>
#include <map>
#include <guiddef.h>

namespace Networking
{
	/// Networking is now ready. As networking requires an addon, this event in not guaranteed to ever be raised.
	/// no payload
	constexpr char const EV_NETWORKING_READY[] = "EV_NW_RDY";
	/// Networking is about to unload, this is your last chance to get packets out.
	/// no payload
	constexpr char const EV_NETWORKING_UNLOADING[] = "EV_NW_UNL";

	typedef GUID UserId;
	constexpr UserId INVALID_USER = {0};

#define SPLIT_USER_ID(id) ((unsigned int*)&id)[0], ((unsigned int*)&id)[1], ((unsigned int*)&id)[2], ((unsigned int*)&id)[3]

	typedef GUID SessionId;
	constexpr SessionId INVALID_SESSION = {0};

#define SPLIT_SESSION_ID(id) ((unsigned int*)&id)[0], ((unsigned int*)&id)[1], ((unsigned int*)&id)[2], ((unsigned int*)&id)[3]

	enum ModuleState {
		_UNKNOWN                 = 0,
		Initializing             = 1,
		WaitingForJoin           = 2,
		WaitingForServerResponse = 3,
		SessionEstablished       = 4
	};

	extern std::map<AddonSignature, PacketHandler*> Handlers;
	extern ModuleState State;
	extern bool AddonLoaded;
	extern UserId MyUserId;
	/// This can be null depending on the state
	extern SquadEvents::Squad* CurrentSquad; //TODO(Rennorb) @cleanup: should this be a shared resource?

	void Init();

	bool PrepareAndBroadcastPacket(Packet* packet);
	void LeaveSession();

	PacketHandler* GetPacketHandlerFromApi(int version, AddonAPI* api);
	void ResetHandlerPtr(int version, AddonAPI* api);
}
