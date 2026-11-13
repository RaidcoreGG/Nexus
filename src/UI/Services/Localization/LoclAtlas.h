///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LoclAtlas.h
/// Description  :  Texts of one source (the locale files, or one addon), per language.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <map>
#include <span>
#include <string>
#include <unordered_map>

///----------------------------------------------------------------------------------------------------
/// LocaleTexts_t Struct
/// 	Texts of one language from one source.
///----------------------------------------------------------------------------------------------------
struct LocaleTexts_t
{
	std::string                                  DisplayName;
	std::unordered_map<std::string, std::string> Texts;
};

///----------------------------------------------------------------------------------------------------
/// LocaleAtlas_t
/// 	Texts of one source, keyed by language id (e.g. "en").
///----------------------------------------------------------------------------------------------------
using LocaleAtlas_t = std::map<std::string, LocaleTexts_t, std::less<>>;

///----------------------------------------------------------------------------------------------------
/// MergeLocaleFile:
/// 	Parses a locale file and merges its texts into the atlas; later files replace earlier texts.
/// 	Nested objects become dotted keys: { "QuickAccess": { "Visibility": ... } } is "QuickAccess.Visibility".
/// 	Logs and returns false if the file cannot be parsed.
///----------------------------------------------------------------------------------------------------
bool MergeLocaleFile(LocaleAtlas_t& aAtlas, std::span<const uint8_t> aData, const std::string& aSourceName, const std::string& aLogChannel);
