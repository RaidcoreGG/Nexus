///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ApiNext.h
/// Description  :  Function types of the next addon API revision. Not frozen until a revision uses them.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "ApiV6.h"
#include "UI/Widgets/Alerts/AlEnum.h"

typedef void (*INPUTBINDS_REGISTERWITHSTRING3)(const char* aIdentifier, INPUTBINDS_PROCESS3 aInputBindHandler, const char* aInputBind);
typedef void (*INPUTBINDS_REGISTERWITHSTRUCT3)(const char* aIdentifier, INPUTBINDS_PROCESS3 aInputBindHandler, InputBindV1_t aInputBind);
typedef bool (*INPUTBINDS_INVOKE2)            (const char* aIdentifier, bool aIsRelease);

typedef Texture_t* (*TEXTURES_GETORCREATEFROMURL2)(const char* aIdentifier, const char* aURL);
typedef void       (*TEXTURES_LOADFROMURL2)       (const char* aIdentifier, const char* aURL, TEXTURES_RECEIVECALLBACK aCallback);

typedef void (*ALERTS_NOTIFY2)(EAlertType aType, const char* aMessage);
