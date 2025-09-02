///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Addon.cpp
/// Description  :  Addon implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Addon.h"

#include <assert.h>
#include <chrono>
#include <psapi.h>

#include "Core/Addons/API/ApiBuilder.h"
#include "Core/Context.h"
#include "Core/Index/Index.h"
#include "Engine/Cleanup/RefCleanerContext.h"
#include "GW2/Build/BuildInfo.h"
#include "Util/DLL.h"
#include "Util/MD5.h"
#include "Util/Paths.h"
#include "Util/Strings.h"

Config_t* CAddon::GetConfig()
{
	return this->Config;
}

bool CAddon::HasInterface(EAddonInterfaces aInterface) const
{
	return (bool)(this->ModuleInterfaces & aInterface);
}

bool CAddon::IsDuplicate()
{
	return this->Loader->IsTrackedSafe(this->GetSignature(), this->GetBase());
}

bool CAddon::IsVersionDisabled() const
{
	if (!this->Config) { return false; }

	/* Tiny optimization, to not calculate the entire MD5. */
	if (this->Config->DisableVersion.empty())
	{
		return false;
	}

	return this->Config->DisableVersion == this->GetMD5().string();
}

bool CAddon::SupportsLoading() const
{
	return this->HasInterface(EAddonInterfaces::Nexus);
}

bool CAddon::SupportsUpdates() const
{
	return this->HasInterface(EAddonInterfaces::Nexus);
}

void CAddon::UninstallInternal()
{
	this->Logger->Trace(CH_ADDON, "CAddon::UninstallInternal(%s)", this->Location.string().c_str());

	if (this->IsUninstalled())
	{
		this->Logger->Debug(CH_ADDON, "Already uninstalled. (%s)", this->Location.string().c_str());
		return;
	}

	/* If still loaded but state isn't locked, unload first. */
	if (this->IsLoaded() && !this->IsStateLocked())
	{
		this->Logger->Debug(CH_ADDON, "Still loaded but state isn't locked requeing with unload. (%s)", this->Location.string().c_str());

		/* Queue unload. */
		this->Unload();

		/* Queue uninstall. */
		this->Uninstall();

		/* Cancel this uninstall procedure. */
		return;
	}

	/* Set the flag. */
	this->Flags |= EAddonFlags::Uninstalled;

	/* If it's statelocked and still loaded, keep the config in memory, but remove from disk. */
	if (this->IsLoaded() && this->IsStateLocked())
	{
		this->Logger->Debug(
			CH_ADDON,
			"Addon is still loaded, config already removed: %s",
			this->Location.string().c_str()
		);
		
		if (this->Config)
		{
			this->Config->Persist = false;

			/* Already remove the config from disk. */
			this->ConfigMgr->SaveConfigs();
		}
	}
	else if (!this->IsLoaded())
	{
		this->ConfigMgr->DeleteConfig(this->GetSignature());
	}
	else
	{
		/* If we reach this, the addon is still loaded, but not state locked. However it should've been unloaded first based on this exact check prior.*/
		assert(false && "Unreachable code.");
	}

	if (this->IsFileLocked())
	{
		this->Logger->Warning(
			CH_ADDON,
			"Addon is still loaded, it will be uninstalled the next time the game is started: %s",
			this->Location.string().c_str()
		);

		/* File is locked, try to move it into temp so it can be deleted next game start. */
		try
		{
			std::filesystem::path targetLocation = Path::GetUnused(Index(EPath::DIR_TEMP) / this->Location.filename().string().append(".uninstall"));
			std::filesystem::rename(this->Location, targetLocation);

			this->Logger->Debug(
				CH_ADDON,
				"Addon uninstall.\n\tOld Location: %s\n\tNew Location: %s",
				this->Location.string().c_str(),
				targetLocation.string().c_str()
			);

			this->Location = targetLocation;
		}
		catch (std::filesystem::filesystem_error fErr)
		{
			this->Flags |= EAddonFlags::FileLocked;
			this->Logger->Critical(
				CH_ADDON,
				"Error while trying to uninstall(move) locked addon: %s (%s)",
				fErr.what(),
				this->Location.string().c_str()
			);
		}
	}
	else
	{
		/* File is not locked, we try to remove it. */
		try
		{
			Logger->Info(CH_ADDON, "Uninstalled addon: %s", this->Location.string().c_str());
			std::filesystem::remove(this->Location);
			this->Location.clear();
			this->MD5.clear();
		}
		catch (std::filesystem::filesystem_error fErr)
		{
			this->Flags |= EAddonFlags::FileLocked;
			Logger->Warning(
				CH_ADDON,
				"Cannot remove addon from disk: %s (%s)",
				fErr.what(),
				this->Location.string().c_str()
			);
		}
	}

	this->Loader->NotifyChanges();
}

const EAddonInterfaces& CAddon::EnumInterfaces()
{
	this->Logger->Trace(CH_ADDON, "CAddon::EnumInterfaces(%s)", this->Location.string().c_str());

	/* Reset interfaces. */
	this->ModuleInterfaces = EAddonInterfaces::None;

	if (this->Location.empty()) { return this->ModuleInterfaces; }

	HMODULE module = LoadLibraryA(this->Location.string().c_str());

	if (!module)
	{
		DWORD lasterror = GetLastError();
		const std::error_condition ecnd = std::system_category().default_error_condition(lasterror);
		this->Logger->Warning(
			CH_ADDON,
			"Cannot enumerate addon interfaces. LoadLibrary(%s) failed: %s (%d)",
			this->Location.string().c_str(),
			ecnd.message().c_str(),
			lasterror
		);
		return this->ModuleInterfaces;
	}

	EAddonInterfaces interfaces = EAddonInterfaces::None;

	void* discard = nullptr;

	if (DLL::FindFunction(module, &discard, "GetAddonDef"))
	{
		interfaces |= EAddonInterfaces::Nexus;
	}

	if (DLL::FindFunction(module, &discard, "get_init_addr") &&
		DLL::FindFunction(module, &discard, "get_release_addr"))
	{
		interfaces |= EAddonInterfaces::ArcDPS;
	}

	if (DLL::FindFunction(module, &discard, "gw2addon_get_description") &&
		DLL::FindFunction(module, &discard, "gw2addon_load") &&
		DLL::FindFunction(module, &discard, "gw2addon_unload"))
	{
		interfaces |= EAddonInterfaces::AddonLoader;
	}

	if (DLL::FindFunction(module, &discard, "D3D11CreateDevice") &&
		DLL::FindFunction(module, &discard, "D3D11CreateDeviceAndSwapChain"))
	{
		if (!(DLL::FindFunction(module, &discard, "D3D11CoreCreateDevice") &&
			DLL::FindFunction(module, &discard, "D3D11CoreCreateLayeredDevice") &&
			DLL::FindFunction(module, &discard, "D3D11CoreGetLayeredDeviceSize") &&
			DLL::FindFunction(module, &discard, "D3D11CoreRegisterLayers")))
		{
			this->Logger->Debug(CH_ADDON, "Stub D3D11. (%s)", this->Location.string().c_str());
		}

		interfaces |= EAddonInterfaces::D3D11Proxy;
	}

	if (DLL::FindFunction(module, &discard, "CreateDXGIFactory") &&
		DLL::FindFunction(module, &discard, "CreateDXGIFactory1") &&
		DLL::FindFunction(module, &discard, "CreateDXGIFactory2"))
	{
		if (!(DLL::FindFunction(module, &discard, "DXGIGetDebugInterface1") &&
			DLL::FindFunction(module, &discard, "CompatValue") &&
			DLL::FindFunction(module, &discard, "CompatString")))
		{
			this->Logger->Debug(CH_ADDON, "Stub DXGI. (%s)", this->Location.string().c_str());
		}

		interfaces |= EAddonInterfaces::DXGIProxy;
	}

	this->ModuleInterfaces = interfaces;

	if (!FreeLibrary(module))
	{
		DWORD lasterror = GetLastError();
		const std::error_condition ecnd = std::system_category().default_error_condition(lasterror);
		this->Logger->Warning(
			CH_ADDON,
			"Cannot free library after enumerating interfaces. FreeLibrary(%s) failed: %s (%d)",
			this->Location.string().c_str(),
			ecnd.message().c_str(),
			lasterror
		);
	}

	this->Logger->Debug(CH_ADDON, "Interfaces: %u (%s)", this->ModuleInterfaces, this->Location.string().c_str());

	return this->ModuleInterfaces;
}

bool CAddon::ShouldLoad()
{
	assert(this->Config);

	bool result = true;

	/* Check the last state as per user prefs. But also make sure no state set yet -> Autoload. */
	if (!this->Config->LastLoadState && this->State == EAddonState::None)
	{
		this->Logger->Debug(CH_ADDON, "Canceled load. Config->LastLoadState: false. (%s)", this->Location.string().c_str());
		result = false;
	}

	/* If first launch only. */
	if ((this->NexusAddonDefV1->Flags & EAddonDefFlags::LaunchOnly) == EAddonDefFlags::LaunchOnly)
	{
		/* First launch means, state wasn't set yet. */
		if (this->State != EAddonState::None)
		{
			this->Flags |= EAddonFlags::StateLocked;
			this->Logger->Debug(CH_ADDON, "Canceled load. Only load on initial load. (%s)", this->Location.string().c_str());
			result = false;
		}
	}

	/* If this version is broken and was disabled. */
	if (this->IsVersionDisabled())
	{
		this->Logger->Debug(CH_ADDON, "Canceled load. This version is disabled. (%s)", this->Location.string().c_str());
		result = false;
	}

	/* If this addon is volatile and the change difference is greater than 350. */
	if (this->IsVolatileDisabled())
	{
		this->Logger->Debug(
			CH_ADDON,
			"Canceled load. Volatile addon and gamebuild diff is %u. (%s)",
			GW2::GetGameBuild() - this->Config->LastGameBuild,
			this->Location.string().c_str()
		);
		result = false;
	}

	return result;
}

bool CAddon::ShouldUpdate()
{
	assert(this->Config);

	switch (this->Config->UpdateMode)
	{
		default:
		case EUpdateMode::None:       { return false; }
		case EUpdateMode::Background: { return false; }
		case EUpdateMode::Notify:     { return false; }
		case EUpdateMode::Automatic:  { return true; }
	}
}

bool CAddon::IsVolatileDisabled()
{
	if (!this->NexusAddonDefV1)
	{
		return false;
	}

	if ((this->NexusAddonDefV1->Flags & EAddonDefFlags::IsVolatile) != EAddonDefFlags::IsVolatile)
	{
		return false;
	}

	if (this->Config->LastGameBuild != 0 && GW2::GetGameBuild() - this->Config->LastGameBuild > 350)
	{
		/* FIXME: move this elsewhere maybe? rather than the check */
		if (this->Config->DisableVersion.empty() || this->Config->DisableVersion != this->GetMD5().string())
		{
			this->Config->DisableVersion = this->GetMD5().string();
			this->ConfigMgr->SaveConfigs();
		}
		return true;
	}

	return false;
}
