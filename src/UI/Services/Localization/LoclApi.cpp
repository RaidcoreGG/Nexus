///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LoclApi.cpp
/// Description  :  Handles localization of strings.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "LoclApi.h"

#include <fstream>
#include <windows.h>

#pragma warning(push, 0)
#include "nlohmann/json.hpp"
#pragma warning(pop)
using json = nlohmann::json;

#include "Host/GW2/Index/Index.h"
#include "res/Resources.h"
#include "Host/GW2/Context.h"

using Raidcore::Core::Platform::Windows::CPackFile;

static CLocalization* s_Localization{};

/*static*/ void CLocalization::OnUELanguageChanged(uint32_t* aLanguage)
{
	if (!aLanguage) { return; }
	if (!s_Localization) { return; }

	switch (*aLanguage)
	{
		case 0:
		{
			s_Localization->SetLanguage("en");
			break;
		}
		case 1:
		{
			s_Localization->SetLanguage("kr");
			break;
		}
		case 2:
		{
			s_Localization->SetLanguage("fr");
			break;
		}
		case 3:
		{
			s_Localization->SetLanguage("de");
			break;
		}
		case 4:
		{
			s_Localization->SetLanguage("es");
			break;
		}
		case 5:
		{
			s_Localization->SetLanguage("cn");
			break;
		}
	}
}

CLocalization::CLocalization()
{
	CContext* ctx = CContext::GetContext();
	CEventApi* evtapi = ctx->GetEventApi();

	/* Older versions unpacked the embedded locales to disk. Remove them so stale copies do not override the embedded ones. */
	for (EPath legacy : { EPath::LocaleEN, EPath::LocaleDE, EPath::LocaleFR, EPath::LocaleES, EPath::LocaleCN, EPath::LocaleKR,
	                      EPath::LocaleBR, EPath::LocaleCZ, EPath::LocaleIT, EPath::LocalePL, EPath::LocaleRU })
	{
		std::error_code ec;
		std::filesystem::remove(Index(legacy), ec);
	}

	this->SetLocaleDirectory(Index(EPath::DIR_LOCALES));

	std::string lang = this->Language.Get();
	this->SetLanguage(!lang.empty() ? lang : "en");

	evtapi->Subscribe("EV_UNOFFICIAL_EXTRAS_LANGUAGE_CHANGED", reinterpret_cast<EVENT_CONSUME>(CLocalization::OnUELanguageChanged));

	s_Localization = this;
}

CLocalization::~CLocalization()
{
	this->ClearLocaleAtlas();
}

bool CLocalization::Advance()
{
	bool didModify = false;

	/* Set thread ID. We may only call Advance from the UI thread. */
	if (this->ThreadID == 0)
	{
		this->ThreadID = GetCurrentThreadId();
	}

	assert(this->ThreadID == GetCurrentThreadId());

	if (!this->IsLocaleAtlasBuilt)
	{
		this->BuildLocaleAtlas();
		didModify = true;
	}

	/* Process queued texts. */
	while (this->QueuedTexts.size() > 0)
	{
		QueuedText_t& item = this->QueuedTexts.front();
		
		auto atlasIt = this->LocaleAtlas.find(item.LanguageIdentifier);

		if (atlasIt != this->LocaleAtlas.end())
		{
			auto textIt = atlasIt->second.Texts.find(item.Identifier);

			if (textIt != atlasIt->second.Texts.end())
			{
				/* Sanity check. Free the existing string. */
				if (textIt->second != nullptr)
				{
					free((void*)textIt->second);
				}

				textIt->second = _strdup(item.Text.c_str());
			}
			else
			{
				atlasIt->second.Texts.emplace(item.Identifier, _strdup(item.Text.c_str()));
			}
		}

		this->QueuedTexts.erase(this->QueuedTexts.begin());

		didModify = true;
	}

	/* Process queued language. */
	if (!this->QueuedLanguage.empty())
	{
		std::string newlang = this->QueuedLanguage;
		this->QueuedLanguage.clear();

		auto atlasIt = this->LocaleAtlas.find(newlang);

		/* Find it via short identifier. */
		if (atlasIt != this->LocaleAtlas.end())
		{
			this->ActiveLocale = &atlasIt->second;
		}
		else /* Find it via display name. */
		{
			for (auto& it : this->LocaleAtlas)
			{
				if (it.second.DisplayName == newlang)
				{
					this->ActiveLocale = &it.second;
					break;
				}
			}
		}

		assert(this->ActiveLocale);
	}

	return didModify;
}

const char* CLocalization::Translate(const char* aIdentifier, const char* aLanguageIdentifier)
{
	if (aIdentifier == nullptr) { return aIdentifier; }

	if (!this->IsLocaleAtlasBuilt)
	{
		/* Cannot translate, if atlas was never built. */
		return aIdentifier;
	}

	std::string identifier = aIdentifier;

	/* If language identifier was passed, translate from that. */
	if (aLanguageIdentifier)
	{
		std::string languageIdentifier = aLanguageIdentifier;

		auto atlasIt = this->LocaleAtlas.find(languageIdentifier);

		if (atlasIt != this->LocaleAtlas.end())
		{
			auto it = atlasIt->second.Texts.find(identifier);

			if (it != atlasIt->second.Texts.end())
			{
				return it->second;
			}
		}
	}

	/* If no language identifier was passed, or the text does not exist in it, search active locale. */
	if (this->ActiveLocale)
	{
		auto it = this->ActiveLocale->Texts.find(identifier);

		if (it != this->ActiveLocale->Texts.end())
		{
			return it->second;
		}
	}

	/* If not found in active locale, and active locale is not english, try to search in English. */
	if (!this->ActiveLocale || this->ActiveLocale->DisplayName != "English")
	{
		auto atlasIt = this->LocaleAtlas.find("en");

		if (atlasIt != this->LocaleAtlas.end())
		{
			auto it = atlasIt->second.Texts.find(identifier);

			if (it != atlasIt->second.Texts.end())
			{
				return it->second;
			}
		}
	}

	/* Text not found in specified language, active locale or English. */
	return aIdentifier;
}

void CLocalization::Set(const char* aIdentifier, const char* aLanguageIdentifier, const char* aText)
{
	if (!aIdentifier)         { return; }
	if (!aLanguageIdentifier) { return; }
	if (!aText)               { return; }

	this->QueuedTexts.push_back({ aIdentifier, aLanguageIdentifier, aText });
}

void CLocalization::SetLanguage(const std::string& aIdentifier)
{
	this->QueuedLanguage = aIdentifier;
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
	assert(this->ActiveLocale);
	return this->ActiveLocale->DisplayName;
}

void CLocalization::SetLocaleDirectory(std::filesystem::path aPath)
{
	this->Directory = aPath;
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

void CLocalization::BuildLocaleAtlas()
{
	/* Clear existing atlas. */
	this->ClearLocaleAtlas();

	static constexpr std::pair<const char*, const CPackFile*> s_EmbeddedLocales[] = {
		{ "en/DisplayStrings", &Resources::LocaleDisplayStringsEN }, { "en/GameBinds", &Resources::LocaleGameBindsEN }, { "en/InputBinds", &Resources::LocaleInputBindsEN },
		{ "de/DisplayStrings", &Resources::LocaleDisplayStringsDE }, { "de/GameBinds", &Resources::LocaleGameBindsDE }, { "de/InputBinds", &Resources::LocaleInputBindsDE },
		{ "fr/DisplayStrings", &Resources::LocaleDisplayStringsFR }, { "fr/GameBinds", &Resources::LocaleGameBindsFR }, { "fr/InputBinds", &Resources::LocaleInputBindsFR },
		{ "es/DisplayStrings", &Resources::LocaleDisplayStringsES }, { "es/GameBinds", &Resources::LocaleGameBindsES }, { "es/InputBinds", &Resources::LocaleInputBindsES },
		{ "cn/DisplayStrings", &Resources::LocaleDisplayStringsCN }, { "cn/GameBinds", &Resources::LocaleGameBindsCN }, { "cn/InputBinds", &Resources::LocaleInputBindsCN },
		{ "kr/DisplayStrings", &Resources::LocaleDisplayStringsKR }, { "kr/GameBinds", &Resources::LocaleGameBindsKR }, { "kr/InputBinds", &Resources::LocaleInputBindsKR },
		{ "br/DisplayStrings", &Resources::LocaleDisplayStringsBR },                                                               { "br/InputBinds", &Resources::LocaleInputBindsBR },
		{ "cz/DisplayStrings", &Resources::LocaleDisplayStringsCZ },                                                               { "cz/InputBinds", &Resources::LocaleInputBindsCZ },
		{ "it/DisplayStrings", &Resources::LocaleDisplayStringsIT }, { "it/GameBinds", &Resources::LocaleGameBindsIT }, { "it/InputBinds", &Resources::LocaleInputBindsIT },
		{ "pl/DisplayStrings", &Resources::LocaleDisplayStringsPL },                                                               { "pl/InputBinds", &Resources::LocaleInputBindsPL },
		{ "ru/DisplayStrings", &Resources::LocaleDisplayStringsRU },                                                               { "ru/InputBinds", &Resources::LocaleInputBindsRU }
	};

	for (const auto& [name, packFile] : s_EmbeddedLocales)
	{
		std::span<const uint8_t> data = packFile->GetBytes();

		if (data.empty())
		{
			LogWarning(CH_LOCALIZATION, "Embedded locale \"%s\" could not be loaded.", name);
			continue;
		}

		this->MergeLocale(data, std::string("Embedded locale ") + name);
	}

	/* Files on disk are merged after the embedded locales, so they can add languages or override texts. */
	if (!this->Directory.empty() && std::filesystem::exists(this->Directory))
	{
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

			std::ifstream file(path, std::ios::binary);
			std::vector<uint8_t> content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			file.close();

			this->MergeLocale(content, path.filename().string());
		}
	}

	this->IsLocaleAtlasBuilt = true;
}

void CLocalization::MergeLocale(std::span<const uint8_t> aData, const std::string& aSourceName)
{
	try
	{
		json localeJson = json::parse(aData.begin(), aData.end());

		if (localeJson.is_null())
		{
			return;
		}

		if (localeJson["Identifier"].is_null())
		{
			return;
		}

		if (localeJson["Texts"].is_null())
		{
			return;
		}

		std::string locId = localeJson["Identifier"].get<std::string>();

		auto atlasIt = this->LocaleAtlas.find(locId);

		Locale_t loc{};
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

			auto textIt = loc.Texts.find(key);

			/* if a value is already set, clear it. only used when merging */
			if (textIt != loc.Texts.end())
			{
				/* Sanity check. Free the existing string. */
				if (textIt->second != nullptr)
				{
					free((void*)textIt->second);
				}

				textIt->second = _strdup(value.get<std::string>().c_str());
			}
			else
			{
				loc.Texts.emplace(key, _strdup(value.get<std::string>().c_str()));
			}
		}

		if (atlasIt != this->LocaleAtlas.end())
		{
			atlasIt->second = loc;
		}
		else
		{
			this->LocaleAtlas.emplace(locId, loc);
		}
	}
	catch (json::exception& ex)
	{
		LogWarning(CH_LOCALIZATION, "%s could not be parsed. Error: %s", aSourceName.c_str(), ex.what());
	}
}

void CLocalization::ClearLocaleAtlas()
{
	if (!this->IsLocaleAtlasBuilt)
	{
		return;
	}

	/* delete strings */
	for (auto& locale : this->LocaleAtlas)
	{
		for (auto& text : locale.second.Texts)
		{
			/* Sanity check. Free the existing string. */
			if (text.second != nullptr)
			{
				free((void*)text.second);
			}
		}
	}

	this->IsLocaleAtlasBuilt = false;
}
