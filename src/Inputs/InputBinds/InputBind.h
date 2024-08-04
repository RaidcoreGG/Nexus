///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  InputBind.h
/// Description  :  InputBind struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef INPUTBIND_H
#define INPUTBIND_H

#include "EInputBindType.h"

///----------------------------------------------------------------------------------------------------
/// LegacyInputBind Struct
/// 	Old Keybind struct used for backwards compatibility within APIs.
///----------------------------------------------------------------------------------------------------
struct LegacyInputBind
{
	unsigned short	Key;
	bool			Alt;
	bool			Ctrl;
	bool			Shift;
};

///----------------------------------------------------------------------------------------------------
/// InputBind Struct
///----------------------------------------------------------------------------------------------------
struct InputBind
{
	bool			Alt;
	bool			Ctrl;
	bool			Shift;

	EInputBindType	Type;
	unsigned short	Code;

	InputBind() = default;
	InputBind(bool aAlt, bool aCtrl, bool aShift, EInputBindType aType, unsigned short aCode)
		: Alt{ aAlt }
		, Ctrl{ aCtrl }
		, Shift{ aShift }
		, Type{ aType }
		, Code{ aCode }
	{}
	InputBind(LegacyInputBind aLegacyInputBind);

	///----------------------------------------------------------------------------------------------------
	/// IsBound:
	/// 	Returns true if this input bind has any values set.
	///----------------------------------------------------------------------------------------------------
	bool IsBound();
};

bool operator==(const InputBind& lhs, const InputBind& rhs);
bool operator!=(const InputBind& lhs, const InputBind& rhs);

#endif
