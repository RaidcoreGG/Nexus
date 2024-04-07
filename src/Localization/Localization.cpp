///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Localization.cpp
/// Description  :  Handles localization of strings.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Localization.h"

#include "Consts.h"
#include "Shared.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace Localization
{
	const char* ADDONAPI_Translate(const char* aIdentifier)
	{
		CLocalization& inst = CLocalization::GetInstance();
		return inst.Translate(aIdentifier);
	}

	const char* ADDONAPI_TranslateTo(const char* aIdentifier, const char* aLanguageIdentifier)
	{
		CLocalization& inst = CLocalization::GetInstance();
		return inst.Translate(aIdentifier, aLanguageIdentifier);
	}

	/*static void ADDONAPI_Set(const char* aIdentifier, const char* aLanguageIdentifier, const char* aString)
	{
		CLocalization& inst = CLocalization::GetInstance();
		inst.Set(aIdentifier, aLanguageIdentifier, aString);
	}*/
}

CLocalization& CLocalization::GetInstance()
{
	static CLocalization Instance;
	return Instance;
}

const char* CLocalization::Translate(const char* aIdentifier, const char* aLanguageIdentifier)
{
	if (!IsLocaleAtlasBuilt)
	{
		/* atlas was never built, cannot localize */
		return aIdentifier;
	}

	std::string identifier = aIdentifier;

	if (aLanguageIdentifier)
	{
		std::string languageIdentifier = aLanguageIdentifier;

		auto atlasIt = LocaleAtlas.find(languageIdentifier);

		if (atlasIt != LocaleAtlas.end())
		{
			auto it = atlasIt->second.Texts.find(identifier);

			if (it != atlasIt->second.Texts.end())
			{
				return it->second;
			}
		}
	}

	if (ActiveLocale)
	{
		auto it = ActiveLocale->Texts.find(identifier);

		if (it != ActiveLocale->Texts.end())
		{
			return it->second;
		}
	}

	if (!ActiveLocale || ActiveLocale->DisplayName != "English")
	{
		auto atlasIt = LocaleAtlas.find("en");

		if (atlasIt != LocaleAtlas.end())
		{
			auto it = atlasIt->second.Texts.find(identifier);

			if (it != atlasIt->second.Texts.end())
			{
				return it->second;
			}
		}
	}

	/* no active language set and no language specified, cannot localize */
	return aIdentifier;
}

/*void CLocalization::Set(const char* aIdentifier, const char* aLanguageIdentifier, const char* aString)
{
	if (!(aIdentifier && aLanguageIdentifier && aString))
	{
		return;
	}

	std::string identifier = aIdentifier;
	std::string languageIdentifier = aLanguageIdentifier;

	if (aLanguageIdentifier)
	{
		std::string languageIdentifier = aLanguageIdentifier;

		auto atlasIt = LocaleAtlas.find(languageIdentifier);

		if (atlasIt != LocaleAtlas.end())
		{
			atlasIt->second.Texts[identifier] = _strdup(aString);
		}
	}
}*/

void CLocalization::SetLanguage(const std::string& aIdentifier)
{
	auto atlasIt = LocaleAtlas.find(aIdentifier);

	/* find via identifier */
	if (atlasIt != LocaleAtlas.end())
	{
		ActiveLocale = &atlasIt->second;
		return;
	}

	/* find via display name */
	for (auto& it : LocaleAtlas)
	{
		if (it.second.DisplayName == aIdentifier)
		{
			ActiveLocale = &it.second;
			return;
		}
	}

	ActiveLocale = nullptr;
}

std::vector<std::string> CLocalization::GetLanguages()
{
	std::vector<std::string> langs;

	for (auto& it : LocaleAtlas)
	{
		langs.push_back(it.second.DisplayName);
	}

	return langs;
}

const std::string& CLocalization::GetActiveLanguage()
{
	if (ActiveLocale)
	{
		return ActiveLocale->DisplayName;
	}

	return "(null)";
}

void CLocalization::SetLocaleDirectory(std::filesystem::path aPath)
{
	Directory = aPath;
	IsLocaleDirectorySet = true;
}

void CLocalization::BuildLocaleAtlas()
{
	if (!IsLocaleDirectorySet)
	{
		return;
	}

	/* Clear existing atlas */
	ClearLocaleAtlas();

	const std::lock_guard<std::mutex> lock(Mutex);
	/* find files, merge files, alloc strings */
	for (const std::filesystem::directory_entry entry : std::filesystem::directory_iterator(Directory))
	{
		std::filesystem::path path = entry.path();

		if (path.extension() != ".json")
		{
			continue;
		}

		if (std::filesystem::file_size(path) == 0)
		{
			continue;
		}

		try
		{
			std::ifstream file(path);
			json localeJson = json::parse(file);
			file.close();

			if (localeJson.is_null())
			{
				continue;
			}

			if (localeJson["Identifier"].is_null())
			{
				continue;
			}

			if (localeJson["Texts"].is_null())
			{
				continue;
			}

			std::string locId = localeJson["Identifier"].get<std::string>();;

			auto atlasIt = LocaleAtlas.find(locId);

			Locale loc{};
			if (atlasIt != LocaleAtlas.end())
			{
				loc = atlasIt->second;
			}

			/* DisplayName can be null, hopefully *any* of the files have it set, if not fallback to identifier */
			if (!localeJson["DisplayName"].is_null() && loc.DisplayName.empty())
			{
				localeJson["DisplayName"].get_to(loc.DisplayName);
			}

			for (auto& [key, value] : localeJson["Texts"].items())
			{
				if (value.is_null() || !value.is_string())
				{
					continue;
				}

				auto txtIt = loc.Texts.find(key);

				/* if a value is already set, clear it. only used when merging */
				if (txtIt != loc.Texts.end())
				{
					free((char*)txtIt->second);
					txtIt->second = nullptr;
				}

				loc.Texts[key] = _strdup(value.get<std::string>().c_str());
			}

			LocaleAtlas[locId] = loc;
		}
		catch (json::parse_error& ex)
		{
			LogWarning(CH_LOCALIZATION, "%s could not be parsed. Error: %s", path.filename().string().c_str(), ex.what());
		}
	}

	IsLocaleAtlasBuilt = true;
}

void CLocalization::ClearLocaleAtlas()
{
	if (!IsLocaleAtlasBuilt)
	{
		return;
	}

	const std::lock_guard<std::mutex> lock(Mutex);

	/* delete strings */
	for (auto& locale : LocaleAtlas)
	{
		for (auto& text : locale.second.Texts)
		{
			free((char*)text.second);
		}
	}

	IsLocaleAtlasBuilt = false;
}
