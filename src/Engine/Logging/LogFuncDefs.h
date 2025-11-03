///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LogFuncDefs.h
/// Description  :  Function definitions for logging.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "LogEnum.h"

typedef void (*LOGGER_LOG) (ELogLevel aLogLevel, const char* aStr);
typedef void (*LOGGER_LOG2)(ELogLevel aLogLevel, const char* aChannel, const char* aStr);
