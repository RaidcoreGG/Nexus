///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IbMapping.h
/// Description  :  Mapped Bind_t struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "IbBind.h"
#include "IbEnum.h"

namespace Raidcore::Engine::Input
{
	/* Input Handler Down Only */
	typedef void (*IB_CALLBACK_V1)(const char* aIdentifier);

	/* Input Handler Down and Release */
	typedef void (*IB_CALLBACK_V2)(const char* aIdentifier, bool aIsRelease);

	/* Input Handler Down and Release also return whether input should not be processed further. */
	typedef bool (*IB_CALLBACK_V3)(const char* aIdentifier, bool aIsRelease);

	///----------------------------------------------------------------------------------------------------
	/// IbMapping_t Struct
	///----------------------------------------------------------------------------------------------------
	struct IbMapping_t
	{
		Bind_t    Bind = {};
		bool           Passthrough = false;
		EIbHandlerType HandlerType = EIbHandlerType::None;
		union
		{
			IB_CALLBACK_V1 Handler_DownOnlyAsync;
			IB_CALLBACK_V2 Handler_DownReleaseAsync;
			IB_CALLBACK_V3 Handler_DownRelease = nullptr;
		};
	};
}
