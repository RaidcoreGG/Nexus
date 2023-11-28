#include "Updater.h"

#include <mutex>
#include <map>
#include <set>
#include <thread>
#include <filesystem>
#include <fstream>
#include <regex>

#include "Consts.h"
#include "Shared.h"
#include "Paths.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "httpslib.h"

const char* BASE_URL = "https://api.raidcore.gg";
const char* NEXUS_VERSION_ENDPOINT = "/nexusversion.json";
const char* NEXUS_DLL_ENDPOINT = "/d3d11.dll";

namespace Updater
{
	void SelfUpdate()
	{
		if (std::filesystem::exists(Path::F_OLD_DLL))
		{
			std::filesystem::remove(Path::F_OLD_DLL);
		}

		httplib::Client client(BASE_URL);
		client.enable_server_certificate_verification(false);
		auto result = client.Get(NEXUS_VERSION_ENDPOINT);

		if (!result)
		{
			LogWarning(CH_UPDATER, "Error fetching %s%s", BASE_URL, NEXUS_VERSION_ENDPOINT);
		}
		else
		{
			if (result->status != 200) // not HTTP_OK
			{
				LogWarning(CH_UPDATER, "Status %d when fetching %s%s", result->status, BASE_URL, NEXUS_VERSION_ENDPOINT);
			}
			else
			{
				//LogDebug(CH_UPDATER, "Body: %s", result->body.c_str());
				json resVersion = json::parse(result->body);
				if (resVersion.is_null())
				{
					LogWarning(CH_UPDATER, "Error parsing API response.");
				}
				else
				{
					bool anyNull = false;
					AddonVersion remoteVersion{};
					if (!resVersion["Major"].is_null()) { remoteVersion.Major = resVersion["Major"].get<signed short>(); } else { anyNull = true; }
					if (!resVersion["Minor"].is_null()) { remoteVersion.Minor = resVersion["Minor"].get<signed short>(); } else { anyNull = true; }
					if (!resVersion["Build"].is_null()) { remoteVersion.Build = resVersion["Build"].get<signed short>(); } else { anyNull = true; }
					if (!resVersion["Revision"].is_null()) { remoteVersion.Revision = resVersion["Revision"].get<signed short>(); } else { anyNull = true; }

					if (anyNull)
					{
						LogWarning(CH_UPDATER, "One or more fields in the API response were null.");
					}
					else
					{
						if (remoteVersion > *Version)
						{
							LogInfo(CH_UPDATER, "Outdated: API replied with Version %s but installed is Version %s", remoteVersion.ToString().c_str(), Version->ToString().c_str());

							size_t bytesWritten = 0;

							std::ofstream file(Path::F_UPDATE_DLL, std::ofstream::binary);
							auto downloadResult = client.Get(NEXUS_DLL_ENDPOINT, [&](const char* data, size_t data_length) {
								file.write(data, data_length);
								bytesWritten += data_length;
								return true;
							});
							file.close();

							if (!downloadResult || downloadResult->status != 200 || bytesWritten == 0)
							{
								LogWarning(CH_UPDATER, "Error fetching %s%s", BASE_URL, NEXUS_DLL_ENDPOINT);
							}
							else
							{
								std::filesystem::rename(Path::F_HOST_DLL, Path::F_OLD_DLL);
								std::filesystem::rename(Path::F_UPDATE_DLL, Path::F_HOST_DLL);

								LogInfo(CH_UPDATER, "Successfully updated Nexus. Restart required to take effect.");
							}
						}
						else if (remoteVersion < *Version)
						{
							LogInfo(CH_UPDATER, "Installed Build of Nexus is more up-to-date than remote. (Installed: %s) (Remote: %s)", Version->ToString().c_str(), remoteVersion.ToString().c_str());
						}
						else
						{
							LogInfo(CH_UPDATER, "Installed Build of Nexus is up-to-date.");
						}
					}
				}
			}
		}

		if (std::filesystem::exists(Path::F_UPDATE_DLL))
		{
			std::filesystem::remove(Path::F_UPDATE_DLL);
		}
	}

	bool CheckForUpdate(std::filesystem::path aPath, AddonDefinition* aDefinitions)
	{
		assert(aDefinitions && "AddonDefinition* aDefinitions == nullptr");

		bool wasUpdated = false;

		std::string baseUrl = "";
		std::string endpoint = "";

		size_t idx = 0;

		/* setup baseUrl and endpoint */
		switch (aDefinitions->Provider)
		{
		case EUpdateProvider::None: return false;

		case EUpdateProvider::Raidcore:
			baseUrl = "https://api.raidcore.gg";
			endpoint = "/addons/" + std::to_string(aDefinitions->Signature);

			break;

		case EUpdateProvider::GitHub:
			baseUrl = "https://api.github.com";
			if (aDefinitions->UpdateLink == nullptr)
			{
				LogWarning(CH_UPDATER, "Addon %s declares EUpdateProvider::GitHub but has no UpdateLink set.", aDefinitions->Name);
				return false;
			}
			endpoint = aDefinitions->UpdateLink;

			idx = endpoint.find("github.com");
			if (idx != std::string::npos)
			{
				// substr because user could've provided https:// or even www prefix as well
				endpoint = "/repos" + endpoint.substr(idx + 10) + "/releases/latest"; // 10 is length of "github.com"
			}

			break;

		case EUpdateProvider::Direct:
			if (aDefinitions->UpdateLink == nullptr)
			{
				LogWarning(CH_UPDATER, "Addon %s declares EUpdateProvider::Direct but has no UpdateLink set.", aDefinitions->Name);
				return false;
			}
			
			size_t httpIdx = endpoint.find("http://");
			size_t httpsIdx = endpoint.find("https://");

			size_t off = 0;
			if (httpIdx != std::string::npos)
			{
				off = httpIdx;
			}
			if (httpsIdx != std::string::npos)
			{
				off = httpsIdx;
			}

			idx = endpoint.find("/", off);
			if (idx == std::string::npos)
			{
				LogWarning("Abort update check, malformed URL (%s) on %s. (EUpdateProvider::Direct)", aDefinitions->UpdateLink, aDefinitions->Name);
				return false;
			}

			baseUrl = endpoint.substr(0, idx);
			endpoint = endpoint.substr(idx);

			break;
		}

		/* setup paths */
		std::filesystem::path pathOld = aPath.string() + ".old";
		std::filesystem::path pathUpdate = aPath.string() + ".update";

		/* cleanup old files */
		if (std::filesystem::exists(pathOld)) { std::filesystem::remove(pathOld); }
		if (std::filesystem::exists(pathUpdate)) { std::filesystem::remove(pathUpdate); }

		/* get request */
		httplib::Client client(baseUrl);
		client.enable_server_certificate_verification(false);
		auto result = client.Get(endpoint);

		if (EUpdateProvider::Raidcore == aDefinitions->Provider)
		{
			if (!result)
			{
				LogWarning(CH_UPDATER, "Error fetching %s%s", baseUrl.c_str(), endpoint.c_str());
			}
			else
			{
				if (result->status != 200) // not HTTP_OK
				{
					LogWarning(CH_UPDATER, "Status %d when fetching %s%s", result->status, baseUrl.c_str(), endpoint.c_str());
				}
				else
				{
					json resVersion = json::parse(result->body);
					if (resVersion.is_null())
					{
						LogWarning(CH_UPDATER, "Error parsing API response.");
					}
					else
					{
						bool anyNull = false;
						AddonVersion remoteVersion{};
						if (!resVersion["Major"].is_null()) { remoteVersion.Major = resVersion["Major"].get<signed short>(); }
						else { anyNull = true; }
						if (!resVersion["Minor"].is_null()) { remoteVersion.Minor = resVersion["Minor"].get<signed short>(); }
						else { anyNull = true; }
						if (!resVersion["Build"].is_null()) { remoteVersion.Build = resVersion["Build"].get<signed short>(); }
						else { anyNull = true; }
						if (!resVersion["Revision"].is_null()) { remoteVersion.Revision = resVersion["Revision"].get<signed short>(); }
						else { anyNull = true; }

						if (anyNull)
						{
							LogWarning(CH_UPDATER, "One or more fields in the API response were null.");
						}
						else
						{
							if (remoteVersion > aDefinitions->Version)
							{
								LogInfo(CH_UPDATER, "%s is outdated: API replied with Version %s but installed is Version %s", aDefinitions->Name, remoteVersion.ToString().c_str(), aDefinitions->Version.ToString().c_str());

								std::string endpointDownload = endpoint + "/download"; // e.g. api.raidcore.gg/addons/17/download

								size_t bytesWritten = 0;
								std::ofstream file(pathUpdate, std::ofstream::binary);
								auto downloadResult = client.Get(endpointDownload, [&](const char* data, size_t data_length) {
									file.write(data, data_length);
									bytesWritten += data_length;
									return true;
									});
								file.close();

								if (!downloadResult || downloadResult->status != 200 || bytesWritten == 0)
								{
									LogWarning(CH_UPDATER, "Error fetching %s%s", baseUrl.c_str(), endpointDownload.c_str());
								}
								else
								{
									std::filesystem::rename(aPath, pathOld);
									std::filesystem::rename(pathUpdate, aPath);

									LogInfo(CH_UPDATER, "Successfully updated %s.", aDefinitions->Name);
									wasUpdated = true;
								}
							}
						}
					}
				}
			}
		}
		else if (EUpdateProvider::GitHub == aDefinitions->Provider)
		{
			if (!result)
			{
				LogWarning(CH_UPDATER, "Error fetching %s%s", baseUrl.c_str(), endpoint.c_str());
			}
			else
			{
				if (result->status != 200) // not HTTP_OK
				{
					LogWarning(CH_UPDATER, "Status %d when fetching %s%s", result->status, baseUrl.c_str(), endpoint.c_str());
				}
				else
				{
					json response = json::parse(result->body);
					if (response.is_null())
					{
						LogWarning(CH_UPDATER, "Error parsing API response.");
					}
					else
					{
						AddonVersion remoteVersion{};

						if (response["tag_name"].is_null())
						{
							LogWarning("No tag_name set on %s%s", baseUrl.c_str(), endpoint.c_str());
							return false;
						}

						std::string tagName = response["tag_name"].get<std::string>();

						if (!std::regex_match(tagName, std::regex("v?\\d+[.]\\d+[.]\\d+[.]\\d+")))
						{
							LogWarning("tag_name on %s%s does not match convention e.g. \"1.0.0.1\" or \"v1.0.0.1\". Cannot check against version.", baseUrl.c_str(), endpoint.c_str());
							return false;
						}

						if (tagName._Starts_with("v"))
						{
							tagName = tagName.substr(1);
						}

						std::vector<signed short> versionBits;

						size_t pos = 0;
						std::string token;
						while ((pos = tagName.find(".")) != std::string::npos) {
							token = tagName.substr(0, pos);
							versionBits.push_back(static_cast<unsigned short>(std::stoi(token)));
							tagName.erase(0, pos + 1);
						}
						versionBits.push_back(static_cast<unsigned short>(std::stoi(tagName)));

						remoteVersion.Major = versionBits[0];
						remoteVersion.Minor = versionBits[1];
						remoteVersion.Build = versionBits[2];
						remoteVersion.Revision = versionBits[3];

						if (remoteVersion > aDefinitions->Version)
						{
							LogInfo(CH_UPDATER, "%s is outdated: API replied with Version %s but installed is Version %s", aDefinitions->Name, remoteVersion.ToString().c_str(), aDefinitions->Version.ToString().c_str());

							std::string endpointDownload; // e.g. github.com/RaidcoreGG/GW2-CommandersToolkit/releases/download/20220918-135925/squadmanager.dll

							if (response["assets"].is_null())
							{
								LogWarning("Release has no assets. Cannot check against version. (%s%s)", baseUrl.c_str(), endpoint.c_str());
							}

							for (auto& asset : response["assets"])
							{
								std::string assetName = asset["name"].get<std::string>();
								
								if (assetName.size() < 4) {
									continue;
								}

								if (std::string_view(assetName).substr(assetName.size() - 4) == ".dll") {
									endpointDownload = asset["browser_download_url"].get<std::string>();
								}
							}

							std::string downloadBaseUrl;

							size_t downloadHttpIdx = endpointDownload.find("http://");
							size_t downloadHttpsIdx = endpointDownload.find("https://");

							size_t downloadOff = 0;
							if (downloadHttpIdx != std::string::npos)
							{
								downloadOff = downloadHttpIdx + 7; // 7 is length of "http://"
							}
							if (downloadHttpsIdx != std::string::npos)
							{
								downloadOff = downloadHttpsIdx + 8; // 8 is length of "https://"
							}
							
							idx = endpointDownload.find("/", downloadOff);
							if (idx == std::string::npos)
							{
								LogWarning("Abort update download, malformed URL (%s) on %s. (EUpdateProvider::GitHub)", endpointDownload.c_str(), aDefinitions->Name);
								return false;
							}

							downloadBaseUrl = endpointDownload.substr(0, idx);
							endpointDownload = endpointDownload.substr(idx);

							httplib::Client downloadClient(downloadBaseUrl);
							downloadClient.enable_server_certificate_verification(false);
							downloadClient.set_follow_location(true);

							size_t bytesWritten = 0;
							std::ofstream file(pathUpdate, std::ofstream::binary);
							auto downloadResult = downloadClient.Get(endpointDownload, [&](const char* data, size_t data_length) {
								file.write(data, data_length);
								bytesWritten += data_length;
								return true;
								});
							file.close();

							if (!downloadResult || downloadResult->status != 200 || bytesWritten == 0)
							{
								LogWarning(CH_UPDATER, "Error fetching %s%s", downloadBaseUrl.c_str(), endpointDownload.c_str());
							}
							else
							{
								std::filesystem::rename(aPath, pathOld);
								std::filesystem::rename(pathUpdate, aPath);

								LogInfo(CH_UPDATER, "Successfully updated %s.", aDefinitions->Name);
								wasUpdated = true;
							}
						}
					}
				}
			}
		}
		else if (EUpdateProvider::Direct == aDefinitions->Provider)
		{
			LogDebug(CH_UPDATER, "EUpdateProvider::Direct not implemented.");
			return false; // not implemented: need a way to check for update. md5? extra file? filename?
		}

		return wasUpdated;
	}
}