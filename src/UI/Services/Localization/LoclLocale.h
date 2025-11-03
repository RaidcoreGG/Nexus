///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LoclLocale.h
/// Description  :  Definition for locale struct.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <string>
#include <unordered_map>

///----------------------------------------------------------------------------------------------------
/// Locale_t Struct
///----------------------------------------------------------------------------------------------------
struct Locale_t
{
	std::string                                  DisplayName;
	std::unordered_map<std::string, const char*> Texts;
};
