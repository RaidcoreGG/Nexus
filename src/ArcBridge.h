#pragma once

#include <stdint.h>
#include <Windows.h>

namespace ArcBridge {
	char const *const EV_REQ_ACC_NAME = "EV_REQUEST_ACCOUNT_NAME";

	char const *const EV_RES_ACC_NAME          = "EV_ACCOUNT_NAME";
	//NOTE(Rennorb): This isn't actually a "squad" leave as you would expect, its a "context" leave.
	// So this gets triggered for exampl upon logging out of the game.
	char const *const EV_RES_SQUAD_SELF_LEAVE  = "EV_ARCDPS_SELF_LEAVE";
	char const *const EV_RES_SQUAD_SELF_JOIN   = "EV_ARCDPS_SELF_JOIN";
	char const *const EV_RES_SQUAD_OTHER_JOIN  = "EV_ARCDPS_SQUAD_JOIN";
	char const *const EV_RES_SQUAD_OTHER_LEAVE = "EV_ARCDPS_SQUAD_LEAVE";
	char const *const EV_RES_SQUAD_UPDATE      = "EV_UNOFFICIAL_EXTRAS_SQUAD_UPDATE";

	//NOTE(Rennorb): directly joinked from UA
	enum class UserRole : uint8_t
	{
		SquadLeader = 0,
		Lieutenant = 1,
		Member = 2,
		Invited = 3,
		Applied = 4,
		None = 5,
		Invalid = 6 // Internal only
	};

	//NOTE(Rennorb): directly joinked from UA
	struct UserInfo {
		const char* AccountName; // Null terminated account name, including leading ':'. Only valid for the duration of the call

		// Unix timestamp when the user joined the squad (or 0 if time could not be determined)
		__time64_t JoinTime;

		UserRole Role; // Role in squad, or ::None if the user was removed from the squad

		// Subgroup the user is in (0 when no subgroup could be found, which is either the first subgroup or no subgroup)
		uint8_t Subgroup;

		// Whether this player is ready or not (in a squad ready check). Role == UserRole::SquadLeader and ReadyStatus == true
		// implies that a ready check was just started. Similarly, Role == UserRole::SquadLeader and ReadyStatus == false
		// implies that a ready check either finished or was cancelled. If everyone in the squad had an event sent with
		// ReadyStatus == true then that means that the ready check finished successfully (after which there will be events
		// sent for each user where their ReadyStatus == false)
		bool ReadyStatus;

		uint8_t _Unused1 = 0; // padding
		uint32_t _Unused2 = 0; // padding
	};

	//NOTE(Rennorb): directly joinked from the bridge addon
	struct SquadUpdate {
		UserInfo* Infos;
		uint64_t  InfoCount;
	};

	//NOTE(Rennorb): directly joinked from the bridge addon
	struct AgentUpdate {
		char account[64];		// dst->name	= account name
		char character[64];		// src->name	= character name
		uintptr_t id;			// src->id		= agent id
		uintptr_t instanceId;	// dst->id		= instance id (per map)
		uint32_t added;			// src->prof	= is new agent
		uint32_t target;		// src->elite	= is new targeted agent
		uint32_t Self;			// dst->Self	= is Self
		uint32_t prof;			// dst->prof	= profession / core spec
		uint32_t elite;			// dst->elite	= elite spec
		uint16_t team;			// src->team	= team
		uint16_t subgroup;		// dst->team	= subgroup
	}; 
}
