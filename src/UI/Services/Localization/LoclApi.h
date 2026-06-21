///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LoclApi.h
/// Description  :  Handles localization of strings.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <mutex>
#include <filesystem>
#include <map>
#include <string>

#include "Engine/Logging/LogApi.h"
#include "LoclLocale.h"
#include "LoclQueuedText.h"

constexpr const char* CH_LOCALIZATION = "Localization";

///----------------------------------------------------------------------------------------------------
/// CLocalization Class
///----------------------------------------------------------------------------------------------------
class CLocalization
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// OnUELanguageChanged:
	/// 	Receives runtime language updates from unofficial extras.
	///----------------------------------------------------------------------------------------------------
	static void OnUELanguageChanged(uint32_t* aLanguage);

	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CLocalization(CLogApi* aLogger);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CLocalization();

	///----------------------------------------------------------------------------------------------------
	/// Advance:
	/// 	Processes new strings and adds them to the atlas.
	/// 	Returns true if the atlas was modified.
	///----------------------------------------------------------------------------------------------------
	bool Advance();

	///----------------------------------------------------------------------------------------------------
	/// Translate:
	/// 	Returns the translated string with the given identifier and language.
	/// 	If no language is specified, the currently set one will be used.
	///----------------------------------------------------------------------------------------------------
	const char* Translate(const char* aIdentifier, const char* aLanguageIdentifier = nullptr);

	///----------------------------------------------------------------------------------------------------
	/// Set:
	/// 	Adds or sets/overrides a localized string with a given identifier.
	///----------------------------------------------------------------------------------------------------
	void Set(const char* aIdentifier, const char* aLanguageIdentifier, const char* aText);

	///----------------------------------------------------------------------------------------------------
	/// SetLanguage:
	/// 	Sets the currently active language.
	///----------------------------------------------------------------------------------------------------
	void SetLanguage(const std::string& aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// GetLanguages:
	/// 	Gets the all available languages.
	///----------------------------------------------------------------------------------------------------
	std::vector<std::string> GetLanguages();

	///----------------------------------------------------------------------------------------------------
	/// GetActiveLanguage:
	/// 	Get the currently active language.
	///----------------------------------------------------------------------------------------------------
	const std::string& GetActiveLanguage();

	///----------------------------------------------------------------------------------------------------
	/// SetLocaleDirectory:
	/// 	Sets the directory from which the LocaleAtlas is built.
	///----------------------------------------------------------------------------------------------------
	void SetLocaleDirectory(std::filesystem::path aPath);

	///----------------------------------------------------------------------------------------------------
	/// GetAllTexts:
	/// 	Returns every single string.
	///----------------------------------------------------------------------------------------------------
	std::vector<const char*> GetAllTexts();

	private:
	CLogApi*                         Logger   = nullptr;

	uint32_t                         ThreadID = 0;

	std::filesystem::path            Directory;
	bool                             IsLocaleAtlasBuilt = false;
	std::map<std::string, Locale_t>  LocaleAtlas; /* Identifier(e.g. "EN-GB") maps to object with display name("English (United Kingdom)") and a map of all its texts*/

	Locale_t*                        ActiveLocale = nullptr;

	std::vector<QueuedText_t>        QueuedTexts;
	std::string                      QueuedLanguage;

	///----------------------------------------------------------------------------------------------------
	/// BuildLocaleAtlas:
	/// 	Builds the LocaleAtlas, if the directory is set.
	///----------------------------------------------------------------------------------------------------
	void BuildLocaleAtlas();

	///----------------------------------------------------------------------------------------------------
	/// ClearLocaleAtlas:
	/// 	Clears the LocaleAtlas.
	///----------------------------------------------------------------------------------------------------
	void ClearLocaleAtlas();
};
