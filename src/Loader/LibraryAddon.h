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

#include "EUpdateProvider.h"

///----------------------------------------------------------------------------------------------------
/// LibraryAddon Struct
///----------------------------------------------------------------------------------------------------
struct LibraryAddon
{
	signed int      Signature;
	std::string     Name;
	std::string     Author;
	std::string     Description;
	EUpdateProvider Provider;
	std::string     DownloadURL;
	std::string     ToSComplianceNotice;
	int             PolicyTier;
	bool            IsNew;
};

#endif
