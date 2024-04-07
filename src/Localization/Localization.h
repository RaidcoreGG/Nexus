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

namespace Localization
{
	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_Translate:
	/// 	Addon API wrapper function for Translate. Translates into the currently set language.
	///----------------------------------------------------------------------------------------------------
	const char* ADDONAPI_Translate(const char* aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_TranslateTo:
	/// 	Addon API wrapper function for Translate. Translates into a specific language.
	///----------------------------------------------------------------------------------------------------
	const char* ADDONAPI_TranslateTo(const char* aIdentifier, const char* aLanguageIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_Set:
	/// 	Addon API wrapper function to set translated strings.
	///----------------------------------------------------------------------------------------------------
	/*static void ADDONAPI_Set(const char* aIdentifier, const char* aLanguageIdentifier, const char* aString);*/
}

///----------------------------------------------------------------------------------------------------
/// Locale data struct
///----------------------------------------------------------------------------------------------------
struct Locale
{
	std::string DisplayName;
	std::map<std::string, const char*> Texts;
};

///----------------------------------------------------------------------------------------------------
/// CLocalization Singleton
///----------------------------------------------------------------------------------------------------
class CLocalization
{
public:
	///----------------------------------------------------------------------------------------------------
	/// GetInstance:
	/// 	Returns a reference to an instance of CLocalization.
	///----------------------------------------------------------------------------------------------------
	static CLocalization& GetInstance();

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
	/*void Set(const char* aIdentifier, const char* aLanguageIdentifier, const char* aString);*/

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

	CLocalization(CLocalization const&)		= delete;
	void operator=(CLocalization const&)	= delete;

private:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CLocalization() {}
	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CLocalization() { ClearLocaleAtlas(); }

	std::mutex								Mutex;
	std::filesystem::path					Directory;
	// Identifier (e.g. "EN-GB") maps to object with display name ("English (United Kingdom)") and a map of all its texts
	std::map<std::string, Locale>			LocaleAtlas;

	Locale* ActiveLocale					= nullptr;

	bool IsLocaleDirectorySet				= false;
	bool IsLocaleAtlasBuilt					= false;
};

#endif
