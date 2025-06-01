///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LogEnum.h
/// Description  :  Enumerations for logging.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef LOGENUM_H
#define LOGENUM_H

#include <cstdint>

///----------------------------------------------------------------------------------------------------
/// ELogLevel Enumeration
///----------------------------------------------------------------------------------------------------
enum class ELogLevel : uint32_t
{
	OFF      = 0,
	CRITICAL = 1,
	WARNING  = 2,
	INFO     = 3,
	DEBUG    = 4,
	TRACE    = 5,
	ALL
};

#endif
