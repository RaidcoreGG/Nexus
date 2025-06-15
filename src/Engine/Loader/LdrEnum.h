///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LdrEnum.h
/// Description  :  Enumerations for the loader.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef LDRENUM_H
#define LDRENUM_H

#include <windows.h>
#include <string>

#include "Util/Strings.h"

///----------------------------------------------------------------------------------------------------
/// EAddonFlags Enumeration
///----------------------------------------------------------------------------------------------------
enum class EAddonFlags
{
	None                             = 0,
	IsVolatile                       = 1 << 0, /* is game build dependant and wants to be disabled after game updates */
	DisableHotloading                = 1 << 1, /* prevents unloading at runtime, aka. will require a restart if updated, etc. */
	OnlyLoadDuringGameLaunchSequence = 1 << 2, /* prevents loading the addon later than the initial character select */
	CanCreateImGuiContext            = 1 << 3  /* addon is capable of receiving nullptr instead of imgui context and allocators and can manage its own */
};

DEFINE_ENUM_FLAG_OPERATORS(EAddonFlags);

///----------------------------------------------------------------------------------------------------
/// EAddonState Enumeration
///----------------------------------------------------------------------------------------------------
enum class EAddonState
{
	None,

	NotLoaded,                /* Addon is not loaded. */
	NotLoadedDuplicate,       /* Addon is not loaded, because it has the same signature as another addon. */
	NotLoadedIncompatible,    /* The file is incompatible with Nexus. */
	NotLoadedIncompatibleAPI, /* Addon requested an API that doesn't exist. */

	Loaded,                   /* Addon is loaded. */
	LoadedLOCKED              /* Addon is loaded, but locked and mustn't be unloaded. */
};

///----------------------------------------------------------------------------------------------------
/// ELoaderAction Enumeration
///----------------------------------------------------------------------------------------------------
enum class ELoaderAction
{
	None,
	Load,
	Unload,
	Uninstall,
	Reload,
	FreeLibrary,
	FreeLibraryThenLoad
};

///----------------------------------------------------------------------------------------------------
/// EUpdateProvider Enumeration
///----------------------------------------------------------------------------------------------------
enum class EUpdateProvider
{
	None,     /* Does not support auto updating */
	Raidcore, /* Provider is Raidcore (via API) */
	GitHub,   /* Provider is GitHub Releases */
	Direct,   /* Provider is direct file link */
	Self      /* Provider is self check, addon has to request manually and version will not be verified */
};

inline EUpdateProvider GetProvider(const std::string& aUrl)
{
	if (aUrl.empty())
	{
		return EUpdateProvider::None;
	}

	if (String::Contains(aUrl, "raidcore.gg"))
	{
		return EUpdateProvider::Raidcore;
	}

	if (String::Contains(aUrl, "github.com"))
	{
		return EUpdateProvider::GitHub;
	}

	return EUpdateProvider::Direct;
}

#endif
