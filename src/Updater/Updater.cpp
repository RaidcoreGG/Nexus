#include "Updater.h"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "../httplib/httplib.h"

const char* BASE_URL = "https://api.raidcore.gg";
const char* NEXUS_VERSION_ENDPOINT = "/nexusversion.json";
const char* NEXUS_DLL_ENDPOINT = "/nexus.dll";

namespace Updater
{
	void Initialize()
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
						else
						{
							LogInfo(CH_UPDATER, "Installed Build of Nexus is up-to-date.");
						}
					}
				}
			}
		}
	}
}