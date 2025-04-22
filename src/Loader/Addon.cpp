///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Addon.cpp
/// Description  :  Contains the logic for addons.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Addon.h"

#include "Index.h"

CAddon::CAddon(CLogHandler* aLogger, CEventApi* aEventApi, CPreferenceMgr* aPrefMgr, CLibraryMgr* aLibraryMgr)
{
	this->Logger = aLogger;
	this->EventApi = aEventApi;
	this->PrefMgr = aPrefMgr;
	this->LibraryMgr = aLibraryMgr;
}

CAddon::~CAddon()
{
	this->IsCanceled = true;
	this->ConVar.notify_one();
	this->ProcessorThread.join();

	this->Logger = nullptr;
	this->PrefMgr = nullptr;
	this->LibraryMgr = nullptr;
}

#pragma region Public
bool CAddon::IsValid() const
{
	return this->GetFlags().HasLibraryDef || this->GetFlags().HasAddonDef || this->GetFlags().HasPluginDef;
}

const AddonFlags& CAddon::GetFlags() const
{
	return this->Flags;
}

bool CAddon::IsUpdateAvailable() const
{
	return this->GetFlags().IsUpdateAvailable;
}

std::string CAddon::GetError()
{
	assert(this->State == EAddonState::Error);

	return this->ErrorDescription;
}

bool CAddon::OwnsAddress(void* aAddress) const
{
	if (!this->GetFlags().HasAddonDef && !this->GetFlags().IsModuleLoaded)
	{
		return false;
	}

	if (this->Module && this->ModuleSize > 0)
	{
		void* startAddress = this->Module;
		void* endAddress = ((PBYTE)this->Module) + this->ModuleSize;

		if (aAddress >= startAddress && aAddress <= endAddress)
		{
			return true;
		}
	}

	return false;
}

const std::string& CAddon::GetName() const
{
	if (this->GetFlags().HasAddonDef)
	{
		return this->AddonDef.Name;
	}

	if (this->GetFlags().HasPluginDef)
	{
		return this->PluginDef.Name;
	}

	if (this->GetFlags().HasLibraryDef)
	{
		return this->LibraryDef->Name;
	}

	return NULLSTR;
}

const std::filesystem::path& CAddon::GetLocation() const
{
	return this->Location;
}

const std::vector<unsigned char>& CAddon::GetMD5() const
{
	return this->MD5;
}

const LibraryAddon* CAddon::GetLibraryDef() const
{
	return this->LibraryDef;
}

void CAddon::QueueAction(EAddonAction aAction)
{
	{
		std::lock_guard<std::mutex> lock(this->ProcessorMutex);
		this->QueuedAction = aAction;
	}
	this->ConVar.notify_one();
}
#pragma endregion

#pragma region Private
void CAddon::ProcessAction()
{
	if (this->ProcessorThreadId == 0)
	{
		this->ProcessorThreadId = GetCurrentThreadId();
	}

	for (;;)
	{
		EAddonAction action = EAddonAction::None;
		
		{
			std::unique_lock<std::mutex> lockThread(this->ProcessorMutex);
			this->ConVar.wait(lockThread, [this]{ return this->Flags.IsRunningAction == false; });

			if (this->IsCanceled) { return; }

			std::swap(action, this->QueuedAction);
		}

		this->Flags.IsRunningAction = true;

		switch (action)
		{
			case EAddonAction::Load:
			{
				this->Load();
				break;
			}
			case EAddonAction::Unload:
			{
				this->Unload();
				break;
			}
			case EAddonAction::Install:
			{
				this->Install();
				break;
			}
			case EAddonAction::Uninstall:
			{
				this->Uninstall();
				break;
			}
			case EAddonAction::CheckUpdate:
			{
				this->CheckForUpdate();
				break;
			}
			case EAddonAction::Update:
			{
				this->Update();
				break;
			}
		}

		this->Flags.IsRunningAction = false;
	}
}

void CAddon::Load()
{
	assert(this->ProcessorThreadId == GetCurrentThreadId());
	assert(this->Type == EAddonType::Nexus);

	/* already loaded */
	if (this->State == EAddonState::Loaded)
	{
		return;
	}

	/* locked addons' state cannot be modified */
	if (this->Flags.IsStateLocked)
	{
		this->Flags.LoadNextLaunch = true;
		this->Flags.UnloadNextLaunch = false;
		this->Flags.IsUninstalled = false;
		this->PrefMgr->SavePrefsSafe();
		return;
	}

	// TODO: load
}

void CAddon::Unload()
{
	assert(this->ProcessorThreadId == GetCurrentThreadId());
	assert(this->Type == EAddonType::Nexus);
	assert(this->AddonDef.IsValid());

	/* Already unloaded. */
	if (this->State == EAddonState::NotLoaded)
	{
		return;
	}

	assert(this->Flags.IsModuleLoaded);

	/* Locked addons' state cannot be modified */
	if (this->Flags.IsStateLocked)
	{
		this->Flags.LoadNextLaunch = false;
		this->Flags.UnloadNextLaunch = true;
		this->Flags.IsUninstalled = false;
		this->PrefMgr->SavePrefsSafe();
		return;
	}

	/* Stage references for removal. Disables any callback. */
	this->API_Invalidate();

	/* Call unload procedure of the addon. */
	std::chrono::steady_clock::time_point start_time = std::chrono::high_resolution_clock::now();
	
	this->AddonDef.Unload();

	std::chrono::steady_clock::time_point end_time = std::chrono::high_resolution_clock::now();
	std::chrono::steady_clock::duration time = end_time - start_time;

	/* Remove leftover references of the addon, if it's not potty trained. */
	this->API_Clear();

	this->State = EAddonState::NotLoaded;
	this->PrefMgr->SavePrefsSafe();

	/* Raise event for other addons to react. */
	this->EventApi->Raise(EV_ADDON_UNLOADED, &this->AddonDef.Signature);
	
	/* FreeLibrary the module, if the lord see fit. */
	if (!this->Flags.IsModuleLocked && FreeLibrary(this->Module))
	{
		this->Flags.IsModuleLoaded = false;
		this->Module = 0;
		this->ModuleSize = 0;
	}
	else if (!this->Flags.IsModuleLocked)
	{
		this->Flags.IsModuleLocked = true;
		const std::error_condition ecnd = std::system_category().default_error_condition(GetLastError());
		this->Logger->Warning(
			CH_LOADER,
			"Unload was called but FreeLibrary failed: %s\n\tError Code %u : %s",
			this->Location.c_str(),
			ecnd.value(),
			ecnd.message().c_str()
		);
	}
	else if (this->Flags.IsModuleLocked)
	{
		this->Logger->Debug(CH_LOADER, "Unload was called but module is locked: %s", this->Location.c_str());
	}
	else
	{
		assert("Unreachable code.");
	}

	Logger->Info(CH_LOADER, u8"Unloaded addon \"%s\" in %uµs.", this->Location.c_str(), time / std::chrono::microseconds(1));
}

void CAddon::Install()
{
	assert(this->ProcessorThreadId == GetCurrentThreadId());
	assert(this->Type == EAddonType::Library);
	assert(this->State == EAddonState::None);

	// TODO: install
}

void CAddon::Uninstall()
{
	assert(this->ProcessorThreadId == GetCurrentThreadId());
	assert(this->Type == EAddonType::Nexus);
	assert(this->State == EAddonState::NotLoaded || this->State == EAddonState::Loaded);

	/* Force unload. */
	this->Unload();

	/* Set flags and print warning, if locked. */
	if (this->Flags.IsStateLocked)
	{
		try
		{
			std::filesystem::path targetLocation = Index::D_GW2_ADDONS_NEXUS_TEMP / this->Location.filename().append(EXT_UNINSTALL);

			std::filesystem::rename(this->Location, targetLocation);

			this->Flags.LoadNextLaunch = false;
			this->Flags.UnloadNextLaunch = false;
			this->Flags.IsUninstalled = true;

			this->Logger->Warning(CH_LOADER, "Addon is still loaded, it will be uninstalled the next time the game is started: %s", this->Location.string().c_str());
		
			this->PrefMgr->DeletePrefs(this->AddonDef.Signature);
			this->Location = targetLocation;
			this->MD5.clear();

		}
		catch (std::filesystem::filesystem_error fErr)
		{
			Logger->Debug(CH_LOADER, "%s", fErr.what());
		}

		return;
	}

	/* Already uninstalled. */
	if (!std::filesystem::exists(this->Location))
	{
		this->PrefMgr->DeletePrefs(this->AddonDef.Signature);
		this->Location.clear();
		this->MD5.clear();
		return;
	}

	/* Delete file from disk. */
	try
	{
		std::filesystem::remove(this->Location);

		Logger->Info(CH_LOADER, "Uninstalled addon: %s", this->Location);

		this->PrefMgr->DeletePrefs(this->AddonDef.Signature);
		this->Location.clear();
		this->MD5.clear();
	}
	catch (std::filesystem::filesystem_error fErr)
	{
		Logger->Debug(CH_LOADER, "%s", fErr.what());
	}
}

void CAddon::CheckForUpdate()
{
	assert(this->ProcessorThreadId == GetCurrentThreadId());
	assert(this->Type == EAddonType::Nexus);

	// TODO: check
}

void CAddon::Update()
{
	assert(this->ProcessorThreadId == GetCurrentThreadId());
	assert(this->Type == EAddonType::Nexus);

	// TODO: update
}

void CAddon::API_Commit()
{
	assert(this->Type == EAddonType::Nexus);

	// TODO: API_Commit
}

void CAddon::API_Invalidate()
{
	assert(this->Type == EAddonType::Nexus);

	// TODO: API_Invalidate
}

void CAddon::API_Clear()
{
	assert(this->Type == EAddonType::Nexus);

	// TODO: API_Clear
}

bool CAddon::ShouldCheckForUpdate()
{
	assert(this->Type == EAddonType::Nexus);
	assert(this->Preferences);

	switch (this->Preferences->UpdateMode)
	{
		default:
		case EUpdateMode::None:       return false;

		case EUpdateMode::Background: return true;
		case EUpdateMode::Prompt:     return true;
		case EUpdateMode::AutoUpdate: return true;
	}
}

bool CAddon::ShouldUpdate()
{
	assert(this->Type == EAddonType::Nexus);
	assert(this->Preferences);

	switch (this->Preferences->UpdateMode)
	{
		default:
		case EUpdateMode::None:       return false;
		case EUpdateMode::Background: return false;
		case EUpdateMode::Prompt:     return false;

		case EUpdateMode::AutoUpdate: return true;
	}
}

bool CAddon::ShouldLoad()
{
	assert(this->Type == EAddonType::Nexus);
	assert(this->Preferences);

	return this->Preferences->ShouldLoad;
}
#pragma endregion
