///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Paths.h
/// Description  :  Contains a variety of utility for the filesystem and paths.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef PATHS_H
#define PATHS_H

#include <filesystem>

///----------------------------------------------------------------------------------------------------
/// Path Namespace
///----------------------------------------------------------------------------------------------------
namespace Path
{
	///----------------------------------------------------------------------------------------------------
	/// GetModule:
	/// 	Returns the path of the specified module.
	///----------------------------------------------------------------------------------------------------
	std::filesystem::path GetModule(HMODULE aModule);

	///----------------------------------------------------------------------------------------------------
	/// GetSystem:
	/// 	Returns a system path with the specified string appended.
	///----------------------------------------------------------------------------------------------------
	std::filesystem::path GetSystem(const char* aAppend = nullptr);

	///----------------------------------------------------------------------------------------------------
	/// CreateDir:
	/// 	Wrapper function to create directories recursively.
	///----------------------------------------------------------------------------------------------------
	void CreateDir(const std::filesystem::path& aDirectory);

	///----------------------------------------------------------------------------------------------------
	/// GetUnused:
	/// 	Returns an unused path based on the passed one. Increments a _# suffix until unused.
	/// 	aOverWriteExtension = true will append the suffix to the extension rather than the filename.
	///----------------------------------------------------------------------------------------------------
	std::filesystem::path GetUnused(const std::filesystem::path& aPath, bool aOverwriteExtension = false);
}

#endif
