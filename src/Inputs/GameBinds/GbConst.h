///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  GbConst.h
/// Description  :  Constant data for game binds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef GBCONST_H
#define GBCONST_H

#include <string>

#include "EGameBinds.h"

///----------------------------------------------------------------------------------------------------
/// NameFrom:
/// 	Returns the unlocalized identifier of a game bind.
///----------------------------------------------------------------------------------------------------
std::string& NameFrom(EGameBinds aGameBind);

///----------------------------------------------------------------------------------------------------
/// CategoryNameFrom:
/// 	Returns the unlocalized category of a game bind.
///----------------------------------------------------------------------------------------------------
std::string& CategoryNameFrom(EGameBinds aGameBind);

///----------------------------------------------------------------------------------------------------
/// GameScanCodeToScanCode:
/// 	Converts a scan code from the game to a regular scan code.
///----------------------------------------------------------------------------------------------------
unsigned short GameScanCodeToScanCode(unsigned short aGameScanCode);

///----------------------------------------------------------------------------------------------------
/// ScanCodeToGameScanCode:
/// 	Converts a scan code from the game to a regular scan code.
///----------------------------------------------------------------------------------------------------
unsigned short ScanCodeToGameScanCode(unsigned short aScanCode);

#endif
