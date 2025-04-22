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
	None,     /* Does not support auto updating. */
	Raidcore, /* Provider is Raidcore API. */
	GitHub,   /* Provider is GitHub Releases. */
	Direct,   /* Provider is direct file URL. */
	Self      /* Provider is self check, addon has to request manually and version will not be verified. */
};

///----------------------------------------------------------------------------------------------------
/// Merge:
/// 	Deduces the update provider based on known URLs.
///----------------------------------------------------------------------------------------------------
EUpdateProvider GetProvider(const std::string& aUrl);

#endif
