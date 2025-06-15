///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LdrFuncDefs.h
/// Description  :  Function definitions for the loader..
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef LDRFUNCDEFS_H
#define LDRFUNCDEFS_H

#include "AddonDefinition.h"
#include "Core/Addons/API/ApiBase.h"

struct AddonDef_t;

typedef AddonDef_t* (*GETADDONDEF) ();
typedef void        (*ADDON_LOAD)  (AddonAPI_t* aAPI);
typedef void        (*ADDON_UNLOAD)();


#endif
