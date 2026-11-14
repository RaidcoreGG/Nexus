///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IbMapping.h
/// Description  :  Mapped InputBind_t struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "IbBindV2.h"
#include "IbEnum.h"

/* Input Handler Down Only */
typedef void (*INPUTBINDS_PROCESS) (const char* aIdentifier);

/* Input Handler Down and Release */
typedef void (*INPUTBINDS_PROCESS2)(const char* aIdentifier, bool aIsRelease);

/* Input Handler Down and Release also return whether input should not be processed further. */
typedef bool (*INPUTBINDS_PROCESS3)(const char* aIdentifier, bool aIsRelease);

///----------------------------------------------------------------------------------------------------
/// IbMapping_t Struct
///----------------------------------------------------------------------------------------------------
struct IbMapping_t
{
	InputBind_t    Bind        = {};
	bool           Passthrough = false;
	EIbHandlerType HandlerType = EIbHandlerType::None;
	union {
		INPUTBINDS_PROCESS  Handler_DownOnlyAsync;
		INPUTBINDS_PROCESS2 Handler_DownReleaseAsync;
		INPUTBINDS_PROCESS3 Handler_DownRelease      = nullptr;
	};
};
