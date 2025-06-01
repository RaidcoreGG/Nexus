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
/// 	Helper function to create a InputBind_t from a string.
///----------------------------------------------------------------------------------------------------
InputBind_t IBFromString(std::string aInputBind);

///----------------------------------------------------------------------------------------------------
/// IBToString:
/// 	Helper function to get the display string of a InputBind_t.
///----------------------------------------------------------------------------------------------------
std::string IBToString(const InputBind_t& aKebyind, bool aPadded = false);

#endif
