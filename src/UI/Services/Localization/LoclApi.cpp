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

#include "Util/Resources.h"
#include "Core/Index/Index.h"
#include "Resources/ResConst.h"
#include "Core/Context.h"
#include "Core/Preferences/PrefConst.h"

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

CLocalization::CLocalization(CLogApi* aLogger)
{
	assert(aLogger);

	this->Logger = aLogger;

	CContext* ctx = CContext::GetContext();
	CSettings* settingsctx = ctx->GetSettingsCtx();
	CEventApi* evtapi = ctx->GetEventApi();

	this->SetLocaleDirectory(Index(EPath::DIR_LOCALES));
	Resources::Unpack(ctx->GetModule(), Index(EPath::LocaleEN), RES_LOCALE_EN, "JSON");
	Resources::Unpack(ctx->GetModule(), Index(EPath::LocaleDE), RES_LOCALE_DE, "JSON");
	Resources::Unpack(ctx->GetModule(), Index(EPath::LocaleFR), RES_LOCALE_FR, "JSON");
	Resources::Unpack(ctx->GetModule(), Index(EPath::LocaleES), RES_LOCALE_ES, "JSON");
	Resources::Unpack(ctx->GetModule(), Index(EPath::LocaleBR), RES_LOCALE_BR, "JSON");
	Resources::Unpack(ctx->GetModule(), Index(EPath::LocaleCZ), RES_LOCALE_CZ, "JSON");
	Resources::Unpack(ctx->GetModule(), Index(EPath::LocaleIT), RES_LOCALE_IT, "JSON");
	Resources::Unpack(ctx->GetModule(), Index(EPath::LocalePL), RES_LOCALE_PL, "JSON");
	Resources::Unpack(ctx->GetModule(), Index(EPath::LocaleRU), RES_LOCALE_RU, "JSON");
	Resources::Unpack(ctx->GetModule(), Index(EPath::LocaleCN), RES_LOCALE_CN, "JSON");

	std::string lang = settingsctx->Get<std::string>(OPT_LANGUAGE, "en");
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
	/* Directory not set. */
	if (this->Directory.empty())
	{
		return;
	}

	/* Clear existing atlas. */
	this->ClearLocaleAtlas();

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
		catch (json::parse_error& ex)
		{
			Logger->Warning(CH_LOCALIZATION, "%s could not be parsed. Error: %s", path.filename().string().c_str(), ex.what());
		}
	}

	this->IsLocaleAtlasBuilt = true;
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
