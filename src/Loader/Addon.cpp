///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Addon.cpp
/// Description  :  Contains the logic for addons.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Addon.h"

CAddon::CAddon(AddonDefinition* aAddonDef)
{
	
}

CAddon::CAddon(ArcdpsPlugin* aPluginDef)
{

}

CAddon::~CAddon()
{
	this->IsCanceled = true;
	this->ConVar.notify_one();
	this->ProcessorThread.join();
}

#pragma region Public
const AddonFlags& CAddon::GetFlags()
{
	return this->Flags;
}

bool CAddon::IsUpdateAvailable()
{
	assert(this->Type == EAddonType::Nexus);

	return this->Flags.IsUpdateAvailable;
}

std::string CAddon::GetError()
{
	assert(this->State == EAddonState::Error);

	return this->ErrorDescription;
}

/* FIXME: this needs to be moved to the loader, as that will own the prefs */
json CAddon::GetPrefs()
{
	if (!this->IsPersistent())
	{
		return nullptr;
	}

	/* Do not save any preferences if uninstalling. */
	if (this->Flags.UninstallNextLaunch)
	{
		return nullptr;
	}

	json j{};
	
	j[PREFS_ADDON_SIGNATURE]             = this->AddonDef.Signature;
	j[PREFS_ADDON_NAME]                  = this->AddonDef.Name;
	j[PREFS_ADDON_UPDATEMODE]            = this->Preferences->UpdateMode;
	j[PREFS_ADDON_ALLOWPRERELEASES]      = this->Preferences->AllowPreReleases;
	j[PREFS_ADDON_ISFAVORITE]            = this->Preferences->IsFavorite;
	j[PREFS_ADDON_ISDISABLEDUNTILUPDATE] = this->Preferences->IsDisabledUntilUpdate;

	j[PREFS_ADDON_ISLOADED]              = (this->State == EAddonState::Loaded || this->Flags.LoadNextLaunch) && !this->Flags.UnloadNextLaunch;
	
	/* Sanity check. */
	if (j[PREFS_ADDON_SIGNATURE].get<signed int>() == 0)
	{
		return nullptr;
	}

	return j;
}

bool CAddon::OwnsAddress(void* aAddress)
{
	if (this->Type != EAddonType::Nexus)
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

std::string CAddon::GetName()
{
	switch (this->Type)
	{
		case EAddonType::Nexus:
		{
			return this->AddonDef.Name;
		}
		case EAddonType::Library:
		{
			assert(this->LibraryDef);
			return this->LibraryDef->Name;
		}
		case EAddonType::ArcDPS:
		{
			return this->PluginDef.Name;
		}
	}

	return NULLSTR;
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
void CAddon::Process()
{
	for (;;)
	{
		EAddonAction action = EAddonAction::None;
		
		{
			std::unique_lock<std::mutex> lockThread(this->ProcessorMutex);
			this->ConVar.wait(lockThread);

			if (this->IsCanceled) { return; }

			std::swap(action, this->QueuedAction);
		}

		switch (action)
		{
			case EAddonAction::Load:
			{
				this->Flags.IsLoading = true;
				this->Load();
				this->Flags.IsLoading = false;
				break;
			}
			case EAddonAction::Unload:
			{
				this->Flags.IsUnloading = true;
				this->Unload();
				this->Flags.IsUnloading = false;
				break;
			}
			case EAddonAction::Install:
			{
				this->Flags.IsInstalling = true;
				this->Install();
				this->Flags.IsInstalling = false;
				break;
			}
			case EAddonAction::Uninstall:
			{
				this->Flags.IsUninstalling = true;
				this->Uninstall();
				this->Flags.IsUninstalling = false;
				break;
			}
			case EAddonAction::CheckUpdate:
			{
				this->Flags.IsCheckingForUpdates = true;
				this->CheckForUpdate();
				this->Flags.IsCheckingForUpdates = false;
				break;
			}
			case EAddonAction::Update:
			{
				this->Flags.IsUpdating = true;
				this->Update();
				this->Flags.IsUpdating = false;
				break;
			}
		}
	}
}

void CAddon::Load()
{
	assert(this->Type == EAddonType::Nexus);

	/* already loaded */
	if (this->State == EAddonState::Loaded)
	{
		return;
	}

	/* locked addons' state cannot be modified */
	if (this->Flags.IsLocked)
	{
		this->Flags.LoadNextLaunch = true;
		this->Flags.UnloadNextLaunch = false;
		this->Flags.UninstallNextLaunch = false;
		return;
	}
}

void CAddon::Unload()
{
	assert(this->Type == EAddonType::Nexus);

	/* already unloaded */
	if (this->State == EAddonState::NotLoaded)
	{
		return;
	}

	/* locked addons' state cannot be modified */
	if (this->Flags.IsLocked)
	{
		this->Flags.LoadNextLaunch = false;
		this->Flags.UnloadNextLaunch = true;
		this->Flags.UninstallNextLaunch = false;
		return;
	}
}

void CAddon::Install()
{
	assert(this->Type == EAddonType::Library);
}

void CAddon::Uninstall()
{
	assert(this->Type == EAddonType::Nexus || this->Type == EAddonType::ArcDPS);

	/* ensure it's not loaded if it's a Nexus addon */
	if (this->Type == EAddonType::Nexus)
	{
		this->Unload();
	}

	if (this->Flags.IsLocked)
	{
		this->Flags.LoadNextLaunch = false;
		this->Flags.UnloadNextLaunch = false;
		this->Flags.UninstallNextLaunch = true;

		try
		{
			std::filesystem::path current = this->Location;

			std::filesystem::rename(this->Location, this->Location.string() + EXT_UNINSTALL);

			this->RealLocation = this->Location; // points to .dll.uninstall
			this->Location     = current;        // points to .dll

			this->Logger->Warning(CH_LOADER, "Addon is stilled loaded, it will be uninstalled the next time the game is started: %s", this->Location.string().c_str());
		}
		catch (std::filesystem::filesystem_error fErr)
		{
			Logger->Debug(CH_LOADER, "%s", fErr.what());
		}

		return;
	}

	/* file no longer on disk */
	if (!std::filesystem::exists(this->RealLocation))
	{
		this->RealLocation.clear();
		return;
	}

	try
	{
		std::filesystem::remove(this->RealLocation.string().c_str());

		switch (this->Type)
		{
			case EAddonType::Nexus:
			{
				this->Loader->DeregisterPrefs(this->AddonDef.Signature);
				Logger->Info(CH_LOADER, "Uninstalled addon: %s", this->Location.string().c_str());
				break;
			}
			case EAddonType::ArcDPS:
			{
				Logger->Info(CH_LOADER, "Uninstalled ArcDPS plugin: %s", this->Location.string().c_str());
				break;
			}
		}
	}
	catch (std::filesystem::filesystem_error fErr)
	{
		Logger->Debug(CH_LOADER, "%s", fErr.what());
	}
}

void CAddon::CheckForUpdate()
{
	assert(this->Type == EAddonType::Nexus);
}

void CAddon::Update()
{
	assert(this->Type == EAddonType::Nexus);
}

bool CAddon::ShouldUpdate()
{
	assert(this->Type == EAddonType::Nexus);
	assert(this->Preferences);

	if (this->Preferences->UpdateMode == EUpdateMode::AutoUpdate)
	{
		return true;
	}

	return false;
}

bool CAddon::ShouldLoad()
{
	assert(this->Type == EAddonType::Nexus);

	return false;
}
#pragma endregion
