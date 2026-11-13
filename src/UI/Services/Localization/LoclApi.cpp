///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LoclApi.cpp
/// Description  :  Handles localization of strings.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "LoclApi.h"

#include <algorithm>
#include <fstream>
#include <windows.h>

#include "Host/GW2/Index/Index.h"
#include "res/Resources.h"
#include "Host/GW2/Context.h"

namespace Clockwork = Raidcore::Clockwork;
using Raidcore::Core::Platform::Windows::CPackFile;

static CLocalization* s_Localization{};

///----------------------------------------------------------------------------------------------------
/// LoadFiles:
/// 	Loads the embedded locale files, then the user's files from the directory on top. Runs on a worker.
///----------------------------------------------------------------------------------------------------
static std::shared_ptr<const LocaleAtlas_t> LoadFiles(const std::filesystem::path& aDirectory)
{
	static constexpr std::pair<const char*, const CPackFile*> s_EmbeddedLocales[] = {
		{ "en/DisplayStrings", &Resources::LocaleDisplayStringsEN }, { "en/GameBinds", &Resources::LocaleGameBindsEN }, { "en/InputBinds", &Resources::LocaleInputBindsEN },
		{ "de/DisplayStrings", &Resources::LocaleDisplayStringsDE }, { "de/GameBinds", &Resources::LocaleGameBindsDE }, { "de/InputBinds", &Resources::LocaleInputBindsDE },
		{ "fr/DisplayStrings", &Resources::LocaleDisplayStringsFR }, { "fr/GameBinds", &Resources::LocaleGameBindsFR }, { "fr/InputBinds", &Resources::LocaleInputBindsFR },
		{ "es/DisplayStrings", &Resources::LocaleDisplayStringsES }, { "es/GameBinds", &Resources::LocaleGameBindsES }, { "es/InputBinds", &Resources::LocaleInputBindsES },
		{ "cn/DisplayStrings", &Resources::LocaleDisplayStringsCN }, { "cn/GameBinds", &Resources::LocaleGameBindsCN }, { "cn/InputBinds", &Resources::LocaleInputBindsCN },
		{ "kr/DisplayStrings", &Resources::LocaleDisplayStringsKR }, { "kr/GameBinds", &Resources::LocaleGameBindsKR }, { "kr/InputBinds", &Resources::LocaleInputBindsKR },
		{ "br/DisplayStrings", &Resources::LocaleDisplayStringsBR }, { "br/GameBinds", &Resources::LocaleGameBindsBR }, { "br/InputBinds", &Resources::LocaleInputBindsBR },
		{ "cz/DisplayStrings", &Resources::LocaleDisplayStringsCZ }, { "cz/GameBinds", &Resources::LocaleGameBindsCZ }, { "cz/InputBinds", &Resources::LocaleInputBindsCZ },
		{ "it/DisplayStrings", &Resources::LocaleDisplayStringsIT }, { "it/GameBinds", &Resources::LocaleGameBindsIT }, { "it/InputBinds", &Resources::LocaleInputBindsIT },
		{ "pl/DisplayStrings", &Resources::LocaleDisplayStringsPL }, { "pl/GameBinds", &Resources::LocaleGameBindsPL }, { "pl/InputBinds", &Resources::LocaleInputBindsPL },
		{ "ru/DisplayStrings", &Resources::LocaleDisplayStringsRU }, { "ru/GameBinds", &Resources::LocaleGameBindsRU }, { "ru/InputBinds", &Resources::LocaleInputBindsRU }
	};

	std::shared_ptr<LocaleAtlas_t> atlas = std::make_shared<LocaleAtlas_t>();

	for (const auto& [name, packFile] : s_EmbeddedLocales)
	{
		std::span<const uint8_t> data = packFile->GetBytes();

		if (data.empty())
		{
			LogWarning(CH_LOCALIZATION, "Embedded locale \"%s\" could not be loaded.", name);
			continue;
		}

		MergeLocaleFile(*atlas, data, std::string("Embedded locale ") + name);
	}

	/* User files are merged after the embedded ones, so they can add languages or override texts. */
	std::error_code ec;

	if (!aDirectory.empty() && std::filesystem::exists(aDirectory, ec))
	{
		for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(aDirectory, ec))
		{
			const std::filesystem::path& path = entry.path();

			if (path.extension() != ".json" || entry.file_size(ec) == 0)
			{
				continue;
			}

			std::ifstream file(path, std::ios::binary);
			std::vector<uint8_t> content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

			MergeLocaleFile(*atlas, content, path.filename().string());
		}
	}

	return atlas;
}

/*static*/ void CLocalization::OnUELanguageChanged(uint32_t* aLanguage)
{
	if (!aLanguage) { return; }
	if (!s_Localization) { return; }

	static constexpr const char* s_Languages[] = { "en", "kr", "fr", "de", "es", "cn" };

	if (*aLanguage < std::size(s_Languages))
	{
		s_Localization->SetLanguage(s_Languages[*aLanguage]);
	}
}

CLocalization::CLocalization() : IRefCleaner("Localization")
{
	CContext* ctx = CContext::GetContext();
	CEventApi* evtapi = ctx->GetEventApi();

	this->SetLocaleDirectory(Index(EPath::DIR_LOCALES));

	std::string lang = this->Language.Get();
	this->SetLanguage(!lang.empty() ? lang : "en");

	evtapi->Subscribe("EV_UNOFFICIAL_EXTRAS_LANGUAGE_CHANGED", reinterpret_cast<EVENT_CONSUME>(CLocalization::OnUELanguageChanged));

	s_Localization = this;
}

CLocalization::~CLocalization()
{
	if (s_Localization == this)
	{
		s_Localization = nullptr;
	}

	this->ActiveLanguage = nullptr;
}

bool CLocalization::Advance()
{
	/* Set thread ID. We may only call Advance from the UI thread. */
	if (this->ThreadID == 0)
	{
		this->ThreadID = GetCurrentThreadId();
	}

	assert(this->ThreadID == GetCurrentThreadId());

	std::vector<LocaleChange_t> changes;
	std::string newLanguage;
	{
		const std::lock_guard<std::mutex> lock(this->ChangesMutex);
		changes.swap(this->Changes);
		newLanguage.swap(this->QueuedLanguage);
	}

	this->IsViewDirty |= this->ApplyChanges(changes);

	bool textsChanged = false;

	if (!newLanguage.empty() && newLanguage != this->ActiveLanguageId)
	{
		this->ActiveLanguageId = newLanguage;
		this->SelectActiveLanguage();
		textsChanged = true;
	}

	if (this->NextFiles && this->NextFiles->Completed())
	{
		this->Files = this->NextFiles->Result();
		this->NextFiles.reset();
		this->IsViewDirty = true;
	}

	if (this->NextView && this->NextView->Completed())
	{
		this->CurrentView = this->NextView->Result();
		this->NextView.reset();
		this->SelectActiveLanguage();
		textsChanged = true;
	}

	/* One task of each kind at a time; changes made meanwhile start the next one when it is done. */
	if (this->IsFilesDirty && !this->NextFiles)
	{
		this->IsFilesDirty = false;

		this->NextFiles = Clockwork::Run<std::shared_ptr<const LocaleAtlas_t>>(Clockwork::ETaskPriority::Normal, [directory = this->Directory](Clockwork::CancellationToken aToken)
		{
			return LoadFiles(directory);
		});
	}

	/* A view built while files are loading would be replaced right away, so it waits for them. */
	if (this->IsViewDirty && !this->NextView && !this->NextFiles && this->Files)
	{
		this->IsViewDirty = false;

		std::vector<std::shared_ptr<const LocaleAtlas_t>> sources;
		sources.push_back(this->Files);

		for (const AddonAtlas_t& addon : this->AddonAtlases)
		{
			sources.push_back(addon.second);
		}

		this->NextView = Clockwork::Run<std::shared_ptr<const LocaleView_t>>(Clockwork::ETaskPriority::Normal, [sources = std::move(sources)](Clockwork::CancellationToken aToken) mutable
		{
			return BuildLocaleView(std::move(sources));
		});
	}

	return textsChanged;
}

bool CLocalization::ApplyChanges(std::vector<LocaleChange_t>& aChanges)
{
	if (aChanges.empty())
	{
		return false;
	}

	/* Copies of the atlases changed in this batch; shared atlases are never modified. */
	std::vector<std::pair<void*, std::shared_ptr<LocaleAtlas_t>>> edits;

	auto editable = [&](void* aOwner) -> LocaleAtlas_t&
	{
		for (auto& [owner, atlas] : edits)
		{
			if (owner == aOwner) { return *atlas; }
		}

		auto it = std::find_if(this->AddonAtlases.begin(), this->AddonAtlases.end(), [aOwner](const AddonAtlas_t& aEntry) { return aEntry.first == aOwner; });

		if (it == this->AddonAtlases.end())
		{
			this->AddonAtlases.emplace_back(aOwner, nullptr);
			it = std::prev(this->AddonAtlases.end());
		}

		auto copy = it->second ? std::make_shared<LocaleAtlas_t>(*it->second) : std::make_shared<LocaleAtlas_t>();
		edits.emplace_back(aOwner, copy);
		return *copy;
	};

	for (LocaleChange_t& change : aChanges)
	{
		if (QueuedText_t* text = std::get_if<QueuedText_t>(&change))
		{
			editable(text->Owner)[text->LanguageIdentifier].Texts.insert_or_assign(std::move(text->Identifier), std::move(text->Text));
		}
		else
		{
			const QueuedCleanup_t& cleanup = std::get<QueuedCleanup_t>(change);

			auto inRange = [&cleanup](void* aOwner)
			{
				return aOwner && aOwner >= cleanup.StartAddress && aOwner <= cleanup.EndAddress;
			};

			std::erase_if(this->AddonAtlases, [&](const AddonAtlas_t& aEntry) { return inRange(aEntry.first); });
			std::erase_if(edits, [&](const auto& aEntry) { return inRange(aEntry.first); });
		}
	}

	for (auto& [owner, atlas] : edits)
	{
		for (AddonAtlas_t& entry : this->AddonAtlases)
		{
			if (entry.first == owner)
			{
				entry.second = std::move(atlas);
				break;
			}
		}
	}

	return true;
}

void CLocalization::SelectActiveLanguage()
{
	const LocaleView_t* view = this->CurrentView.get();
	const LocaleViewLanguage_t* language = nullptr;

	if (view)
	{
		language = view->Find(this->ActiveLanguageId);

		if (!language)
		{
			language = view->Find("en");
		}
	}

	this->ActiveLanguage = language;
}

const char* CLocalization::Translate(const char* aIdentifier, const char* aLanguageIdentifier)
{
	/* Advance swaps the view on the UI thread; ThreadID is set by the first Advance. */
	assert((this->ThreadID == 0 || this->ThreadID == GetCurrentThreadId()) && "Translate is UI thread only.");

	if (aIdentifier == nullptr) { return aIdentifier; }

	const LocaleView_t* view = this->CurrentView.get();

	if (!view)
	{
		return aIdentifier;
	}

	const std::string_view identifier = aIdentifier;

	/* The requested language's own texts first, so a text of the active language still wins over English. */
	if (aLanguageIdentifier)
	{
		auto langIt = view->Languages.find(std::string_view{ aLanguageIdentifier });

		if (langIt != view->Languages.end())
		{
			auto it = langIt->second.Own.find(identifier);

			if (it != langIt->second.Own.end())
			{
				return it->second;
			}
		}
	}

	/* Lookup already holds the English fallback, so one search covers both. */
	const LocaleViewLanguage_t* language = this->ActiveLanguage;

	if (!language)
	{
		language = view->Find("en");
	}

	if (language)
	{
		auto it = language->Lookup.find(identifier);

		if (it != language->Lookup.end())
		{
			return it->second;
		}
	}

	return aIdentifier;
}

void CLocalization::Set(const char* aIdentifier, const char* aLanguageIdentifier, const char* aText, void* aCaller)
{
	if (!aIdentifier)         { return; }
	if (!aLanguageIdentifier) { return; }
	if (!aText)               { return; }

	/* The module base matches the range CleanupRefs receives when that module unloads. */
	HMODULE owner = nullptr;

	if (aCaller)
	{
		GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, static_cast<LPCWSTR>(aCaller), &owner);
	}

	const std::lock_guard<std::mutex> lock(this->ChangesMutex);
	this->Changes.push_back(QueuedText_t{ aIdentifier, aLanguageIdentifier, aText, owner });
}

void CLocalization::SetLanguage(const std::string& aIdentifier)
{
	const std::lock_guard<std::mutex> lock(this->ChangesMutex);
	this->QueuedLanguage = aIdentifier;
}

const char* CText::Text() const
{
	if (!this->KeyStr)
	{
		return "";
	}

	return s_Localization ? s_Localization->Translate(this->KeyStr) : this->KeyStr;
}

uint32_t CLocalization::CleanupRefs(void* aStartAddress, void* aEndAddress)
{
	const std::lock_guard<std::mutex> lock(this->ChangesMutex);
	this->Changes.push_back(QueuedCleanup_t{ aStartAddress, aEndAddress });

	return 0;
}

std::vector<std::string> CLocalization::GetLanguages()
{
	std::vector<std::string> languages;

	if (this->CurrentView)
	{
		for (const auto& [id, language] : this->CurrentView->Languages)
		{
			languages.push_back(language.DisplayName);
		}
	}

	return languages;
}

const std::string& CLocalization::GetActiveLanguage()
{
	static const std::string s_None;

	const LocaleViewLanguage_t* language = this->ActiveLanguage;
	return language ? language->DisplayName : s_None;
}

void CLocalization::SetLocaleDirectory(std::filesystem::path aPath)
{
	this->Directory = std::move(aPath);
	this->IsFilesDirty = true;
}

std::vector<const char*> CLocalization::GetAllTexts()
{
	std::vector<const char*> allTexts;

	if (!this->CurrentView)
	{
		return allTexts;
	}

	for (const auto& [id, language] : this->CurrentView->Languages)
	{
		/* The language picker shows every display name, so their glyphs are needed even if the language is not active. */
		allTexts.push_back(language.DisplayName.c_str());

		for (const auto& [key, text] : language.Own)
		{
			allTexts.push_back(text);
		}
	}

	return allTexts;
}
