///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LoclApi.h
/// Description  :  Handles localization of strings.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "Core/Logging/LogChannel.h"
#include "Core/Preferences/PrefSetting.h"
#include "Engine/Cleanup/RefCleanerBase.h"
#include "Engine/Clockwork/Clockwork.h"
#include "LoclAtlas.h"
#include "LoclQueuedText.h"
#include "LoclText.h"
#include "LoclView.h"


///----------------------------------------------------------------------------------------------------
/// CLocalization Class
/// 	Sources: the locale files, then one atlas per addon that set texts, in the order they first did.
/// 	A view merges them; it is built on a worker and replaces the current one in Advance when ready.
///----------------------------------------------------------------------------------------------------
class CLocalization : public ILogChannel, public virtual IRefCleaner
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
	CLocalization();

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CLocalization();

	///----------------------------------------------------------------------------------------------------
	/// Advance:
	/// 	Applies queued changes, swaps in finished files and views and starts the next load or build. UI thread only.
	/// 	Returns true if the texts changed.
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
	/// 	aCaller is an address inside the calling module; its texts are removed when that module unloads.
	///----------------------------------------------------------------------------------------------------
	void Set(const char* aIdentifier, const char* aLanguageIdentifier, const char* aText, void* aCaller = nullptr);

	///----------------------------------------------------------------------------------------------------
	/// SetLanguage:
	/// 	Sets the currently active language, by id or display name.
	///----------------------------------------------------------------------------------------------------
	void SetLanguage(const std::string& aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// GetLanguages:
	/// 	Gets the display names of all available languages. UI thread only.
	///----------------------------------------------------------------------------------------------------
	std::vector<std::string> GetLanguages();

	///----------------------------------------------------------------------------------------------------
	/// GetActiveLanguage:
	/// 	Gets the display name of the active language. UI thread only.
	///----------------------------------------------------------------------------------------------------
	const std::string& GetActiveLanguage();

	///----------------------------------------------------------------------------------------------------
	/// SetLocaleDirectory:
	/// 	Sets the directory with the user's locale files and reloads the files.
	///----------------------------------------------------------------------------------------------------
	void SetLocaleDirectory(std::filesystem::path aPath);

	///----------------------------------------------------------------------------------------------------
	/// GetActiveTexts:
	/// 	Returns the texts shown in the active language (with their English fallbacks) and every language
	/// 	display name, e.g. to collect the glyphs the fonts need.
	/// 	UI thread only; valid until the next Advance that returns true.
	///----------------------------------------------------------------------------------------------------
	std::vector<const char*> GetActiveTexts();

	///----------------------------------------------------------------------------------------------------
	/// CleanupRefs:
	/// 	Removes the texts set by a module in the address range with the next Advance.
	/// 	Returns 0: addons cannot remove their texts themselves, so they are no leftover references.
	///----------------------------------------------------------------------------------------------------
	uint32_t CleanupRefs(void* aStartAddress, void* aEndAddress) override;

	private:
	using AddonAtlas_t = std::pair<void*, std::shared_ptr<const LocaleAtlas_t>>;

	template <typename T>
	using CTask = std::shared_ptr<Raidcore::Clockwork::Task<std::shared_ptr<const T>>>;

	uint32_t                             ThreadID = 0;

	/* Written from any thread, applied in Advance. */
	std::mutex                           ChangesMutex;
	std::vector<LocaleChange_t>          Changes;
	std::string                          QueuedLanguage;

	/* Sources. UI thread only; immutable once shared, changes replace them. */
	std::filesystem::path                Directory;
	std::shared_ptr<const LocaleAtlas_t> Files;
	CTask<LocaleAtlas_t>                 NextFiles;
	bool                                 IsFilesDirty = false; /* Files must be loaded again. */
	std::vector<AddonAtlas_t>            AddonAtlases;
	bool                                 IsViewDirty = false;  /* Files or addon atlases changed, the view must be built again. */

	/* Lookups read CurrentView while a frame is drawn; Advance swaps in NextView before rendering. */
	std::shared_ptr<const LocaleView_t>  CurrentView;
	CTask<LocaleView_t>                  NextView;
	const LocaleViewLanguage_t*          ActiveLanguage = nullptr;
	std::string                          ActiveLanguageId;

	///----------------------------------------------------------------------------------------------------
	/// ApplyChanges:
	/// 	Applies the queued texts and cleanups to the addon atlases. Returns true if one changed.
	///----------------------------------------------------------------------------------------------------
	bool ApplyChanges(std::vector<LocaleChange_t>& aChanges);

	///----------------------------------------------------------------------------------------------------
	/// SelectActiveLanguage:
	/// 	Points ActiveLanguage at the active language of the current view, else English.
	///----------------------------------------------------------------------------------------------------
	void SelectActiveLanguage();

	public:
	CSetting<std::string> Language{ "Language", "en", [this](const std::string& aLanguage)
	{
		this->SetLanguage(!aLanguage.empty() ? aLanguage : "en");
	} };
};
