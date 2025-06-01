///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IbBindV2.h
/// Description  :  InputBind struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef IBBINDV2_H
#define IBBINDV2_H

#include "IbEnum.h"
#include "IbBind.h"

///----------------------------------------------------------------------------------------------------
/// InputBind_t Struct
///----------------------------------------------------------------------------------------------------
struct InputBind_t
{
	bool           Alt;
	bool           Ctrl;
	bool           Shift;

	EInputDevice   Device;
	unsigned short Code;

	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	InputBind_t() = default;

	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	InputBind_t(bool aAlt, bool aCtrl, bool aShift, EInputDevice aDevice, unsigned short aCode)
		: Alt    { aAlt    }
		, Ctrl   { aCtrl   }
		, Shift  { aShift  }
		, Device { aDevice }
		, Code   { aCode   }
	{}

	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	InputBind_t(InputBindV1_t aLegacyInputBind);

	///----------------------------------------------------------------------------------------------------
	/// IsBound:
	/// 	Returns true if this input bind has any values set.
	///----------------------------------------------------------------------------------------------------
	bool IsBound() const;
};

bool operator==(const InputBind_t& lhs, const InputBind_t& rhs);
bool operator!=(const InputBind_t& lhs, const InputBind_t& rhs);

#endif
