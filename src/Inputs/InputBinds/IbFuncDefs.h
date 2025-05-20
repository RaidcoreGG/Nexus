///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IbFuncDefs.h
/// Description  :  Function definitions for game binds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef IBFUNCDEFS_H
#define IBFUNCDEFS_H

#include "IbBind.h"

typedef void (*INPUTBINDS_PROCESS)            (const char* aIdentifier);
typedef void (*INPUTBINDS_REGISTERWITHSTRING) (const char* aIdentifier, INPUTBINDS_PROCESS aInputBindHandler, const char* aInputBind);
typedef void (*INPUTBINDS_REGISTERWITHSTRUCT) (const char* aIdentifier, INPUTBINDS_PROCESS aInputBindHandler, InputBindV1 aInputBind);

typedef void (*INPUTBINDS_PROCESS2)           (const char* aIdentifier, bool aIsRelease);
typedef void (*INPUTBINDS_REGISTERWITHSTRING2)(const char* aIdentifier, INPUTBINDS_PROCESS2 aInputBindHandler, const char* aInputBind);
typedef void (*INPUTBINDS_REGISTERWITHSTRUCT2)(const char* aIdentifier, INPUTBINDS_PROCESS2 aInputBindHandler, InputBindV1 aInputBind);

typedef void (*INPUTBINDS_INVOKE)             (const char* aIdentifier, bool aIsRelease);

typedef void (*INPUTBINDS_DEREGISTER)         (const char* aIdentifier);

#endif
