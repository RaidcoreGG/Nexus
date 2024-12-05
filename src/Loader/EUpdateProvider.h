///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  EUpdateProvider.h
/// Description  :  Contains the update providers and related logic.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef EUPDATEPROVIDER_H
#define EUPDATEPROVIDER_H

#include <string>

///----------------------------------------------------------------------------------------------------
/// EUpdateProvider Enumeration
///----------------------------------------------------------------------------------------------------
enum class EUpdateProvider
{
	None     = 0, /* Does not support auto updating */
	Raidcore = 1, /* Provider is Raidcore API. */
	GitHub   = 2, /* Provider is GitHub Releases. */
	Direct   = 3, /* Provider is direct file URL. */
	Self     = 4  /* Provider is self check, addon has to request manually and version will not be verified. */
};

///----------------------------------------------------------------------------------------------------
/// Merge:
/// 	Deduces the update provider based on known URLs.
///----------------------------------------------------------------------------------------------------
EUpdateProvider GetProvider(const std::string& aUrl);

#endif
