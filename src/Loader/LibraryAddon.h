///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LibraryAddon.h
/// Description  :  Contains the definition for a library addon listing.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef LIBRARYADDON_H
#define LIBRARYADDON_H

#include <string>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

///----------------------------------------------------------------------------------------------------
/// LibraryAddon Struct
///----------------------------------------------------------------------------------------------------
struct LibraryAddon
{
	signed int  Signature;
	std::string Name;
	std::string Author;
	std::string Description;
	std::string DownloadURL;
	int         PolicyTier;
	std::string FriendlyFilename;

	LibraryAddon(json& aJson);
};

#endif
