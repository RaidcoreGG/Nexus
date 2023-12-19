#include "Updater.h"

#include <mutex>
#include <map>
#include <set>
#include <thread>
#include <filesystem>
#include <fstream>
#include <regex>
#include <fstream>

#include <exception>
#include <typeinfo>
#include <stdexcept>

#include "core.h"
#include "Consts.h"
#include "Shared.h"
#include "Paths.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "httpslib.h"
#include "openssl/md5.h"

const char* BASE_URL = "https://api.raidcore.gg";
const char* NEXUS_VERSION_ENDPOINT = "/nexusversion.json";
const char* NEXUS_DLL_ENDPOINT = "/d3d11.dll";

namespace Updater
{
	std::string GetBaseURL(std::string aUrl)
	{
		size_t httpIdx = aUrl.find("http://");
		size_t httpsIdx = aUrl.find("https://");

		size_t off = 0;
		if (httpIdx != std::string::npos)
		{
			off = httpIdx + 7; // 7 is length of "http://"
		}
		if (httpsIdx != std::string::npos)
		{
			off = httpsIdx + 8; // 8 is length of "https://"
		}

		size_t idx = aUrl.find('/', off);
		if (idx == std::string::npos)
		{
			return aUrl;
		}

		return aUrl.substr(0, idx);
	}
	std::string GetEndpoint(std::string aUrl)
	{
		size_t httpIdx = aUrl.find("http://");
		size_t httpsIdx = aUrl.find("https://");

		size_t off = 0;
		if (httpIdx != std::string::npos)
		{
			off = httpIdx + 7; // 7 is length of "http://"
		}
		if (httpsIdx != std::string::npos)
		{
			off = httpsIdx + 8; // 8 is length of "https://"
		}

		size_t idx = aUrl.find('/', off);
		if (idx == std::string::npos)
		{
			return aUrl;
		}

		return aUrl.substr(idx);
	}

	/* FIXME */
	/* Api requests refactor into API Handler. */

	void SelfUpdate()
	{
		if (std::filesystem::exists(Path::F_OLD_DLL))
		{
			std::filesystem::remove(Path::F_OLD_DLL);
		}

		httplib::Client client(BASE_URL);
		auto result = client.Get(NEXUS_VERSION_ENDPOINT);

		if (!result)
		{
			LogWarning(CH_UPDATER, "Error fetching %s%s", BASE_URL, NEXUS_VERSION_ENDPOINT);
			return;
		}

		if (result->status != 200) // not HTTP_OK
		{
			LogWarning(CH_UPDATER, "Status %d when fetching %s%s", result->status, BASE_URL, NEXUS_VERSION_ENDPOINT);
			return;
		}

		//LogDebug(CH_UPDATER, "Body: %s", result->body.c_str());
		json resVersion = json::parse(result->body);
		if (resVersion.is_null())
		{
			LogWarning(CH_UPDATER, "Error parsing API response.");
			return;
		}

		bool anyNull = false;
		AddonVersion remoteVersion{};
		if (!resVersion["Major"].is_null()) { resVersion["Major"].get_to(remoteVersion.Major); }
		else { anyNull = true; }
		if (!resVersion["Minor"].is_null()) { resVersion["Minor"].get_to(remoteVersion.Minor); }
		else { anyNull = true; }
		if (!resVersion["Build"].is_null()) { resVersion["Build"].get_to(remoteVersion.Build); }
		else { anyNull = true; }
		if (!resVersion["Revision"].is_null()) { resVersion["Revision"].get_to(remoteVersion.Revision); }
		else { anyNull = true; }

		if (anyNull)
		{
			LogWarning(CH_UPDATER, "One or more fields in the API response were null.");
			return;
		}

		if (remoteVersion > Version)
		{
			LogInfo(CH_UPDATER, "Outdated: API replied with Version %s but installed is Version %s", remoteVersion.ToString().c_str(), Version.ToString().c_str());

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
				return;
			}

			std::filesystem::rename(Path::F_HOST_DLL, Path::F_OLD_DLL);
			std::filesystem::rename(Path::F_UPDATE_DLL, Path::F_HOST_DLL);

			LogInfo(CH_UPDATER, "Successfully updated Nexus. Restart required to take effect.");
		}
		else if (remoteVersion < Version)
		{
			LogInfo(CH_UPDATER, "Installed Build of Nexus is more up-to-date than remote. (Installed: %s) (Remote: %s)", Version.ToString().c_str(), remoteVersion.ToString().c_str());
		}
		else
		{
			LogInfo(CH_UPDATER, "Installed Build of Nexus is up-to-date.");
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

		std::string baseUrl;
		std::string endpoint;

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

			endpoint = "/repos" + GetEndpoint(aDefinitions->UpdateLink) + "/releases/latest";

			break;

		case EUpdateProvider::Direct:
			if (aDefinitions->UpdateLink == nullptr)
			{
				LogWarning(CH_UPDATER, "Addon %s declares EUpdateProvider::Direct but has no UpdateLink set.", aDefinitions->Name);
				return false;
			}

			baseUrl = GetBaseURL(aDefinitions->UpdateLink);
			endpoint = GetEndpoint(aDefinitions->UpdateLink);

			break;
		}

		/* setup paths */
		std::filesystem::path pathOld = aPath.string() + ".old";
		std::filesystem::path pathUpdate = aPath.string() + ".update";

		/* cleanup old files */
		if (std::filesystem::exists(pathOld)) { std::filesystem::remove(pathOld); }
		if (std::filesystem::exists(pathUpdate)) { std::filesystem::remove(pathUpdate); }

		/* prepare client request */
		httplib::Client client(baseUrl);

		if (EUpdateProvider::Raidcore == aDefinitions->Provider)
		{
			auto result = client.Get(endpoint);

			if (!result)
			{
				LogWarning(CH_UPDATER, "Error fetching %s%s", baseUrl.c_str(), endpoint.c_str());
				return false;
			}
			
			if (result->status != 200) // not HTTP_OK
			{
				LogWarning(CH_UPDATER, "Status %d when fetching %s%s", result->status, baseUrl.c_str(), endpoint.c_str());
				return false;
			}

			json resVersion;
			try
			{
				resVersion = json::parse(result->body);
			}
			catch (json::parse_error& ex)
			{
				LogWarning(CH_UPDATER, "Response from %s%s could not be parsed. Error: %s", baseUrl.c_str(), endpoint.c_str(), ex.what());
				return false;
			}

			if (resVersion.is_null())
			{
				LogWarning(CH_UPDATER, "Error parsing API response.");
				return false;
			}

			bool anyNull = false;
			AddonVersion remoteVersion{};
			if (!resVersion["Major"].is_null()) { resVersion["Major"].get_to(remoteVersion.Major); }
			else { anyNull = true; }
			if (!resVersion["Minor"].is_null()) { resVersion["Minor"].get_to(remoteVersion.Minor); }
			else { anyNull = true; }
			if (!resVersion["Build"].is_null()) { resVersion["Build"].get_to(remoteVersion.Build); }
			else { anyNull = true; }
			if (!resVersion["Revision"].is_null()) { resVersion["Revision"].get_to(remoteVersion.Revision); }
			else { anyNull = true; }

			if (anyNull)
			{
				LogWarning(CH_UPDATER, "One or more fields in the API response were null.");
				return false;
			}

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
					return false;
				}

				LogInfo(CH_UPDATER, "Successfully updated %s.", aDefinitions->Name);
				wasUpdated = true;
			}
		}
		else if (EUpdateProvider::GitHub == aDefinitions->Provider)
		{
			auto result = client.Get(endpoint);

			if (!result)
			{
				LogWarning(CH_UPDATER, "Error fetching %s%s", baseUrl.c_str(), endpoint.c_str());
				return false;
			}

			if (result->status != 200) // not HTTP_OK
			{
				LogWarning(CH_UPDATER, "Status %d when fetching %s%s", result->status, baseUrl.c_str(), endpoint.c_str());
				return false;
			}

			json response;
			try
			{
				response = json::parse(result->body);
			}
			catch (json::parse_error& ex)
			{
				LogWarning(CH_UPDATER, "Response from %s%s could not be parsed. Error: %s", baseUrl.c_str(), endpoint.c_str(), ex.what());
				return false;
			}

			if (response.is_null())
			{
				LogWarning(CH_UPDATER, "Error parsing API response.");
				return false;
			}

			if (response["tag_name"].is_null())
			{
				LogWarning(CH_UPDATER, "No tag_name set on %s%s", baseUrl.c_str(), endpoint.c_str());
				return false;
			}

			std::string tagName = response["tag_name"].get<std::string>();

			if (!std::regex_match(tagName, std::regex("v?\\d+[.]\\d+[.]\\d+[.]\\d+")))
			{
				LogWarning(CH_UPDATER, "tag_name on %s%s does not match convention e.g. \"1.0.0.1\" or \"v1.0.0.1\". Cannot check against version.", baseUrl.c_str(), endpoint.c_str());
				return false;
			}

			if (tagName._Starts_with("v"))
			{
				tagName = tagName.substr(1);
			}

			AddonVersion remoteVersion{};

			size_t pos = 0;
			int i = 0;
			while ((pos = tagName.find(".")) != std::string::npos)
			{
				switch (i)
				{
				case 0: remoteVersion.Major = static_cast<unsigned short>(std::stoi(tagName.substr(0, pos))); break;
				case 1: remoteVersion.Minor = static_cast<unsigned short>(std::stoi(tagName.substr(0, pos))); break;
				case 2: remoteVersion.Build = static_cast<unsigned short>(std::stoi(tagName.substr(0, pos))); break;
				}
				i++;
				tagName.erase(0, pos + 1);
			}
			remoteVersion.Revision = static_cast<unsigned short>(std::stoi(tagName));

			if (remoteVersion > aDefinitions->Version)
			{
				LogInfo(CH_UPDATER, "%s is outdated: API replied with Version %s but installed is Version %s", aDefinitions->Name, remoteVersion.ToString().c_str(), aDefinitions->Version.ToString().c_str());

				std::string endpointDownload; // e.g. github.com/RaidcoreGG/GW2-CommandersToolkit/releases/download/20220918-135925/squadmanager.dll

				if (response["assets"].is_null())
				{
					LogWarning(CH_UPDATER, "Release has no assets. Cannot check against version. (%s%s)", baseUrl.c_str(), endpoint.c_str());
					return false;
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
					}
				}

				std::string downloadBaseUrl = GetBaseURL(endpointDownload);
				endpointDownload = GetEndpoint(endpointDownload);

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
					return false;
				}

				LogInfo(CH_UPDATER, "Successfully updated %s.", aDefinitions->Name);
				wasUpdated = true;
			}
		}
		else if (EUpdateProvider::Direct == aDefinitions->Provider)
		{
			client.enable_server_certificate_verification(false);

			std::string endpointMD5 = endpoint + ".md5sum";

			std::ifstream fileCurrent(aPath, std::ios::binary);
			fileCurrent.seekg(0, std::ios::end);
			size_t length = fileCurrent.tellg();
			fileCurrent.seekg(0, std::ios::beg);
			char* buffer = new char[length];
			fileCurrent.read(buffer, length);

			std::vector<unsigned char> md5current = MD5((const unsigned char*)buffer, length);
			std::vector<unsigned char> md5remote;

			client.Get(endpointMD5, [&](const char* data, size_t data_length) {
				for (size_t i = 0; i < data_length; i += 2)
				{
					if (md5current.size() == md5remote.size())
					{
						break; // more bytes aren't needed
					}

					std::string str{};
					str += data[i];
					str += data[i + 1];

					unsigned char byte = (unsigned char)strtol(str.c_str(), NULL, 16);

					md5remote.push_back(byte);
				}
				return true;
			});

			delete[] buffer;

			if (md5current == md5remote)
			{
				return false;
			}

			size_t bytesWritten = 0;
			std::ofstream fileUpdate(pathUpdate, std::ofstream::binary);
			auto downloadResult = client.Get(endpoint, [&](const char* data, size_t data_length) {
				fileUpdate.write(data, data_length);
				bytesWritten += data_length;
				return true;
				});
			fileUpdate.close();

			if (!downloadResult || downloadResult->status != 200 || bytesWritten == 0)
			{
				LogWarning(CH_UPDATER, "Error fetching %s%s", baseUrl.c_str(), endpoint.c_str());
				return false;
			}

			wasUpdated = true;
		}

		return wasUpdated;
	}
}