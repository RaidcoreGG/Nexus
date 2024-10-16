///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Localization.cpp
/// Description  :  Handles localization of strings.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Localization.h"

#include <fstream>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "Consts.h"

CLocalization::CLocalization(CLogHandler* aLogger)
{
	assert(aLogger);

	this->Logger = aLogger;
}

CLocalization::~CLocalization()
{
	this->ClearLocaleAtlas();
}

bool CLocalization::Advance()
{
	bool didModify = false;

	if (!this->IsLocaleAtlasBuilt)
	{
		BuildLocaleAtlas();
		didModify = true;
	}

	const std::lock_guard<std::mutex> lock(this->Mutex);

	while (this->Queued.size() > 0)
	{
		QueuedTranslation& item = this->Queued.front();
		
		auto atlasIt = this->LocaleAtlas.find(item.LanguageIdentifier);

		if (atlasIt != this->LocaleAtlas.end())
		{
			atlasIt->second.Texts[item.Identifier] = _strdup(item.Text.c_str());
		}

		this->Queued.erase(this->Queued.begin());

		didModify = true;
	}

	return didModify;
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

void CLocalization::Set(const char* aIdentifier, const char* aLanguageIdentifier, const char* aString)
{
	if (!(aIdentifier && aLanguageIdentifier && aString))
	{
		return;
	}

	this->Queued.push_back({ aIdentifier, aLanguageIdentifier, aString });
}

void CLocalization::SetLanguage(const std::string& aIdentifier)
{
	auto atlasIt = this->LocaleAtlas.find(aIdentifier);

	/* find via identifier */
	if (atlasIt != this->LocaleAtlas.end())
	{
		this->ActiveLocale = &atlasIt->second;
		return;
	}

	/* find via display name */
	for (auto& it : this->LocaleAtlas)
	{
		if (it.second.DisplayName == aIdentifier)
		{
			this->ActiveLocale = &it.second;
			return;
		}
	}

	this->ActiveLocale = nullptr;
}

std::vector<std::string> CLocalization::GetLanguages()
{
	std::vector<std::string> langs;

	for (auto& it : this->LocaleAtlas)
	{
		langs.push_back(it.second.DisplayName);
	}

	return langs;
}

const std::string& CLocalization::GetActiveLanguage()
{
	if (this->ActiveLocale)
	{
		return this->ActiveLocale->DisplayName;
	}

	return NULLSTR;
}

void CLocalization::SetLocaleDirectory(std::filesystem::path aPath)
{
	this->Directory = aPath;
	this->IsLocaleDirectorySet = true;
}

void CLocalization::BuildLocaleAtlas()
{
	if (!IsLocaleDirectorySet)
	{
		return;
	}

	/* Clear existing atlas */
	ClearLocaleAtlas();

	const std::lock_guard<std::mutex> lock(this->Mutex);
	/* find files, merge files, alloc strings */
	for (const std::filesystem::directory_entry entry : std::filesystem::directory_iterator(this->Directory))
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

			auto atlasIt = this->LocaleAtlas.find(locId);

			Locale loc{};
			if (atlasIt != this->LocaleAtlas.end())
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

			this->LocaleAtlas[locId] = loc;
		}
		catch (json::parse_error& ex)
		{
			Logger->Warning(CH_LOCALIZATION, "%s could not be parsed. Error: %s", path.filename().string().c_str(), ex.what());
		}
	}

	this->IsLocaleAtlasBuilt = true;

	if (this->ActiveLocale)
	{
		this->SetLanguage(this->ActiveLocale->DisplayName);
	}
}

void CLocalization::ClearLocaleAtlas()
{
	if (!this->IsLocaleAtlasBuilt)
	{
		return;
	}

	const std::lock_guard<std::mutex> lock(this->Mutex);

	/* delete strings */
	for (auto& locale : this->LocaleAtlas)
	{
		for (auto& text : locale.second.Texts)
		{
			free((char*)text.second);
		}
	}

	this->IsLocaleAtlasBuilt = false;
}

std::vector<const char*> CLocalization::GetAllTexts()
{
	std::vector<const char*> allTexts;

	for (auto& [atlasId, atlasLocale] : this->LocaleAtlas)
	{
		for (auto& [textId, textVal] : atlasLocale.Texts)
		{
			allTexts.push_back(textVal);
		}
	}

	return allTexts;
}
