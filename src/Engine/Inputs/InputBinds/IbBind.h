///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IbBind.h
/// Description  :  InputBind struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "IbEnum.h"
#include "IbBind.h"

namespace Engine::Input
{
	///----------------------------------------------------------------------------------------------------
	/// Bind_t Struct
	///----------------------------------------------------------------------------------------------------
	struct Bind_t
	{
		EModifiers Modifiers;
		EDevice    Device;
		uint16_t   Code;

		///----------------------------------------------------------------------------------------------------
		/// ctor
		///----------------------------------------------------------------------------------------------------
		Bind_t() = default;

		///----------------------------------------------------------------------------------------------------
		/// ctor
		///----------------------------------------------------------------------------------------------------
		Bind_t(EModifiers aModifiers, EDevice aDevice, uint16_t aCode)
			: Modifiers{ aModifiers }
			, Device{ aDevice }
			, Code{ aCode }
		{}


		///----------------------------------------------------------------------------------------------------
		/// ctor
		///----------------------------------------------------------------------------------------------------
		Bind_t(bool aAlt, bool aCtrl, bool aShift, EDevice aDevice, uint16_t aCode)
			: Modifiers {
				(aAlt ? EModifiers::Alt : EModifiers::None)
				| (aCtrl ? EModifiers::Ctrl : EModifiers::None)
				| (aShift ? EModifiers::Shift : EModifiers::None)
			}
			, Device{ aDevice }
			, Code  { aCode   }
		{}

		///----------------------------------------------------------------------------------------------------
		/// IsBound:
		/// 	Returns true if this input bind has any values set.
		///----------------------------------------------------------------------------------------------------
		bool IsBound() const;
	};

	bool operator==(const Bind_t& lhs, const Bind_t& rhs);
	bool operator!=(const Bind_t& lhs, const Bind_t& rhs);
}
