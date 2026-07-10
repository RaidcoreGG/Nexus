///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  GbFuncDefs.h
/// Description  :  Function definitions for game binds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "GbEnum.h"

///----------------------------------------------------------------------------------------------------
/// Raidcore::Nexus::GW2 Namespace
///----------------------------------------------------------------------------------------------------
namespace Raidcore::Nexus::GW2
{
	typedef void (*GAMEBINDS_PRESSASYNC)  (EGameBinds aGameBind);
	typedef void (*GAMEBINDS_RELEASEASYNC)(EGameBinds aGameBind);
	typedef void (*GAMEBINDS_INVOKEASYNC) (EGameBinds aGameBind, int aDuration);
	typedef void (*GAMEBINDS_PRESS)       (EGameBinds aGameBind);
	typedef void (*GAMEBINDS_RELEASE)     (EGameBinds aGameBind);
	typedef bool (*GAMEBINDS_ISBOUND)     (EGameBinds aGameBind);
}
