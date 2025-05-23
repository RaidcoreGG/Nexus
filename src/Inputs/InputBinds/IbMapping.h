///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IbMapping.h
/// Description  :  Mapped InputBind struct definition.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef IBMAPPING_H
#define IBMAPPING_H

#include "IbBindV2.h"
#include "IbEnum.h"
#include "IbFuncDefs.h"

///----------------------------------------------------------------------------------------------------
/// IbMapping Struct
///----------------------------------------------------------------------------------------------------
struct IbMapping
{
	InputBind      Bind        = {};
	bool           Passthrough = false;
	EIbHandlerType HandlerType = EIbHandlerType::None;
	union {
		INPUTBINDS_PROCESS  Handler_DownOnlyAsync;
		INPUTBINDS_PROCESS2 Handler_DownReleaseAsync;
		INPUTBINDS_PROCESS3 Handler_DownRelease;
	};
};

#endif
