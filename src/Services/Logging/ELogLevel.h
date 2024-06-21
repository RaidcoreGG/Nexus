///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ELogLevel.h
/// Description  :  Log level enum.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef ELOGLEVEL_H
#define ELOGLEVEL_H

enum class ELogLevel
{
	OFF         = 0,
	CRITICAL    = 1,
	WARNING     = 2,
	INFO        = 3,
	DEBUG       = 4,
	TRACE       = 5,
	ALL
};

#endif
