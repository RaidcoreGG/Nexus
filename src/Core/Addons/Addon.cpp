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
#include <windows.h>

#include "Core/Addons/API/ApiBuilder.h"
#include "Core/Context.h"
#include "Core/Index/Index.h"
#include "Engine/Cleanup/RefCleanerContext.h"
#include "GW2/Build/BuildInfo.h"
#include "Util/DLL.h"
#include "Util/MD5.h"
#include "Util/Paths.h"
#include "Util/Strings.h"
#include "Util/Time.h"

CAddon::CAddon(std::filesystem::path aLocation)
{
	this->Location = aLocation;
	this->MD5 = MD5Util::FromFile(aLocation);

	CContext* ctx = CContext::GetContext();
	this->Logger = ctx->GetLogger();
	this->Loader = ctx->GetLoader();
	this->EventApi = ctx->GetEventApi();
	this->ConfigMgr = ctx->GetCfgMgr();

	/* Does the initial enumerate addon interfaces and raises a create event. */
	this->QueuedActions.push(EAddonAction::Create);

	this->IsRunning = true;
	this->ProcessorThread = std::thread(&CAddon::ProcessActions, this);
}

CAddon::~CAddon()
{
	/* Clear the entire queue. */
	std::queue<EAddonAction> empty;
	std::swap(this->QueuedActions, empty);

	/* Unloads, exits the processor thread and raises a destroy event. */
	this->QueuedActions.push(EAddonAction::Destroy);

	/* Notify the processor thread. */
	this->ConVar.notify_one();

	if (this->ProcessorThread.joinable())
	{
		this->ProcessorThread.join();
	}

	if (this->NexusAddonDefV1)
	{
		delete this->NexusAddonDefV1;
	}

	if (this->ArcExtensionDef)
	{
		delete this->ArcExtensionDef;
	}
}

uint32_t CAddon::GetSignature()
{
	if (this->NexusAddonDefV1)
	{
		return this->NexusAddonDefV1->Signature;
	}

	if (this->ArcExtensionDef)
	{
		return this->ArcExtensionDef->Signature;
	}

	return 0;
}

void CAddon::Load()
{
	if (!this->IsRunning) { return; }

	std::lock_guard<std::mutex> lock(this->ProcessorMutex);
	this->QueuedActions.push(EAddonAction::Load);
	this->ConVar.notify_one();
}

void CAddon::Unload()
{
	if (!this->IsRunning) { return; }

	std::lock_guard<std::mutex> lock(this->ProcessorMutex);
	this->QueuedActions.push(EAddonAction::Unload);
	this->ConVar.notify_one();
}

void CAddon::Uninstall()
{
	if (!this->IsRunning) { return; }

	std::lock_guard<std::mutex> lock(this->ProcessorMutex);
	this->QueuedActions.push(EAddonAction::Uninstall);
	this->ConVar.notify_one();
}

void CAddon::CheckForUpdate()
{
	if (!this->IsRunning) { return; }

	std::lock_guard<std::mutex> lock(this->ProcessorMutex);
	this->QueuedActions.push(EAddonAction::CheckUpdate);
	this->ConVar.notify_one();
}

void CAddon::Update()
{
	if (!this->IsRunning) { return; }

	std::lock_guard<std::mutex> lock(this->ProcessorMutex);
	this->QueuedActions.push(EAddonAction::Update);
	this->ConVar.notify_one();
}

void CAddon::ProcessActions()
{
	while (this->IsRunning)
	{
		EAddonAction action = EAddonAction::None;

		/* Scope, just to retrieve a queued action. */
		{
			std::unique_lock<std::mutex> lockThread(this->ProcessorMutex);
			this->ConVar.wait(lockThread, [this] { return this->QueuedActions.size() > 0; });

			if (!this->IsRunning) { return; }

			action = this->QueuedActions.front();
			this->QueuedActions.pop();
		}

		this->Flags |= EAddonFlags::RunningAction;

		switch (action)
		{
			case EAddonAction::Create:
			{
				this->Logger->Trace(CH_ADDON, "CAddon::Create(): %s", this->Location.string().c_str());
				this->EnumInterfaces();
				this->EventApi->Raise(0, EV_ADDON_CREATED);
				break;
			}
			case EAddonAction::Destroy:
			{
				this->Flags |= EAddonFlags::Destroying;
				this->Logger->Trace(CH_ADDON, "CAddon::Destroy(): %s", this->Location.string().c_str());
				this->UnloadInternal();
				this->IsRunning = false; /* Just to be sure. */
				this->EventApi->Raise(0, EV_ADDON_DESTROYED);
				return; /* Return the thread entirely. */
			}
			case EAddonAction::EnumInterfaces:
			{
				this->EnumInterfaces();
				break;
			}
			case EAddonAction::Load:
			{
				this->LoadInternal();
				break;
			}
			case EAddonAction::Unload:
			{
				this->UnloadInternal();
				break;
			}
			case EAddonAction::Uninstall:
			{
				this->UninstallInternal();
				break;
			}
			case EAddonAction::CheckUpdate:
			{
				this->CheckUpdateInternal();
				break;
			}
			case EAddonAction::Update:
			{
				this->UpdateInternal();
				break;
			}
		}

		this->Flags &= ~EAddonFlags::RunningAction;
	}
}

void CAddon::LoadInternal()
{
	this->Logger->Trace(CH_ADDON, "CAddon::LoadInternal(%s)", this->Location.string().c_str());

	if (this->IsLoaded())
	{
		this->Logger->Debug(CH_ADDON, "Cannot load. Already loaded. (%s)", this->Location.string().c_str());
		return;
	}

	if (this->IsStateLocked())
	{
		this->Logger->Debug(CH_ADDON, "Cannot load. State is locked. (%s)", this->Location.string().c_str());
		return;
	}

	MD5_t md5 = MD5Util::FromFile(this->Location);

	/* If the file is different, than when it was created, refresh the interfaces. */
	if (md5 != this->GetMD5())
	{
		this->EnumInterfaces();
	}

	/* Update MD5. */
	this->MD5 = md5;

	if (!this->HasInterface(EAddonInterfaces::Nexus))
	{
		this->Logger->Debug(CH_ADDON, "Cannot load. No Nexus addon interface. (%s)", this->Location.string().c_str());
		return;
	}

	HMODULE module = LoadLibraryA(this->Location.string().c_str());

	if (!module)
	{
		DWORD lasterror = GetLastError();
		const std::error_condition ecnd = std::system_category().default_error_condition(lasterror);
		this->Logger->Warning(
			CH_ADDON,
			"Cannot load. LoadLibrary(%s) failed: %s (%d)",
			this->Location.string().c_str(),
			ecnd.message().c_str(),
			lasterror
		);
		this->State = EAddonState::NotLoaded;
		return;
	}

	GETADDONDEF getAddonDef = nullptr;

	if (!DLL::FindFunction(module, &getAddonDef, "GetAddonDef"))
	{
		this->Logger->Debug(CH_ADDON, "Cannot load. Interface was set, but is not actually present. (%s)", this->Location.string().c_str());
		this->ModuleInterfaces &= ~EAddonInterfaces::Nexus;
		FreeLibrary(module);
		this->State = EAddonState::NotLoaded;
		return;
	}

	AddonDefRawV1_t* addondef = getAddonDef();

	if (addondef == nullptr)
	{
		this->Logger->Warning(CH_ADDON, "Cannot load. Addon definition was nullptr. (%s)", this->Location.string().c_str());
		FreeLibrary(module);
		this->State = EAddonState::NotLoaded;
		this->Flags |= EAddonFlags::MissingReqs;
		return;
	}

	/* If a def was already set, delete it. */
	if (this->NexusAddonDefV1)
	{
		delete this->NexusAddonDefV1;
	}

	this->NexusAddonDefV1 = new AddonDefV1_t(*addondef);

	if (!addondef->HasMinimumRequirements())
	{
		this->Logger->Warning(CH_ADDON, "Cannot load. Addon definition does not fulfill minimum requirements. (%s)", this->Location.string().c_str());
		FreeLibrary(module);
		this->State = EAddonState::NotLoaded;
		this->Flags |= EAddonFlags::MissingReqs;
		return;
	}

	/* Unset requirements flag. */
	this->Flags &= ~EAddonFlags::MissingReqs;

	if (!this->Config)
	{
		this->Config = this->ConfigMgr->RegisterConfig(this->NexusAddonDefV1->Signature);
	}

	assert(this->Config);

	if (this->IsDuplicate())
	{
		this->Logger->Warning(CH_ADDON, "Canceled load. Addon is a duplicate. (%s)", this->Location.string().c_str());
		FreeLibrary(module);
		this->State = EAddonState::NotLoaded;
		return;
	}

	this->CheckForUpdate();

	if (this->IsUpdateAvailable() && this->ShouldUpdate())
	{
		this->Update();
	}

	if (!this->ShouldLoad())
	{
		/* Should load prints debug reasons, no need to also print here. */
		FreeLibrary(module);
		this->State = EAddonState::NotLoaded;
		return;
	}

	AddonAPI_t* api = ADDONAPI::Get(this->NexusAddonDefV1->APIVersion);

	if (api == nullptr && this->NexusAddonDefV1->APIVersion > 0)
	{
		this->Logger->Warning(
			CH_ADDON,
			"Canceled load. Addon requested an API revision that doesn't exist (%u). (%s)",
			this->NexusAddonDefV1->APIVersion,
			this->Location.string().c_str()
		);
		FreeLibrary(module);
		this->State = EAddonState::NotLoaded;
		return;
	}

	if ((this->NexusAddonDefV1->Flags & EAddonDefFlags::DisableHotloading) == EAddonDefFlags::DisableHotloading)
	{
		this->Flags |= EAddonFlags::StateLocked;
		this->Flags |= EAddonFlags::FileLocked;
	}

	this->Module = module;

	MODULEINFO moduleInfo{};
	GetModuleInformation(GetCurrentProcess(), this->Module, &moduleInfo, sizeof(moduleInfo));
	this->ModuleSize = moduleInfo.SizeOfImage;

	auto start_time = std::chrono::high_resolution_clock::now();
	this->NexusAddonDefV1->Load(api);
	auto end_time = std::chrono::high_resolution_clock::now();
	auto time = end_time - start_time;

	this->EventApi->Raise(EV_ADDON_LOADED, &this->NexusAddonDefV1->Signature);

	this->Config->LastGameBuild = GW2::GetGameBuild();
	this->Config->LastName = this->NexusAddonDefV1->Name;
	this->State = EAddonState::Loaded;
	this->ConfigMgr->SaveConfigs();

	this->Logger->Info(
		CH_ADDON,
		"Loaded addon: %s\n\tSignature: 0x%08X\n\tAddress Space: %p - %p\n\tAPI Version: %d\n\tFlags: %u\n\tDefFlags: %u\n\tTook %u microseconds to load.",
		this->Location.string().c_str(),
		this->NexusAddonDefV1->Signature,
		this->Module,
		((PBYTE)this->Module) + this->ModuleSize,
		this->NexusAddonDefV1->APIVersion,
		this->Flags,
		this->NexusAddonDefV1->Flags,
		time / std::chrono::microseconds(1)
	);
}

void CAddon::UnloadInternal()
{
	this->Logger->Trace(CH_ADDON, "CAddon::UnloadInternal(%s)", this->Location.string().c_str());

	if (!this->IsLoaded())
	{
		this->Logger->Debug(CH_ADDON, "Already unloaded. (%s)", this->Location.string().c_str());
		return;
	}

	if (this->IsStateLocked())
	{
		this->Logger->Debug(CH_ADDON, "Cannot unload. State is locked. (%s)", this->Location.string().c_str());
		return;
	}

	std::string strUnloadInfo;

	if (this->NexusAddonDefV1->Unload)
	{
		auto start_time = std::chrono::high_resolution_clock::now();
		this->NexusAddonDefV1->Unload();
		auto end_time = std::chrono::high_resolution_clock::now();
		auto time = end_time - start_time;

		strUnloadInfo = String::Format("Took %u microseconds to unload.", time / std::chrono::microseconds(1));
	}
	else
	{
		strUnloadInfo = "No unload routine defined.";
	}

	std::string refcleanup = CRefCleanerContext::Get()->CleanupRefs(this->Module, (PBYTE)this->Module + this->ModuleSize);

	if (!refcleanup.empty())
	{
		this->Logger->Warning(CH_ADDON, "(%s) %s", this->Location.string().c_str(), refcleanup.c_str());
	}

	if ((this->Flags & EAddonFlags::Destroying) != EAddonFlags::Destroying)
	{
		this->EventApi->Raise(EV_ADDON_UNLOADED, &this->NexusAddonDefV1->Signature);
	}

	FreeLibrary(this->Module);
	this->Module = nullptr;
	this->ModuleSize = 0;

	this->State = EAddonState::NotLoaded;

	this->Logger->Info(
		CH_ADDON,
		"Unloaded addon: %s\n\tSignature: 0x%08X\n\tFlags: %u\n\t%s",
		this->Location.string().c_str(),
		this->NexusAddonDefV1->Signature,
		this->Flags,
		strUnloadInfo.c_str()
	);
}

std::string CAddon::GetName()
{
	if (this->NexusAddonDefV1)
	{
		return this->NexusAddonDefV1->Name;
	}

	if (this->ArcExtensionDef)
	{
		return this->ArcExtensionDef->Name;
	}

	if (!this->Location.empty())
	{
		return this->Location.filename().string();
	}

	return "";
}

std::string CAddon::GetAuthor()
{
	if (this->NexusAddonDefV1)
	{
		return this->NexusAddonDefV1->Author;
	}

	return "";
}

std::string CAddon::GetDescription()
{
	if (this->NexusAddonDefV1)
	{
		return this->NexusAddonDefV1->Description;
	}

	return "";
}

std::string CAddon::GetVersion()
{
	if (this->NexusAddonDefV1)
	{
		return this->NexusAddonDefV1->Version.string();
	}

	if (this->ArcExtensionDef)
	{
		return this->ArcExtensionDef->Build;
	}

	return "";
}

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

bool CAddon::IsRunningAction() const
{
	return (this->Flags & EAddonFlags::RunningAction) == EAddonFlags::RunningAction;
}

bool CAddon::IsDestroying() const
{
	return (this->Flags & EAddonFlags::Destroying) == EAddonFlags::Destroying;
}

bool CAddon::IsFileLocked() const
{
	return (this->Flags & EAddonFlags::FileLocked) == EAddonFlags::FileLocked;
}

bool CAddon::IsStateLocked() const
{
	return (this->Flags & EAddonFlags::StateLocked) == EAddonFlags::StateLocked;
}

bool CAddon::IsMissingRequirements() const
{
	return (this->Flags & EAddonFlags::MissingReqs) == EAddonFlags::MissingReqs;
}

bool CAddon::IsUninstalled() const
{
	return (this->Flags & EAddonFlags::Uninstalled) == EAddonFlags::Uninstalled;
}

bool CAddon::IsUpdateAvailable() const
{
	return (this->Flags & EAddonFlags::UpdateAvailable) == EAddonFlags::UpdateAvailable;
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

	GETADDONDEF getAddonDef = nullptr;

	if (DLL::FindFunction(module, &getAddonDef, "GetAddonDef"))
	{
		interfaces |= EAddonInterfaces::Nexus;

		AddonDefRawV1_t* rawdef = getAddonDef();

		if (rawdef)
		{
			if (this->NexusAddonDefV1)
			{
				delete this->NexusAddonDefV1;
				this->NexusAddonDefV1 = nullptr;
			}

			this->NexusAddonDefV1 = new AddonDefV1_t(*rawdef);
		}
	}

	void* discard = nullptr;

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

bool CAddon::SupportsPreReleases() const
{
	if (this->NexusAddonDefV1)
	{
		switch (this->NexusAddonDefV1->Provider)
		{
			case EUpdateProvider::GitHub:
			{
				return true;
			}
		}
	}

	return false;
}

std::string CAddon::GetProjectPageURL() const
{
	if (this->NexusAddonDefV1)
	{
		switch (this->NexusAddonDefV1->Provider)
		{
			case EUpdateProvider::GitHub:
			{
				return this->NexusAddonDefV1->UpdateLink;
			}
		}
	}

	return "";
}

void CAddon::CheckUpdateInternal(bool aIsScheduled)
{
	if (this->IsUpdateAvailable())
	{
		/* Already checked. */
		return;
	}

	/* Automatic internal checks. */
	if (aIsScheduled)
	{
		long long now = Time::GetTimestamp();

		/// Scheduled checks only run, when 
		/// - the last scheduled check was 30 minutes ago
		/// - this version is disabled and the last check was over 5 minutes ago

		// TODO: Make addon thread wake every 5 minutes. Queue update check.

		bool doCheck = false;

		if (this->LastCheckedTimestamp - now >= 30 * 60)
		{
			this->Logger->Trace(CH_ADDON, "Scheduled update check: DeltaT > 1800s. (%s)", this->Location.string().c_str());
			doCheck = true;
		}
		else if (this->IsVersionDisabled() && (this->LastCheckedTimestamp - now >= 5 * 60))
		{
			this->Logger->Trace(CH_ADDON, "Scheduled update check: Version disabled. DeltaT > 300s. (%s)", this->Location.string().c_str());
			doCheck = true;
		}

		if (!doCheck)
		{
			return;
		}

		this->LastCheckedTimestamp = now;
	}

	if (!this->NexusAddonDefV1)
	{
		this->Logger->Warning(CH_ADDON, "Canceled update check. No Nexus addon interface. (%s)", this->Location.string().c_str());
		return;
	}

	switch (this->NexusAddonDefV1->Provider)
	{
		case EUpdateProvider::Raidcore:
		{
			this->Logger->Warning(CH_ADDON, "Using unimplemented provider. (%s)", this->Location.string().c_str());
			break;
		}
		case EUpdateProvider::GitHub:
		{
			this->CheckUpdateViaGitHub();
			break;
		}
		case EUpdateProvider::Direct:
		{
			this->CheckUpdateViaDirect();
			break;
		}
		case EUpdateProvider::Self:
		{
			this->Logger->Trace(CH_ADDON, "Canceled update check. Provider is self. (%s)", this->Location.string().c_str());
			break;
		}
	}
}

bool CAddon::CheckUpdateViaGitHub()
{
	this->Logger->Trace(CH_ADDON, "CheckUpdateViaGitHub (%s)", this->Location.string().c_str());

	return false;
}

bool CAddon::CheckUpdateViaDirect()
{
	this->Logger->Trace(CH_ADDON, "CheckUpdateViaDirect (%s)", this->Location.string().c_str());

	return false;
}

bool CAddon::UpdateInternal()
{
	this->Logger->Trace(CH_ADDON, "CAddon::UpdateInternal(%s)", this->Location.string().c_str());

	if (!this->IsUpdateAvailable())
	{
		this->Logger->Debug(CH_ADDON, "Can't update. No update available. (%s)", this->Location.string().c_str());
		return false;
	}

	/* If the update is already locally available, just apply it. */
	if (!this->UpdateLocal.empty())
	{
		//this->Location
	}

	/* If the update is still on a remote, download it. */
	if (!this->UpdateRemote.empty())
	{
		//this->Location
	}

	return false;
}
