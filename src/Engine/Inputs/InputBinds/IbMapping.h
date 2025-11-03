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
#include "IbFuncDefs.h"

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
