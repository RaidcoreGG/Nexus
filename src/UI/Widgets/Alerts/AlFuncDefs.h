///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AlFuncDefs.h
/// Description  :  Function definitions for alerts.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "AlEnum.h"

typedef void (*ALERTS_NOTIFY)  (const char* aMessage);
typedef void (*ALERTS_NOTIFY2) (EAlertType aType, const char* aMessage);
