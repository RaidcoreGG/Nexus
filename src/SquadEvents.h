#pragma once
#include <stdint.h>

namespace SquadEvents {
	/// No Payload
	constexpr char const* EV_SQUAD_REQUEST_STATE = "EV_SQUAD_REQS";
	/// Payload: *Squad
	constexpr char const* EV_SQUAD_RESPONSE_STATE = "EV_SQUAD_RESS";

	/// Payload: *Squad
	constexpr char const* EV_SQUAD_I_JOINED = "EV_SQUAD_IJ";
	/// Payload: LeaveReason (without a pointer, directly as the argument)
	constexpr char const* EV_SQUAD_I_LEFT = "EV_SQUAD_IL";
	/// Payload *SquadMember
	constexpr char const* EV_SQUAD_SOMEONE_GOT_INVITED = "EV_SQUAD_INV";
	/// Payload *SquadMember
	constexpr char const* EV_SQUAD_SOMEONE_JOINED = "EV_SQUAD_SJ";
	/// Payload *SquadMember
	constexpr char const* EV_SQUAD_SOMEONE_MOVED = "EV_SQUAD_SM";
	/// Payload: LeaveReason
	constexpr char const* EV_SQUAD_SOMEONE_LEFT = "EV_SQUAD_SL";

	/// Payload: SquadMemberLimit (without a pointer, directly as the argument)
	constexpr char const* EV_SQUAD_MEMBER_LIMIT_CHANGED = "EV_SQUAD_MLC";


	enum LeaveReason : uint8_t {
		None   = 0,
		Kicked = 1,
	};

	enum SquadMemberLimit : uint8_t {
		Party   =  5,
		Raid    = 10,
		Default = 50,
	};

	#ifndef UUID_DEFINED // windows guid
	struct UUID {
		uint32_t ints[4];
	};
	#endif

	struct SquadMember {
		UUID id;
		char account_name[64];
		char character_name[64];
		uint8_t profession;
		/// can be zero
		uint8_t elite_spec;
		uint8_t subgroup;
		/// Current slot in the global array, or the slot this member was removed from.
		/// If the member is not yet in the squad, this will be `MEMBER_INDEX_EXTERNAL` (256).
		uint8_t array_index;
		uint8_t is_ready;
		uint8_t _reserved[3];
	};

	constexpr uint8_t const MEMBER_INDEX_EXTERNAL = 255;

	struct Squad {
		UUID id;
		uint8_t member_limit;
		uint8_t member_count;
		uint8_t my_index;
		uint8_t _reserved[1];
	#pragma warning(push)
	#pragma warning(disable : 4200) // msvc: nonstandard data[]
		SquadMember members[];
	#pragma warning(pop)
	};

}