///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Updater.cpp
/// Description  :  Implementation of the updater.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Updater.h"

#include "Runtime/Runtime.h"
using namespace Raidcore::Nexus;

#include "Core/Index/Index.h"
#include "Util/Paths.h"

namespace Raidcore::Nexus::Network
{
	Updater::Updater(Core::LogApi* aLogger)
	{
		this->Logger = aLogger;

		this->UpdateThread = std::thread(&Updater::Run, this);
	}

	Updater::~Updater()
	{
		if (this->UpdateThread.joinable())
		{
			this->UpdateThread.join();
		}

		/* Release the patch mutex, so other instances can update Nexus. */
		if (this->UpdateMutex)
		{
			ReleaseMutex(this->UpdateMutex);
			CloseHandle(this->UpdateMutex);
		}
	}

	const Version_t& Updater::GetRemoteVersion()
	{
		if (this->RemoteVersion != Version_t{})
		{
			return this->RemoteVersion;
		}

		/* The client is not dependency injected, as we only create it on demand. */
		CHttpClient& raidcoreapi = Runtime::Get().Network().GetHttpClient("https://api.raidcore.gg");

		/* Request version info, bypass cache. */
		HttpResponse_t result = raidcoreapi.Get("/nexusversion", "", 0);

		if (!result.Success())
		{
			this->Logger->Warning(
				CH_SELFUPDATER,
				"Failed to fetch Nexus version.\n\tStatus: %s\n\tError: %s",
				result.Status(),
				result.Error.c_str()
			);
			return this->RemoteVersion;
		}

		json versionJSON = result.ContentJSON();

		if (versionJSON.is_null())
		{
			this->Logger->Warning(CH_SELFUPDATER, "Failed to fetch Nexus version. JSON was null.");
			return this->RemoteVersion;
		}

		/* Extract the version. */
		if (versionJSON["Major"].is_null() ||
			versionJSON["Minor"].is_null() ||
			versionJSON["Build"].is_null() ||
			versionJSON["Revision"].is_null())
		{
			this->Logger->Warning(CH_SELFUPDATER, "Failed to fetch Nexus version. Incomplete version info.");
			return this->RemoteVersion;
		}

		versionJSON["Major"].get_to(this->RemoteVersion.Major);
		versionJSON["Minor"].get_to(this->RemoteVersion.Minor);
		versionJSON["Build"].get_to(this->RemoteVersion.Build);
		versionJSON["Revision"].get_to(this->RemoteVersion.Revision);

		/* Store the changelog. */
		if (!versionJSON["Changelog"].is_null())
		{
			versionJSON["Changelog"].get_to(this->Changelog);
		}

		return this->RemoteVersion;
	}

	bool Updater::IsUpdateAvailable()
	{
		return this->RemoteVersion > Runtime::Get().GetVersion();
	}

	const std::string& Updater::GetChangelog()
	{
		return this->Changelog;
	}

	bool Updater::CreatePatchMutex()
	{
		/* System-wide mutex to protect updates during multiboxing. */
		this->UpdateMutex = CreateMutexA(0, true, "RCGG-Mutex-Patch-Nexus");

		/* If it exists, a handle to it is returned, but last error is set. */
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			/* Close the handle, we don't control it. */
			if (this->UpdateMutex)
			{
				CloseHandle(this->UpdateMutex);
				this->UpdateMutex = NULL;
			}

			this->Logger->Info(CH_SELFUPDATER, "Cannot patch Nexus, mutex locked.");
			return false;
		}

		return true;
	}

	void Updater::CleanupUpdateFiles()
	{
		/* Ensure .dll.old path is not claimed. */
		if (std::filesystem::exists(Index(EPath::NexusDLL_Old)))
		{
			try
			{
				std::filesystem::remove(Index(EPath::NexusDLL_Old));
			}
			catch (std::filesystem::filesystem_error fErr)
			{
				std::filesystem::path fallback = Path::GetUnused(Index(EPath::DIR_TEMP) / "d3d11.dll.old");
				std::filesystem::rename(Index(EPath::NexusDLL_Old), fallback);

				this->Logger->Warning(
					CH_SELFUPDATER,
					"Couldn't remove \"%s\". Renamed to \"%s\".",
					Index(EPath::NexusDLL_Old).string().c_str(),
					fallback.string().c_str()
				);
			}
		}

		/* Ensure .dll.update path is not claimed. */
		if (std::filesystem::exists(Index(EPath::NexusDLL_Update)))
		{
			try
			{
				std::filesystem::remove(Index(EPath::NexusDLL_Update));
			}
			catch (std::filesystem::filesystem_error fErr)
			{
				std::filesystem::path fallback = Path::GetUnused(Index(EPath::DIR_TEMP) / "d3d11.dll.update");
				std::filesystem::rename(Index(EPath::NexusDLL_Update), fallback);

				this->Logger->Warning(
					CH_SELFUPDATER,
					"Couldn't remove \"%s\". Renamed to \"%s\".",
					Index(EPath::NexusDLL_Update).string().c_str(),
					fallback.string().c_str()
				);
			}
		}
	}

	bool Updater::DownloadUpdate()
	{
		CHttpClient githubclient = CHttpClient(this->Logger, "https://github.com");

		HttpResponse_t ghresult = githubclient.Download(
			Index(EPath::NexusDLL_Update),
			"/RaidcoreGG/Nexus/releases/latest/download/d3d11.dll"
		);

		if (ghresult.Success())
		{
			return true;
		}

		this->Logger->Warning(
			CH_SELFUPDATER,
			"Failed to download Nexus update from GitHub.\n\tStatus: %s\n\tError: %s",
			ghresult.Status().empty() ? "(null)" : ghresult.Status().c_str(),
			ghresult.Error.c_str()
		);

		CHttpClient& raidcoreapi = Runtime::Get().Network().GetHttpClient("https://api.raidcore.gg");

		HttpResponse_t fbresult = raidcoreapi.Download(
			Index(EPath::NexusDLL_Update),
			"/d3d11.dll"
		);

		if (fbresult.Success())
		{
			return true;
		}

		this->Logger->Warning(
			CH_SELFUPDATER,
			"Failed to download Nexus update from fallback. (Raidcore API)\n\tStatus: %s\n\tError: %s",
			fbresult.Status().empty() ? "(null)" : fbresult.Status().c_str(),
			fbresult.Error.c_str()
		);

		return false;
	}

	void Updater::Run()
	{
		Runtime& ctx = Runtime::Get();

		/* If the mutex creation failed, this is a nth game client instance, not the first. */
		if (this->CreatePatchMutex() == false)
		{
			return;
		}

		/* Update check and perform logic below. */
		const Version_t& currentVersion = ctx.GetVersion();

		/* These paths in theory should be protected by the mutex, but we safeguard them anyway. */
		this->CleanupUpdateFiles();

		/* Fetch the version and store it in the private member field. */
		this->GetRemoteVersion();

		if (this->RemoteVersion > currentVersion)
		{
			this->Logger->Info(
				CH_SELFUPDATER,
				"Update available. (Current: %s) (Remote: %s)",
				currentVersion.string().c_str(),
				this->RemoteVersion.string().c_str()
			);

			/* Downloads the update to d3d11.dll.update. */
			this->DownloadUpdate();

			/* Try renaming .dll to .dll.old. */
			try
			{
				std::filesystem::rename(Index(EPath::NexusDLL), Index(EPath::NexusDLL_Old));
			}
			catch (std::filesystem::filesystem_error fErr)
			{
				this->Logger->Warning(
					CH_SELFUPDATER,
					"Nexus update failed: Couldn't move \"%s\" to \"%s\".",
					Index(EPath::NexusDLL).string().c_str(),
					Index(EPath::NexusDLL_Old).string().c_str()
				);
				return;
			}

			/* Try renaming .dll.update to .dll. */
			try
			{
				std::filesystem::rename(Index(EPath::NexusDLL_Update), Index(EPath::NexusDLL));
			}
			catch (std::filesystem::filesystem_error fErr)
			{
				this->Logger->Warning(
					CH_SELFUPDATER,
					"Nexus update failed: Couldn't move \"%s\" to \"%s\".",
					Index(EPath::NexusDLL_Update).string().c_str(),
					Index(EPath::NexusDLL).string().c_str()
				);

				/* Try reverting .dll.old to .dll. */
				try
				{
					std::filesystem::rename(Index(EPath::NexusDLL_Old), Index(EPath::NexusDLL));
				}
				catch (std::filesystem::filesystem_error fErr)
				{
					this->Logger->Warning(
						CH_SELFUPDATER,
						"Nexus update failed: Couldn't move \"%s\" to \"%s\".",
						Index(EPath::NexusDLL_Old).string().c_str(),
						Index(EPath::NexusDLL).string().c_str()
					);
					return;
				}

				return;
			}

			this->Logger->Info(
				CH_SELFUPDATER,
				"Successfully updated Nexus. Restart required to take effect. (Current: %s) (Remote: %s)",
				currentVersion.string().c_str(),
				this->RemoteVersion.string().c_str()
			);
			return;
		}

		if (this->RemoteVersion < currentVersion)
		{
			this->Logger->Info(
				CH_SELFUPDATER,
				"Installed Build of Nexus is more up-to-date than remote. (Current: %s) (Remote: %s)",
				currentVersion.string().c_str(),
				this->RemoteVersion.string().c_str()
			);
			return;
		}

		this->Logger->Info(CH_SELFUPDATER, "Installed Build of Nexus is up-to-date.");
	}
}
