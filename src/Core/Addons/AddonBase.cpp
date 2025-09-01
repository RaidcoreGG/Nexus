///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AddonBase.cpp
/// Description  :  Addon base implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Addon.h"

#include <psapi.h>
#include <windows.h>

#include "Core/Addons/API/ApiBuilder.h"
#include "Core/Context.h"
#include "Engine/Cleanup/RefCleanerContext.h"
#include "GW2/Build/BuildInfo.h"
#include "Util/DLL.h"
#include "Util/MD5.h"
#include "Util/Strings.h"

CAddon::CAddon(std::filesystem::path aLocation)
{
	this->Location = aLocation;
	this->MD5 = MD5Util::FromFile(aLocation);

	CContext* ctx = CContext::GetContext();
	this->Logger    = ctx->GetLogger();
	this->Loader    = ctx->GetLoader();
	this->EventApi  = ctx->GetEventApi();
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
