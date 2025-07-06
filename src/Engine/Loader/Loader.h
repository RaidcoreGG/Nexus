///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Loader.h
/// Description  :  Addon loader component.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef LOADER_H
#define LOADER_H

#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <shtypes.h>
#include <vector>
#include <windows.h>

#include "Engine/Logging/LogApi.h"
#include "Engine/Renderer/RdrContext.h"
#include "LdrAddonBase.h"

constexpr const uint32_t WM_ADDONDIRUPDATE = WM_USER + 101;
constexpr const char* CH_LOADER = "Loader";

///----------------------------------------------------------------------------------------------------
/// CLoader Class
///----------------------------------------------------------------------------------------------------
class CLoader
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	CLoader(
		CLogApi*              aLogger,
		RenderContext_t*      aRenderContext,
		IADDON_FACTORY        aFactoryFunction,
		std::filesystem::path aDirectory
	);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CLoader();

	void Shutdown();

	///----------------------------------------------------------------------------------------------------
	/// Init:
	/// 	Initializes the necessary resouces to receive directory updates and the processor thread.
	///----------------------------------------------------------------------------------------------------
	void Init();

	///----------------------------------------------------------------------------------------------------
	/// WndProc:
	/// 	Returns 0 if message was processed.
	///----------------------------------------------------------------------------------------------------
	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	///----------------------------------------------------------------------------------------------------
	/// NotifyChanges:
	/// 	Notifies that something in the addon directory changed.
	///----------------------------------------------------------------------------------------------------
	void NotifyChanges();

	///----------------------------------------------------------------------------------------------------
	/// Add:
	/// 	Adds a specific path for addon tracking.
	///----------------------------------------------------------------------------------------------------
	void Add(std::filesystem::path aPath);

	///----------------------------------------------------------------------------------------------------
	/// Load:
	/// 	Loads the addon matching the signature.
	///----------------------------------------------------------------------------------------------------
	void LoadSafe(std::filesystem::path aPath);

	///----------------------------------------------------------------------------------------------------
	/// Unload:
	/// 	Unloads the addon matching the signature.
	///----------------------------------------------------------------------------------------------------
	void UnloadSafe(std::filesystem::path aPath);

	///----------------------------------------------------------------------------------------------------
	/// Uninstall:
	/// 	Uninstalls the addon matching the path.
	///----------------------------------------------------------------------------------------------------
	void UninstallSafe(std::filesystem::path aPath);

	///----------------------------------------------------------------------------------------------------
	/// GetOwner:
	/// 	Returns the owner of the passed address, or nullptr.
	///----------------------------------------------------------------------------------------------------
	IAddon* GetOwner(void* aAddress) const;

	///----------------------------------------------------------------------------------------------------
	/// IsTrackedSafe:
	/// 	Returns true, if the provided signature is an already tracked addon.
	/// 	[optional] aAddon: Do not compare against the provided addon.
	///----------------------------------------------------------------------------------------------------
	bool IsTrackedSafe(uint32_t aSignature, IAddon* aAddon = nullptr) const;

	///----------------------------------------------------------------------------------------------------
	/// IsTrackedSafe:
	/// 	Returns true, if the provided path is an already tracked addon.
	/// 	[optional] aAddon: Do not compare against the provided addon.
	///----------------------------------------------------------------------------------------------------
	bool IsTrackedSafe(std::filesystem::path aPath, IAddon* aAddon = nullptr) const;

	///----------------------------------------------------------------------------------------------------
	/// IsTrackedSafe:
	/// 	Returns true, if the provided MD5 is an already tracked addon.
	/// 	[optional] aAddon: Do not compare against the provided addon.
	///----------------------------------------------------------------------------------------------------
	bool IsTrackedSafe(MD5_t aMD5, IAddon* aAddon = nullptr) const;

	///----------------------------------------------------------------------------------------------------
	/// GetAddons:
	/// 	Returns all tracked addons.
	///----------------------------------------------------------------------------------------------------
	std::vector<IAddon*> GetAddons() const;

	private:
	CLogApi*                Logger        = nullptr;
	RenderContext_t*        RenderContext = nullptr;

	std::filesystem::path   Directory;

	std::mutex              FSMutex;
	PIDLIST_ABSOLUTE        FSItemList    = nullptr;
	ULONG                   FSNotifierID  = 0;

	std::thread             ProcThread;
	bool                    IsRunning     = false;

	mutable std::mutex      Mutex;
	std::condition_variable ConVar;

	IADDON_FACTORY          CreateAddon;
	std::vector<IAddon*>    Addons;

	///----------------------------------------------------------------------------------------------------
	/// DeinitDirectoryUpdates:
	/// 	Deinitializes the necessary resouces to receive directory updates.
	///----------------------------------------------------------------------------------------------------
	void DeinitDirectoryUpdates();

	///----------------------------------------------------------------------------------------------------
	/// IsValid:
	/// 	Returns true if the provided addon path is valid for loading.
	///----------------------------------------------------------------------------------------------------
	bool IsValid(const std::filesystem::path& aPath);

	///----------------------------------------------------------------------------------------------------
	/// ProcessChanges:
	/// 	Detects and processes any changes to addons.
	///----------------------------------------------------------------------------------------------------
	void ProcessChanges();

	///----------------------------------------------------------------------------------------------------
	/// Discover:
	/// 	Discovers the addons on disk and creates them if they are valid.
	/// 	Does not check if the file is already tracked, only call this once for the initial discovery.
	///----------------------------------------------------------------------------------------------------
	void Discover();

	///----------------------------------------------------------------------------------------------------
	/// IsTracked:
	/// 	Returns true if the provided signature is an already tracked addon.
	/// 	[optional] aAddon: Do not compare against the provided addon.
	///----------------------------------------------------------------------------------------------------
	bool IsTracked(uint32_t aSignature, IAddon* aAddon = nullptr) const;

	///----------------------------------------------------------------------------------------------------
	/// IsTracked:
	/// 	Returns true if the provided path is an already tracked addon.
	/// 	[optional] aAddon: Do not compare against the provided addon.
	///----------------------------------------------------------------------------------------------------
	bool IsTracked(std::filesystem::path aPath, IAddon* aAddon = nullptr) const;

	///----------------------------------------------------------------------------------------------------
	/// IsTracked:
	/// 	Returns true if the provided MD5 is an already tracked addon.
	/// 	[optional] aAddon: Do not compare against the provided addon.
	///----------------------------------------------------------------------------------------------------
	bool IsTracked(MD5_t aMD5, IAddon* aAddon = nullptr) const;

	///----------------------------------------------------------------------------------------------------
	/// Load:
	/// 	Loads the addon matching the signature.
	///----------------------------------------------------------------------------------------------------
	void Load(std::filesystem::path aPath);

	///----------------------------------------------------------------------------------------------------
	/// Unload:
	/// 	Unloads the addon matching the signature.
	///----------------------------------------------------------------------------------------------------
	void Unload(std::filesystem::path aPath);

	///----------------------------------------------------------------------------------------------------
	/// Uninstall:
	/// 	Uninstalls the addon matching the path.
	///----------------------------------------------------------------------------------------------------
	void Uninstall(std::filesystem::path aPath);
};

#endif
