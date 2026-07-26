///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LoclApi.h
/// Description  :  Handles localization of strings.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <filesystem>
#include <map>
#include <mutex>
#include <string>

#include "Core/Logging/LogApi.h"
#include "Core/Settings/SettingsMgr.h"
#include "Host/Events/EvtApi.h"
#include "LoclLocale.h"
#include "LoclQueuedText.h"

constexpr const char* CH_LOCALIZATION = "Localization";

///----------------------------------------------------------------------------------------------------
/// Raidcore::Nexus::GUI Namespace
///----------------------------------------------------------------------------------------------------
namespace Raidcore::Nexus::GUI
{
	///----------------------------------------------------------------------------------------------------
	/// Localization Class
	///----------------------------------------------------------------------------------------------------
	class Localization
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
		Localization(Core::LogApi& aLogger, Core::SettingsMgr& aSettings, Host::EventApi& aEventApi);

		///----------------------------------------------------------------------------------------------------
		/// dtor
		///----------------------------------------------------------------------------------------------------
		~Localization();

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
		Core::LogApi&      Logger;
		Core::SettingsMgr& Settings;
		Host::EventApi&    EventApi;

		uint32_t                         ThreadID = 0;

		std::filesystem::path            Directory;
		bool                             IsLocaleAtlasBuilt = false;
		std::map<std::string, Locale_t>  LocaleAtlas; /* Identifier(e.g. "EN-GB") maps to object with display name("English (United Kingdom)") and a map of all its texts*/

		Locale_t* ActiveLocale = nullptr;

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
}
