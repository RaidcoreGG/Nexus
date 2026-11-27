///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  BuildInfo.cpp
/// Description  :  Retrieves the game build.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "BuildInfo.h"

#include <mutex>

#include "Host/GW2/Context.h"
#include "Engine/Networking/WebRequests/WreClient.h"

namespace Raidcore::GW2
{
	uint32_t GW2::GetGameBuild()
	{
		static uint32_t s_GameBuild = 0;
		static std::mutex s_Mutex;

		const std::lock_guard<std::mutex> lock(s_Mutex);

		if (s_GameBuild > 0)
		{
			return s_GameBuild;
		}

		Engine::Networking::CHttpClient* client = GW2::CContext::GetContext()->GetHttpClient("http://assetcdn.101.arenanetworks.com", /*disablecache=*/ true);

		Engine::Networking::HttpResponse_t result = client->Get("/latest64/101");

		if (!result.Success())
		{
			Core::LogWarning(
				"GW2 BuildInfo",
				"Failed to fetch game build.\n\tStatus: %s\n\tError: %s",
				result.Status().c_str(),
				result.Error.c_str()
			);
			return s_GameBuild;
		}

		try
		{
			s_GameBuild = std::stoi(result.Content);
			Core::LogDebug("GW2 BuildInfo", "%d \n%s", s_GameBuild, result.Content.c_str());
		}
		catch (...)
		{
			Core::LogWarning("GW2 BuildInfo", "Unknown error processing \"%s\".", result.Content.c_str());
		}

		return s_GameBuild;
	}
}
