///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LibAddon.h
/// Description  :  Contains the definition for a library addon listing.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef LIBADDON_H
#define LIBADDON_H

#include <cstdint>
#include <string>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

///----------------------------------------------------------------------------------------------------
/// LibraryAddon_t Struct
///----------------------------------------------------------------------------------------------------
struct LibraryAddon_t
{
	int32_t     Signature;
	std::string Name;
	std::string Author;
	std::string Description;
	std::string DownloadURL;
	int32_t     PolicyTier;
	std::string FriendlyFilename;

	LibraryAddon_t(json& aJson);
};

#endif
