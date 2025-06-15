///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LdrConst.h
/// Description  :  Constant data for the loader.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef LDRCONST_H
#define LDRCONST_H

#include <cstdint>

constexpr const uint32_t WM_ADDONDIRUPDATE = WM_USER + 101;

constexpr const char* EV_ADDON_LOADED            = "EV_ADDON_LOADED";
constexpr const char* EV_ADDON_UNLOADED          = "EV_ADDON_UNLOADED";
constexpr const char* EV_VOLATILE_ADDON_DISABLED = "EV_VOLATILE_ADDON_DISABLED";

#endif
