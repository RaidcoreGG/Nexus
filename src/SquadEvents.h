#pragma once
#include <stdint.h>

namespace SquadEvents {
	//
	// NOTE(Rennorb): All of this is still very much work in progress.
	//  Elements prefixed with an underscore are planned, but don't properly work yet.
	// NOTE(Rennorb): At least for now the Squad pointer is always stable, but the members might get rearranged at any time.
	// 



	/// No Payload
	constexpr char const* EV_SQUAD_REQUEST_STATE = "EV_SQUAD_REQS";
	/// Payload: *Squad
	constexpr char const* EV_SQUAD_RESPONSE_STATE = "EV_SQUAD_RESS";

	/// Payload: *Squad
	constexpr char const* EV_SQUAD_I_GOT_INVITED = "EV_SQUAD_IGI";
	//TODO(Rennorb) @bug: this currently also fires after "i got invited"
	/// Payload: *Squad
	constexpr char const* EV_SQUAD_I_JOINED = "EV_SQUAD_IJ";
	/// Payload: LeaveReason (without a pointer, directly as the argument)
	constexpr char const* EV_SQUAD_I_LEFT = "EV_SQUAD_IL";
	/// Payload *SquadMember
	constexpr char const* EV_SQUAD_SOMEONE_GOT_INVITED = "EV_SQUAD_SINV";
	/// Payload *InvitationResolution (squad will no longer have the member if the resolution is unsuccessful. data will invalidate after the event)
	constexpr char const* _EV_SQUAD_INVITATION_RESOLVED = "EV_SQUAD_SINVR";
	//TODO(Rennorb) @bug: this currently also fires before "someone got invited"
	/// Payload *SquadMember
	constexpr char const* EV_SQUAD_SOMEONE_JOINED = "EV_SQUAD_SJ";
	/// Payload *SquadMember
	constexpr char const* _EV_SQUAD_SOMEONE_MOVED = "EV_SQUAD_SM";
	//TODO(Rennorb) @bug: this currently also fires before "I left squad", once for each remaining member
	/// Payload: *MemberLeftData (the squad is already updated and the data will invalidate after the event)
	constexpr char const* EV_SQUAD_SOMEONE_LEFT = "EV_SQUAD_SL";

	/// Payload *SquadMember
	constexpr char const* _EV_SQUAD_READY_CHECK_STARTED = "EV_SQUAD_RCS";
	/// Payload *SquadMember
	constexpr char const* _EV_SQUAD_READY_CHECK_CHANGED = "EV_SQUAD_RCC";
	/// Payload ReadyCheckOutcome (without a pointer, directly as the argument)
	constexpr char const* _EV_SQUAD_READY_CHECK_ENDED = "EV_SQUAD_RCE";

	/// Payload: SquadMemberLimit (without a pointer, directly as the argument)
	constexpr char const* _EV_SQUAD_MEMBER_LIMIT_CHANGED = "EV_SQUAD_MLC";

	enum SquadMemberLimit : uint8_t {
		NoSquad =  0,
		Party   =  5,
		Raid    = 10,
		Default = 50,
	};

	enum SquadMemberFlags : uint8_t {
		_InvitePending = 1 << 0,
		_IsCommander   = 1 << 1,
		_IsLieutenant  = 1 << 2,
		_IsLeech       = 1 << 3,
		_IsReady       = 1 << 4,
	};

	struct SquadMember {
		UUID id;
		char account_name[64];
		char character_name[64];
		uint8_t _profession;
		/// can be zero
		uint8_t _elite_spec;
		uint8_t _subgroup;
		/// Current slot in the global array, or the slot this member was removed from.
		/// If the member is not yet in the squad, this will be `MEMBER_INDEX_EXTERNAL` (255).
		uint8_t array_index;
		SquadMemberFlags flags;
		uint8_t _reserved[3];
	};

	constexpr uint8_t const MEMBER_INDEX_EXTERNAL = 255;

	struct Squad {
		UUID id;
		SquadMemberLimit member_limit;
		uint8_t member_count;
		uint8_t my_index;
		/// If the member is no comm in the squad, this will be `MEMBER_INDEX_EXTERNAL` (255).
		uint8_t commander_index;
#pragma warning(push)
#pragma warning(disable : 4200) // msvc: nonstandard data[]
		SquadMember members[];
#pragma warning(pop)
	};

	enum LeaveReason : uint8_t {
		None   = 0,
		Kicked = 1,
	};

	struct MemberLeftData {
		SquadMember member;
		LeaveReason reason;
		uint8_t _reserved[3];
	};

	enum ReadyCheckOutcome : uint8_t {
		Abort = 0,
		Ready = 1,
	};

	enum InvitationOutcome : uint8_t {
		Accepted = 0,
		Canceled = 1,
		Rejected = 2,
	};

	struct InvitationResolution {
		SquadMember* member;
		InvitationOutcome outcome;
		uint8_t _reserved[3];
	};

}