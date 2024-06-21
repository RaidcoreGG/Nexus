///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Paths.cpp
/// Description  :  Contains a variety of utility for the filesystem and paths.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include <Windows.h>
#include <string>

#include "Paths.h"

namespace Path
{
	std::filesystem::path GetModule(HMODULE aModule)
	{
		char buff[MAX_PATH]{};
		GetModuleFileNameA(aModule, buff, MAX_PATH);

		return buff;
	}

	std::filesystem::path GetSystem(const char* aAppend)
	{
		char buff[MAX_PATH]{};
		GetSystemDirectoryA(buff, MAX_PATH);

		if (!aAppend)
		{
			/* return just the system path */
			return buff;
		}

		std::filesystem::path path = buff;
		path.append(aAppend);

		return path;
	}

	void CreateDir(const std::filesystem::path& aDirectory)
	{
		if (!std::filesystem::exists(aDirectory))
		{
			std::filesystem::create_directories(aDirectory);
		}
	}

	std::filesystem::path GetUnused(const std::filesystem::path& aPath, bool aOverwriteExtension)
	{
		std::filesystem::path probe = aPath;

		int i = 0;
		while (std::filesystem::exists(probe))
		{
			probe = aPath.string() + "_" + std::to_string(i);
			i++;
		}

		return probe;
	}
}
