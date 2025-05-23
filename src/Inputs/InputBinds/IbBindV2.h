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
/// InputBind Struct
///----------------------------------------------------------------------------------------------------
struct InputBind
{
	bool           Alt;
	bool           Ctrl;
	bool           Shift;

	EInputDevice   Device;
	unsigned short Code;

	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	InputBind() = default;

	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	InputBind(bool aAlt, bool aCtrl, bool aShift, EInputDevice aDevice, unsigned short aCode)
		: Alt         { aAlt         }
		, Ctrl        { aCtrl        }
		, Shift       { aShift       }
		, Device      { aDevice      }
		, Code        { aCode        }
	{}

	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	InputBind(InputBindV1 aLegacyInputBind);

	///----------------------------------------------------------------------------------------------------
	/// IsBound:
	/// 	Returns true if this input bind has any values set.
	///----------------------------------------------------------------------------------------------------
	bool IsBound() const;
};

bool operator==(const InputBind& lhs, const InputBind& rhs);
bool operator!=(const InputBind& lhs, const InputBind& rhs);

#endif
