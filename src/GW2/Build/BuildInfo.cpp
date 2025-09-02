///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  BuildInfo.cpp
/// Description  :  Retrieves the game build.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "BuildInfo.h"

#include <mutex>

#include "Core/Context.h"
#include "Engine/Networking/WebRequests/WreClient.h"

uint32_t GW2::GetGameBuild()
{
	static uint32_t s_GameBuild = 0;
	static CLogApi* s_Logger = CContext::GetContext()->GetLogger();
	static std::mutex s_Mutex;

	const std::lock_guard<std::mutex> lock(s_Mutex);

	if (s_GameBuild > 0)
	{
		return s_GameBuild;
	}

	CHttpClient client = CHttpClient(s_Logger, "http://assetcdn.101.arenanetworks.com");

	HttpResponse_t result = client.Get("/latest64/101");

	if (!result.Success())
	{
		s_Logger->Warning(
			"GW2 BuildInfo",
			"Failed to fetch game build.\n\tStatus: %s\n\tError: %s",
			result.Status(),
			result.Error.c_str()
		);
		return s_GameBuild;
	}

	try
	{
		s_GameBuild = std::stoi(result.Content);
		s_Logger->Debug("GW2 BuildInfo", "%d \n%s", s_GameBuild, result.Content.c_str());
	}
	catch (...)
	{
		s_Logger->Warning("GW2 BuildInfo", "Unknown error processing \"%s\".");
	}

	return s_GameBuild;
}
