#include "BuildInfoService.h"

namespace Raidcore::Nexus::GW2
{
	BuildInfoService::BuildInfoService(Network::CHttpClient& aArenaNetAssetCDN, Core::LogApi& aLogger)
		: _ArenaNetAssetCDN(aArenaNetAssetCDN)
		, _Logger(aLogger)
	{}

	uint32_t BuildInfoService::Build()
	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		if (this->_Build > 0)
		{
			return this->_Build;
		}

		Network::HttpResponse_t result = this->_ArenaNetAssetCDN.Get("/latest64/101");

		if (!result.Success())
		{
			this->_Logger.Warning(
				"GW2 BuildInfo",
				"Failed to fetch game build.\n\tStatus: %s\n\tError: %s",
				result.Status(),
				result.Error.c_str()
			);
			return this->_Build;
		}

		try
		{
			this->_Build = std::stoi(result.Content);
			this->_Logger.Debug("GW2 BuildInfo", "%d \n%s", this->_Build, result.Content.c_str());
		}
		catch (...)
		{
			this->_Logger.Warning("GW2 BuildInfo", "Unknown error processing \"%s\".");
		}

		return this->_Build;
	}
}
