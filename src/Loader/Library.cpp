///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Library.cpp
/// Description  :  Handles installation of new addons and fetching them from the Raidcore API.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Library.h"

#include "Loader.h"
#include "ArcDPS.h"

#include "core.h"
#include "Consts.h"
#include "Shared.h"
#include "Paths.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

std::string extDll = ".dll";

namespace Loader
{
	namespace Library
	{
		std::mutex					Mutex;
		std::vector<LibraryAddon*>	Addons;

		void Fetch()
		{
			json response = RaidcoreAPI->Get("/addonlibrary");

			if (!response.is_null())
			{
				const std::lock_guard<std::mutex> lock(Mutex);
				Addons.clear();

				for (const auto& addon : response)
				{
					LibraryAddon* newAddon = new LibraryAddon{};
					newAddon->Signature = addon["id"];
					newAddon->Name = addon["name"];
					newAddon->Description = addon["description"];
					newAddon->Provider = GetProvider(addon["download"]);
					newAddon->DownloadURL = addon["download"];
					if (addon.contains("tos_compliance") && !addon["tos_compliance"].is_null())
					{
						newAddon->ToSComplianceNotice = addon["tos_compliance"];
					}

					Addons.push_back(newAddon);
				}

				std::sort(Addons.begin(), Addons.end(), [](LibraryAddon* a, LibraryAddon* b) {
					return a->Name < b->Name;
					});
			}
			else
			{
				LogWarning(CH_LOADER, "Error parsing API response for /addonlibrary.");
			}
		}

		void InstallAddon(LibraryAddon* aAddon, bool aIsArcPlugin)
		{
			aAddon->IsInstalling = true;

			std::filesystem::path installPath;

			/* this is all modified duplicate code from update */
			std::string baseUrl;
			std::string endpoint;

			// override provider if none set, but a Raidcore ID is used
			if (aAddon->Provider == EUpdateProvider::None && aAddon->Signature > 0)
			{
				aAddon->Provider = EUpdateProvider::Raidcore;
			}

			/* setup baseUrl and endpoint */
			switch (aAddon->Provider)
			{
			case EUpdateProvider::None: return;

			case EUpdateProvider::Raidcore:
				baseUrl = API_RAIDCORE;
				endpoint = "/addons/" + std::to_string(aAddon->Signature);

				break;

			case EUpdateProvider::GitHub:
				baseUrl = API_GITHUB;
				if (aAddon->DownloadURL.empty())
				{
					LogWarning(CH_LOADER, "Addon %s declares EUpdateProvider::GitHub but has no UpdateLink set.", aAddon->Name);
					return;
				}

				endpoint = "/repos" + GetEndpoint(aAddon->DownloadURL) + "/releases"; // "/releases/latest"; // fuck you Sognus

				break;

			case EUpdateProvider::Direct:
				if (aAddon->DownloadURL.empty())
				{
					LogWarning(CH_LOADER, "Addon %s declares EUpdateProvider::Direct but has no UpdateLink set.", aAddon->Name);
					return;
				}

				baseUrl = GetBaseURL(aAddon->DownloadURL);
				endpoint = GetEndpoint(aAddon->DownloadURL);

				if (baseUrl.empty() || endpoint.empty())
				{
					return;
				}

				break;
			}

			if (EUpdateProvider::Raidcore == aAddon->Provider)
			{
				LogWarning(CH_LOADER, "Downloading via Raidcore is not implemented yet, due to user-friendly names requiring an API request. If you see this tell the developers about it! Thank you!");
				return;
				//RaidcoreAPI->Download(addonPath, endpoint + "/download"); // e.g. api.raidcore.gg/addons/17/download
			}
			else if (EUpdateProvider::GitHub == aAddon->Provider)
			{
				json response = GitHubAPI->Get(endpoint);

				if (response.is_null())
				{
					LogWarning(CH_LOADER, "Error parsing API response.");
					return;
				}

				response = response[0]; // filthy hack to get "latest"

				if (response["tag_name"].is_null())
				{
					LogWarning(CH_LOADER, "No tag_name set on %s%s", baseUrl.c_str(), endpoint.c_str());
					return;
				}

				std::string endpointDownload; // e.g. github.com/RaidcoreGG/GW2-CommandersToolkit/releases/download/20220918-135925/squadmanager.dll

				if (response["assets"].is_null())
				{
					LogWarning(CH_LOADER, "Release has no assets. Cannot check against version. (%s%s)", baseUrl.c_str(), endpoint.c_str());
					return;
				}

				for (auto& asset : response["assets"])
				{
					std::string assetName = asset["name"].get<std::string>();

					if (assetName.size() < 4)
					{
						continue;
					}

					if (std::string_view(assetName).substr(assetName.size() - 4) == ".dll")
					{
						asset["browser_download_url"].get_to(endpointDownload);
						break;
					}
				}

				std::string downloadBaseUrl = GetBaseURL(endpointDownload);
				endpointDownload = GetEndpoint(endpointDownload);

				httplib::Client downloadClient(downloadBaseUrl);
				downloadClient.enable_server_certificate_verification(false);
				downloadClient.set_follow_location(true);

				size_t lastSlashPos = endpointDownload.find_last_of('/');
				std::string filename = endpointDownload.substr(lastSlashPos + 1);
				size_t dotDllPos = filename.find(extDll);
				filename = filename.substr(0, filename.length() - extDll.length());

				std::filesystem::path probe = Path::D_GW2_ADDONS / (filename + extDll);

				int i = 0;
				while (std::filesystem::exists(probe))
				{
					probe = Path::D_GW2_ADDONS / (filename + "_" + std::to_string(i) + extDll);
					i++;
				}

				installPath = probe;

				size_t bytesWritten = 0;
				std::ofstream file(probe, std::ofstream::binary);
				auto downloadResult = downloadClient.Get(endpointDownload, [&](const char* data, size_t data_length) {
					file.write(data, data_length);
					bytesWritten += data_length;
					return true;
					});
				file.close();

				if (!downloadResult || downloadResult->status != 200 || bytesWritten == 0)
				{
					LogWarning(CH_LOADER, "Error fetching %s%s", downloadBaseUrl.c_str(), endpointDownload.c_str());
					return;
				}
			}
			else if (EUpdateProvider::Direct == aAddon->Provider)
			{
				/* prepare client request */
				httplib::Client client(baseUrl);
				client.enable_server_certificate_verification(false);
				client.set_follow_location(true);

				size_t lastSlashPos = endpoint.find_last_of('/');
				std::string filename = endpoint.substr(lastSlashPos + 1);
				size_t dotDllPos = filename.find(extDll);
				if (dotDllPos != std::string::npos)
				{
					filename = filename.substr(0, filename.length() - extDll.length());
				}
				else
				{
					filename = Normalize(aAddon->Name);
				}

				std::filesystem::path probe = Path::D_GW2_ADDONS / (filename + extDll);

				int i = 0;
				while (std::filesystem::exists(probe))
				{
					probe = Path::D_GW2_ADDONS / (filename + "_" + std::to_string(i) + extDll);
					i++;
				}

				installPath = probe;

				size_t bytesWritten = 0;
				std::ofstream fileUpdate(probe, std::ofstream::binary);
				auto downloadResult = client.Get(endpoint, [&](const char* data, size_t data_length) {
					fileUpdate.write(data, data_length);
					bytesWritten += data_length;
					return true;
					});
				fileUpdate.close();

				if (!downloadResult || downloadResult->status != 200 || bytesWritten == 0)
				{
					LogWarning(CH_LOADER, "Error fetching %s%s", baseUrl.c_str(), endpoint.c_str());
					return;
				}
			}

			LogInfo(CH_LOADER, "Successfully installed %s.", aAddon->Name.c_str());
			if (aIsArcPlugin)
			{
				ArcDPS::AddToAtlasBySig(aAddon->Signature);
			}
			else
			{
				Loader::QueueAddon(ELoaderAction::Reload, installPath);
			}
		}
	}
}
