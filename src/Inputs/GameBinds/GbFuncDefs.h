///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  GbFuncDefs.h
/// Description  :  Function definitions for game binds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------


#ifndef GBFUNCDEFS_H
#define GBFUNCDEFS_H

#include "GbEnum.h"

typedef void (*GAMEBINDS_PRESSASYNC)  (EGameBinds aGameBind);
typedef void (*GAMEBINDS_RELEASEASYNC)(EGameBinds aGameBind);
typedef void (*GAMEBINDS_INVOKEASYNC) (EGameBinds aGameBind, int aDuration);
typedef void (*GAMEBINDS_PRESS)       (EGameBinds aGameBind);
typedef void (*GAMEBINDS_RELEASE)     (EGameBinds aGameBind);
typedef bool (*GAMEBINDS_ISBOUND)     (EGameBinds aGameBind);

#endif
