///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LoclAtlas.cpp
/// Description  :  Texts of one source (the locale files, or one addon), per language.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "LoclAtlas.h"

#pragma warning(push, 0)
#include "nlohmann/json.hpp"
#pragma warning(pop)
using json = nlohmann::json;

#include "Core/Logging/LogApi.h"
#include "LoclApi.h"

static void MergeTexts(LocaleTexts_t& aLanguage, const json& aNode, const std::string& aPrefix)
{
	for (auto& [key, value] : aNode.items())
	{
		std::string fullKey = aPrefix.empty() ? key : aPrefix + "." + key;

		if (value.is_object())
		{
			MergeTexts(aLanguage, value, fullKey);
		}
		else if (value.is_string())
		{
			aLanguage.Texts.insert_or_assign(std::move(fullKey), value.get<std::string>());
		}
	}
}

bool MergeLocaleFile(LocaleAtlas_t& aAtlas, std::span<const uint8_t> aData, const std::string& aSourceName, const std::string& aLogChannel)
{
	try
	{
		json file = json::parse(aData.begin(), aData.end());

		if (!file.is_object() || !file["Identifier"].is_string() || !file["Texts"].is_object())
		{
			LogWarning(aLogChannel, "%s is not a locale file: \"Identifier\" or \"Texts\" is missing.", aSourceName.c_str());
			return false;
		}

		LocaleTexts_t& language = aAtlas[file["Identifier"].get<std::string>()];

		/* The first file of a language that names it wins. */
		if (language.DisplayName.empty() && file["DisplayName"].is_string())
		{
			language.DisplayName = file["DisplayName"].get<std::string>();
		}

		MergeTexts(language, file["Texts"], "");

		return true;
	}
	catch (const json::exception& ex)
	{
		LogWarning(aLogChannel, "%s could not be parsed. Error: %s", aSourceName.c_str(), ex.what());
		return false;
	}
}
