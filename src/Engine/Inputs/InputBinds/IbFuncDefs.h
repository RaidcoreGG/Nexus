///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IbFuncDefs.h
/// Description  :  Function definitions for game binds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "IbBind.h"

/* Input Handler Down Only */
typedef void (*INPUTBINDS_PROCESS)            (const char* aIdentifier);
typedef void (*INPUTBINDS_REGISTERWITHSTRING) (const char* aIdentifier, INPUTBINDS_PROCESS aInputBindHandler, const char* aInputBind);
typedef void (*INPUTBINDS_REGISTERWITHSTRUCT) (const char* aIdentifier, INPUTBINDS_PROCESS aInputBindHandler, InputBindV1_t aInputBind);

/* Input Handler Down and Release */
typedef void (*INPUTBINDS_PROCESS2)           (const char* aIdentifier, bool aIsRelease);
typedef void (*INPUTBINDS_REGISTERWITHSTRING2)(const char* aIdentifier, INPUTBINDS_PROCESS2 aInputBindHandler, const char* aInputBind);
typedef void (*INPUTBINDS_REGISTERWITHSTRUCT2)(const char* aIdentifier, INPUTBINDS_PROCESS2 aInputBindHandler, InputBindV1_t aInputBind);
typedef void (*INPUTBINDS_INVOKE)             (const char* aIdentifier, bool aIsRelease);

/* Input Handler Down and Release also return whether input should not be processed further. */
typedef bool (*INPUTBINDS_PROCESS3)           (const char* aIdentifier, bool aIsRelease);
typedef void (*INPUTBINDS_REGISTERWITHSTRING3)(const char* aIdentifier, INPUTBINDS_PROCESS3 aInputBindHandler, const char* aInputBind);
typedef void (*INPUTBINDS_REGISTERWITHSTRUCT3)(const char* aIdentifier, INPUTBINDS_PROCESS3 aInputBindHandler, InputBindV1_t aInputBind);
typedef bool (*INPUTBINDS_INVOKE2)            (const char* aIdentifier, bool aIsRelease);

typedef void (*INPUTBINDS_DEREGISTER)         (const char* aIdentifier);
