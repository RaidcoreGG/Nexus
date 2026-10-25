///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ExeExports.h
/// Description  :  Reads the exports of a PE file without loading it.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "ExeEnum.h"

///----------------------------------------------------------------------------------------------------
/// Raidcore::Core::Platform::Windows Namespace
///----------------------------------------------------------------------------------------------------
namespace Raidcore::Core::Platform::Windows
{
	///----------------------------------------------------------------------------------------------------
	/// Exports_t Struct
	///----------------------------------------------------------------------------------------------------
	struct Exports_t
	{
		EArchitecture                             Architecture = EArchitecture::None;
		std::unordered_map<std::string, uint16_t> Names;           /* Export name to ordinal.            */
		std::vector<uint16_t>                     UnnamedOrdinals; /* Exports only reachable by ordinal. */

		///----------------------------------------------------------------------------------------------------
		/// Has:
		/// 	Returns true if the export exists.
		///----------------------------------------------------------------------------------------------------
		inline bool Has(std::string aName) const
		{
			return this->Names.find(aName) != this->Names.end();
		}
	};

	///----------------------------------------------------------------------------------------------------
	/// ReadExports:
	/// 	Reads the exports of a DLL from disk, no code of the DLL is executed.
	/// 	Architecture is None if the file cannot be read or is not a DLL. Exports are read for any
	/// 	architecture, compare against ARCH_CURRENT before loading.
	///----------------------------------------------------------------------------------------------------
	Exports_t ReadExports(const std::filesystem::path& aPath);
}
