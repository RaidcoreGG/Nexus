///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Localization.h
/// Description  :  Handles localization of strings.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef LOCALIZATION_H
#define LOCALIZATION_H

#include <mutex>
#include <filesystem>
#include <map>
#include <string>

#include "Services/Logging/LogApi.h"
#include "Events/EvtApi.h"

constexpr const char* CH_LOCALIZATION = "Localization";

///----------------------------------------------------------------------------------------------------
/// Locale_t data struct
///----------------------------------------------------------------------------------------------------
struct Locale_t
{
	std::string DisplayName;
	std::map<std::string, const char*> Texts;
};

///----------------------------------------------------------------------------------------------------
/// QueuedTranslation_t data struct
///----------------------------------------------------------------------------------------------------
struct QueuedTranslation_t
{
	std::string Identifier;
	std::string LanguageIdentifier;
	std::string Text;
};

///----------------------------------------------------------------------------------------------------
/// CLocalization Class
///----------------------------------------------------------------------------------------------------
class CLocalization
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CLocalization(CLogApi* aLogger, CEventApi* aEventApi);
	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CLocalization();

	///----------------------------------------------------------------------------------------------------
	/// Advance:
	/// 	Processes new strings and adds them to the atlas.
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
	void Set(const char* aIdentifier, const char* aLanguageIdentifier, const char* aString);

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
	/// BuildLocaleAtlas:
	/// 	Builds the LocaleAtlas, if the directory is set.
	///----------------------------------------------------------------------------------------------------
	void BuildLocaleAtlas();

	///----------------------------------------------------------------------------------------------------
	/// ClearLocaleAtlas:
	/// 	Clears the LocaleAtlas.
	///----------------------------------------------------------------------------------------------------
	void ClearLocaleAtlas();

	///----------------------------------------------------------------------------------------------------
	/// GetAllTexts:
	/// 	Returns every single string.
	///----------------------------------------------------------------------------------------------------
	std::vector<const char*> GetAllTexts();

	private:
	CLogApi*                       Logger   = nullptr;
	CEventApi*                     EventApi = nullptr;

	std::mutex                     Mutex;
	std::filesystem::path          Directory;
	// Identifier (e.g. "EN-GB") maps to object with display name ("English (United Kingdom)") and a map of all its texts
	std::map<std::string, Locale_t>  LocaleAtlas;
	std::vector<QueuedTranslation_t> Queued;

	Locale_t* ActiveLocale = nullptr;

	bool IsLocaleDirectorySet = false;
	bool IsLocaleAtlasBuilt = false;
};

#endif
