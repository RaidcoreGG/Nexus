///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Loader.cpp
/// Description  :  Addon loader component for managing addons.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Loader.h"

#include <Windows.h>

#include "httplib/httplib.h"

#include "Context.h"
#include "Index.h"
#include "FuncDefs.h"
#include "Util/src/DLL.h"

CLoader::CLoader(CLogHandler* aLogger, CSettings* aSettings, std::vector<signed int> aWhitelist)
{
	this->Logger = aLogger;
	this->Settings = aSettings;

	this->PreferenceMgr = new CPreferenceMgr(aLogger, Index::F_ADDONCONFIG, aWhitelist.size() > 0);

	this->LibraryMgr = new CLibraryMgr(aLogger);
	this->LibraryMgr->AddSource("https://api.raidcore.gg/addonlibrary");
	this->LibraryMgr->AddSource("https://api.raidcore.gg/arcdpslibrary");
}

CLoader::~CLoader()
{
	this->IsCanceled = true;
	this->ProcessorCountdown = 0;
	this->ConVar.notify_one();

	this->Logger = nullptr;
	this->Settings = nullptr;
}

UINT CLoader::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	/* if not addon dir update */
	if (uMsg != WM_ADDONDIRUPDATE)
	{
		return uMsg;
	}

	PIDLIST_ABSOLUTE* ppidl;
	LONG event;
	HANDLE notificationLock = SHChangeNotification_Lock((HANDLE)wParam, static_cast<DWORD>(lParam), &ppidl, &event);

	/* if lock cannot be created */
	if (notificationLock == 0)
	{
		return uMsg;
	}

	/* if not dir or item update */
	if (!(event == SHCNE_UPDATEITEM || event == SHCNE_UPDATEDIR))
	{
		return uMsg;
	}

	char path[MAX_PATH];

	/* if path cannot be fetched */
	if (!SHGetPathFromIDListA(ppidl[0], path))
	{
		/* unlock before returning for further processing */
		SHChangeNotification_Unlock(notificationLock);
		return uMsg;
	}

	if (Index::D_GW2_ADDONS == std::string(path))
	{
		this->NotifyChanges();
	}

	SHChangeNotification_Unlock(notificationLock);

	return 0;
}

void CLoader::NotifyChanges()
{
	if (IsCanceled) { return; }

	this->ProcessorCountdown = LOADER_WAITTIME_MS;
	this->ConVar.notify_one();
}

std::string CLoader::GetOwner(void* aAddress)
{
	if (aAddress == nullptr)
	{
		return NULLSTR;
	}

	const std::lock_guard<std::mutex> lock(AddonsMutex);
	{
		for (CAddon& addon : this->Addons)
		{
			if (addon.OwnsAddress(aAddress))
			{
				return addon.GetName();
			}
		}

		CContext* ctx = CContext::GetContext();

		void* startAddress = ctx->GetModule();
		void* endAddress = ((PBYTE)ctx->GetModule()) + ctx->GetModuleSize();

		if (aAddress >= startAddress && aAddress <= endAddress)
		{
			return "Nexus";
		}
	}

	return NULLSTR;
}

void CLoader::ProcessChanges()
{
	/* frontload some async work */
	this->LibraryMgr->Update();
	this->GetGameBuild();

	for (;;)
	{
		EAddonAction action = EAddonAction::None;

		{
			static std::mutex s_Lock{}; /* dummy for cvar */
			std::unique_lock<std::mutex> lockThread(s_Lock);
			this->ConVar.wait(lockThread);
		}

#ifdef _DEBUG
		auto start_time = std::chrono::high_resolution_clock::now();
#endif

		while (this->ProcessorCountdown > 0 && !this->IsCanceled)
		{
			Sleep(1);
			this->ProcessorCountdown -= 1;
		}

#ifdef _DEBUG
		auto end_time = std::chrono::high_resolution_clock::now();
		auto time = end_time - start_time;
		Logger->Trace(CH_LOADER, "Processing changes after waiting for %ums.", time / std::chrono::milliseconds(1));
#endif

		if (this->IsCanceled) { return; }

		this->DetectChanges();
		this->Discover();
	}
}

void CLoader::DetectChanges()
{

}

void CLoader::Discover()
{
	/* TODO: here or in the main process function, recheck previously erroneous (e.g. duplicate) addons */

	/* check /addons directory for untracked */
	for (const std::filesystem::directory_entry entry : std::filesystem::directory_iterator(Index::D_GW2_ADDONS))
	{
		std::filesystem::path path = entry.path();

		/* check this first, because if it's already tracked it's definitely valid */
		if (this->IsTracked(path))
		{
			continue;
		}

		/* if not tracked, the path needs to be validated */
		if (!this->PathIsValid(path))
		{
			continue;
		}

		/* if same module is already loaded, skip */
		if (this->IsTracked(MD5Util::FromFile(path)))
		{
			/* TODO: maybe track as incompatible */
			continue;
		}

		if (path.extension() == EXT_DLL)
		{
			EAddonType type = this->GetAddonType(path);

			CAddon addon = CAddon(EAddonType::Nexus);
		}
		else if (path.extension() == EXT_UNINSTALL)
		{
			/* TODO: move this to a temp folder and on startup just delete it entirely */
			try
			{
				std::filesystem::remove(path);
			}
			catch (std::filesystem::filesystem_error fErr)
			{
				this->Logger->Debug(CH_LOADER, "%s", fErr.what());
			}
		}
	}
}

bool CLoader::PathIsValid(std::filesystem::path aPath)
{
	if (std::filesystem::is_symlink(aPath))
	{
		/* if symlink call recursively */
		return this->IsValid(std::filesystem::read_symlink(aPath));
	}

	if (!std::filesystem::exists(aPath))
	{
		return false;
	}

	if (std::filesystem::is_directory(aPath))
	{
		return false;
	}

	if (std::filesystem::file_size(aPath) == 0)
	{
		return false;
	}

	return true;
}

bool IsTracked(std::filesystem::path aPath)
{
	throw; /* Not impl. */
}

EAddonType CLoader::GetAddonType(std::filesystem::path aPath)
{
	assert(this->PathIsValid(aPath));

#ifdef _DEBUG
	HMODULE module = LoadLibraryExA(aPath.string().c_str(), 0, DONT_RESOLVE_DLL_REFERENCES);

	if (!module)
	{
		const std::error_condition ecnd = std::system_category().default_error_condition(GetLastError());
		this->Logger->Warning(CH_LOADER, "Failed LoadLibrary on \"%s\" during discovery.\nError Code %u : %s", aPath.filename().string().c_str(), ecnd.value(), ecnd.message().c_str());

		return EAddonType::Incompatible;
	}

	GETADDONDEF getAddonDef = nullptr;
	if (DLL::FindFunction(module, &getAddonDef, "GetAddonDef"))
	{
		FreeLibrary(module);
		return EAddonType::Nexus;
	}

	void* get_init_addr = nullptr;
	void* get_release_addr = nullptr;
	if (DLL::FindFunction(module, &get_init_addr, "get_init_addr") &&
		DLL::FindFunction(module, &get_release_addr, "get_release_addr"))
	{
		FreeLibrary(module);
		return EAddonType::ArcDPS;
	}

	return EAddonType::Incompatible;
#else
	FIXME: do this properly without the deprecated flag
#endif
}

void CLoader::GetGameBuild()
{
	/* prepare client request */
	httplib::Client client("http://assetcdn.101.arenanetworks.com");
	client.enable_server_certificate_verification(false);

	std::string buildStr;

	size_t bytesWritten = 0;
	auto result = client.Get("/latest64/101", [&](const char* data, size_t data_length) {
		buildStr += data;
		bytesWritten += data_length;
		return true;
	});

	if (!result || result->status != 200)
	{
		this->Logger->Warning(CH_LOADER, "Error fetching \"http://assetcdn.101.arenanetworks.com/latest64/101\"\nError: %s", httplib::to_string(result.error()).c_str());
		return;
	}

	this->GameBuild = std::stoi(buildStr);
}
