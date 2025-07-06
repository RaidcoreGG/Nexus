///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Loader.cpp
/// Description  :  Addon loader component.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Loader.h"

#include <shlobj.h>

#include "Util/Strings.h"
#include "Util/MD5.h"

CLoader::CLoader(CLogApi* aLogger, RenderContext_t* aRenderContext, IADDON_FACTORY aFactoryFunction, std::filesystem::path aDirectory)
{
	this->Logger = aLogger;
	this->RenderContext = aRenderContext;
	this->CreateAddon = aFactoryFunction;
	this->Directory = aDirectory;

	if (!std::filesystem::exists(aDirectory))
	{
		std::filesystem::create_directories(aDirectory);
	}
}

CLoader::~CLoader()
{
	this->Shutdown();
}

void CLoader::Shutdown()
{
	this->DeinitDirectoryUpdates();

	/* Stop the processor thread to handle the shutdown. */
	this->IsRunning = false;
	this->NotifyChanges();

	/* Join processing thread. */
	if (this->ProcThread.joinable())
	{
		this->ProcThread.join();
	}
}

void CLoader::Init()
{
	const std::lock_guard<std::mutex> lock(this->FSMutex);

	if (this->FSItemList != nullptr && this->FSNotifierID != 0)
	{
		/* Already initialized. */
		return;
	}

	if (this->RenderContext->Window.Handle == nullptr)
	{
		if (this->RenderContext->Metrics.FrameCount == 0)
		{
			this->Logger->Debug(CH_LOADER, "Window handle is null. Init called before first frame.");
		}
		else
		{
			this->Logger->Warning(CH_LOADER, "Window handle is null at Frame ID %u. Cannot initialize automatic addon loading.", this->RenderContext->Metrics.FrameCount);
		}
		return;
	}

	if (this->RenderContext->Metrics.FrameCount == 1)
	{
		this->Logger->Debug(CH_LOADER, "Init called during first frame.");
	}
	else
	{
		this->Logger->Debug(CH_LOADER, "Late init. Frame ID: %u", this->RenderContext->Metrics.FrameCount);
	}

	/* Already start the thread. */
	if (this->RenderContext->SwapChain && !this->IsRunning)
	{
		this->IsRunning = true;
		this->ProcThread = std::thread(&CLoader::ProcessChanges, this);
	}
	else
	{
		this->Logger->Debug(CH_LOADER, "SwapChain missing, cannot initialize loader thread.");
	}

	/* Setup wndproc directory updates. */
	std::wstring addonDirW = String::ToWString(this->Directory.string());
	HRESULT hresult = SHParseDisplayName(
		addonDirW.c_str(),
		0,
		&this->FSItemList,
		0xFFFFFFFF,
		0
	);

	if (this->FSItemList == 0)
	{
		this->Logger->Warning(CH_LOADER, "Automatic addon loading disabled. Reason: SHParseDisplayName(...) returned %d.", hresult);
		return;
	}

	SHChangeNotifyEntry changeentry{};
	changeentry.pidl = this->FSItemList;
	changeentry.fRecursive = false;
	this->FSNotifierID = SHChangeNotifyRegister(
		this->RenderContext->Window.Handle,
		SHCNRF_InterruptLevel | SHCNRF_NewDelivery,
		SHCNE_UPDATEITEM | SHCNE_UPDATEDIR,
		WM_ADDONDIRUPDATE,
		1,
		&changeentry
	);

	if (this->FSNotifierID == 0)
	{
		this->Logger->Warning(CH_LOADER, "Automatic addon loading disabled. Reason: SHChangeNotifyRegister(...) returned 0.");
		return;
	}

	this->Logger->Info(CH_LOADER, "Automatic addon loading enabled.");
}

UINT CLoader::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg != WM_ADDONDIRUPDATE)
	{
		return uMsg;
	}

	PIDLIST_ABSOLUTE* ppidl;
	LONG event;
	HANDLE notificationLock = SHChangeNotification_Lock((HANDLE)wParam, static_cast<DWORD>(lParam), &ppidl, &event);

	if (notificationLock == nullptr)
	{
		return uMsg;
	}

	if (event == SHCNE_UPDATEITEM || event == SHCNE_UPDATEDIR)
	{
		char path[MAX_PATH];
		if (SHGetPathFromIDListA(ppidl[0], path))
		{
			if (this->Directory == std::string(path))
			{
				this->NotifyChanges();
			}
		}
	}

	SHChangeNotification_Unlock(notificationLock);

	return 0;
}

void CLoader::NotifyChanges()
{
	std::lock_guard<std::mutex> lock(this->Mutex);
	this->ConVar.notify_one();

	this->Logger->Trace(CH_LOADER, "NotifyChanges()");
}

void CLoader::Add(std::filesystem::path aPath)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	/* Ignore, if not valid. */
	if (!this->IsValid(aPath))
	{
		this->Logger->Info(CH_LOADER, "Add(%s) failed. Not a valid path.", aPath.string().c_str());
		return;
	}

	IAddon* addon = this->CreateAddon(aPath);

	this->Addons.push_back(addon);

	addon->Load();
}

void CLoader::LoadSafe(std::filesystem::path aPath)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);
	this->Load(aPath);
}

void CLoader::UnloadSafe(std::filesystem::path aPath)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);
	this->Unload(aPath);
}

void CLoader::UninstallSafe(std::filesystem::path aPath)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);
	this->Uninstall(aPath);
}

IAddon* CLoader::GetOwner(void* aAddress) const
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	for (IAddon* addon : this->Addons)
	{
		if (addon->OwnsAddress(aAddress))
		{
			return addon;
		}
	}

	return nullptr;
}

bool CLoader::IsTrackedSafe(uint32_t aSignature, IAddon* aAddon) const
{
	std::lock_guard<std::mutex> lock(this->Mutex);

	return this->IsTracked(aSignature, aAddon);
}

bool CLoader::IsTrackedSafe(std::filesystem::path aPath, IAddon* aAddon) const
{
	std::lock_guard<std::mutex> lock(this->Mutex);

	return this->IsTracked(aPath, aAddon);
}

bool CLoader::IsTrackedSafe(MD5_t aMD5, IAddon* aAddon) const
{
	std::lock_guard<std::mutex> lock(this->Mutex);

	return this->IsTracked(aMD5, aAddon);
}

std::vector<IAddon*> CLoader::GetAddons() const
{
	std::lock_guard<std::mutex> lock(this->Mutex);

	return this->Addons;
}

void CLoader::DeinitDirectoryUpdates()
{
	const std::lock_guard<std::mutex> lock(this->FSMutex);

	/* Deregister wndproc directory updates. */
	if (this->FSNotifierID != 0)
	{
		SHChangeNotifyDeregister(this->FSNotifierID);
		this->FSNotifierID = 0;
	}

	if (this->FSItemList != 0)
	{
		ILFree(this->FSItemList);
		this->FSItemList = 0;
	}
}

bool CLoader::IsValid(const std::filesystem::path& aPath)
{
	if (aPath.empty()) { return false; }

	if (std::filesystem::is_symlink(aPath))
	{
		/* Resolve symlinks recursively. A valid file has to be at the end of it. */
		return this->IsValid(std::filesystem::read_symlink(aPath));
	}

	if (!std::filesystem::exists(aPath)) { return false; }

	if (std::filesystem::is_directory(aPath)) { return false; }

	if (std::filesystem::file_size(aPath) == 0) { return false; }

	if (aPath.extension() != ".dll") { return false; }

	return true;
}

void CLoader::ProcessChanges()
{
	this->Logger->Trace(CH_LOADER, "Init. Discovering addons.");
	this->Discover();

	while (this->IsRunning)
	{
		std::unique_lock<std::mutex> lock(this->Mutex);
		this->ConVar.wait_for(lock, std::chrono::milliseconds(5000));

		this->Logger->Trace(CH_LOADER, "Processing changes.");

		if (!this->IsRunning)
		{
			/* Early break out of the loop. */
			break;
		}

		std::vector<IAddon*> movedOrDeleted;

		/* Recheck existing addons. */
		for (IAddon* addon : this->Addons)
		{
			/* Check if the file is still on disk. */
			if (!std::filesystem::exists(addon->GetLocation()))
			{
				movedOrDeleted.push_back(addon);
				continue;
			}

			/* Get the MD5 of the current file on disk. */
			MD5_t md5 = MD5Util::FromFile(addon->GetLocation());

			/* If the MD5 has changed, reload the addon. */
			if (md5 != addon->GetMD5())
			{
				addon->Unload();
				addon->Load();
				continue;
			}
		}

		/* Check for new addons. */
		for (const std::filesystem::directory_entry entry : std::filesystem::directory_iterator(this->Directory))
		{
			std::filesystem::path path = entry.path();

			/* Ignore, if not valid. */
			if (!this->IsValid(path))
			{
				continue;
			}

			/* Ignore, if already tracked. */
			if (this->IsTracked(path))
			{
				continue;
			}

			/* Get the MD5 of the file on disk. */
			MD5_t md5 = MD5Util::FromFile(path);

			bool wasMoved = false;

			for (auto it = movedOrDeleted.begin(); it != movedOrDeleted.end();)
			{
				IAddon* addon = *it;
				/* If the MD5 matches, safe to assume it's the same file but was moved. */
				if (addon->GetMD5() == md5)
				{
					/* Set the new location for the addon. */
					addon->SetLocation(path);

					/* Remove the addon from the temp/move list, it is accounted for. */
					it = movedOrDeleted.erase(it);

					/* Ensure no new addon is created. */
					wasMoved = true;
					break;
				}
				else
				{
					it++;
				}
			}

			/* If it is indeed a new file and not one that was moved, create a new addon. */
			if (!wasMoved)
			{
				IAddon* addon = this->CreateAddon(path);

				this->Addons.push_back(addon);

				addon->Load();
			}
		}

		/* If any addons are left in here, they were in fact deleted or moved out of tracking. */
		for (IAddon* addon : movedOrDeleted)
		{
			if (addon->IsLoaded())
			{
				this->Logger->Debug(CH_LOADER, "Addon no longer tracked. Unloading: %s", addon->GetLocation().empty() ? "(null)" : addon->GetLocation().string().c_str());
				addon->Unload();
			}
			else
			{
				this->Logger->Debug(CH_LOADER, "Addon no longer tracked. Deleting: %s", addon->GetLocation().empty() ? "(null)" : addon->GetLocation().string().c_str());

				auto it = std::find(this->Addons.begin(), this->Addons.end(), addon);

				if (it != this->Addons.end())
				{
					this->Addons.erase(it);
				}

				delete addon;
			}
		}
	}

	this->Logger->Trace(CH_LOADER, "Shutdown. Clearing addons.");

	for (IAddon* addon : this->Addons)
	{
		delete addon;
	}

	this->Addons.clear();
}

void CLoader::Discover()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	/* check /addons directory for untracked */
	for (const std::filesystem::directory_entry entry : std::filesystem::directory_iterator(this->Directory))
	{
		const std::filesystem::path& path = entry.path();

		/* Ignore, if not valid. */
		if (!this->IsValid(path))
		{
			continue;
		}

		IAddon* addon = this->CreateAddon(path);

		this->Addons.push_back(addon);

		addon->Load();
	}
}

bool CLoader::IsTracked(uint32_t aSignature, IAddon* aAddon) const
{
	for (IAddon* addon : this->Addons)
	{
		if (addon == aAddon) { continue; }

		if (addon->GetSignature() == aSignature)
		{
			return true;
		}
	}

	return false;
}

bool CLoader::IsTracked(std::filesystem::path aPath, IAddon* aAddon) const
{
	for (IAddon* addon : this->Addons)
	{
		if (addon == aAddon) { continue; }

		if (addon->GetLocation() == aPath)
		{
			return true;
		}
	}

	return false;
}

bool CLoader::IsTracked(MD5_t aMD5, IAddon* aAddon) const
{
	if (aMD5.empty())
	{
		return false;
	}

	for (IAddon* addon : this->Addons)
	{
		if (addon == aAddon) { continue; }

		if (addon->GetMD5() == aMD5)
		{
			return true;
		}
	}

	return false;
}

void CLoader::Load(std::filesystem::path aPath)
{
	for (IAddon* addon : this->Addons)
	{
		if (addon->GetLocation() == aPath)
		{
			this->Logger->Debug(CH_LOADER, "CLoaderBase::Load(%s)", aPath.string().c_str());
			addon->Load();
			return;
		}
	}
}

void CLoader::Unload(std::filesystem::path aPath)
{
	for (IAddon* addon : this->Addons)
	{
		if (addon->GetLocation() == aPath)
		{
			this->Logger->Debug(CH_LOADER, "CLoaderBase::Unload(%s)", aPath.string().c_str());
			addon->Unload();
			return;
		}
	}
}

void CLoader::Uninstall(std::filesystem::path aPath)
{
	for (IAddon* addon : this->Addons)
	{
		if (addon->GetLocation() == aPath)
		{
			this->Logger->Debug(CH_LOADER, "CLoaderBase::Uninstall(%s)", aPath.string().c_str());
			addon->Uninstall();
			return;
		}
	}
}
