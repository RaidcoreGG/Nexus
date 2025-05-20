///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IbConst.h
/// Description  :  Constant data for InputBinds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef IBCONST_H
#define IBCONST_H

#include <string>

#include "IbBindV2.h"

///----------------------------------------------------------------------------------------------------
/// IBFromString:
/// 	Helper function to create a InputBind from a string.
///----------------------------------------------------------------------------------------------------
InputBind IBFromString(std::string aInputBind);

///----------------------------------------------------------------------------------------------------
/// IBToString:
/// 	Helper function to get the display string of a InputBind.
///----------------------------------------------------------------------------------------------------
std::string IBToString(const InputBind& aKebyind, bool aPadded = false);

#endif
