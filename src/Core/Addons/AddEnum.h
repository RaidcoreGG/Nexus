///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AddEnum.h
/// Description  :  Enumerations for addons.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef ADDENUM_H
#define ADDENUM_H

#include <cstdint>
#include <windows.h>

///----------------------------------------------------------------------------------------------------
/// EAddonInterfaces Enumeration
///----------------------------------------------------------------------------------------------------
enum class EAddonInterfaces : uint32_t
{
	None        = 0,
	Nexus       = 1 << 0,
	ArcDPS      = 1 << 1,
	AddonLoader = 1 << 2,
	D3D11Proxy  = 1 << 3,
	DXGIProxy   = 1 << 4
};
DEFINE_ENUM_FLAG_OPERATORS(EAddonInterfaces)

///----------------------------------------------------------------------------------------------------
/// EUpdateProvider Enumeration
///----------------------------------------------------------------------------------------------------
enum class EUpdateProvider : uint32_t
{
	None,     /* Does not support auto updating */
	Raidcore, /* Provider is Raidcore (via API) */
	GitHub,   /* Provider is GitHub Releases */
	Direct,   /* Provider is direct file link */
	Self      /* Provider is self check, addon has to request manually and version will not be verified */
};

///----------------------------------------------------------------------------------------------------
/// EAddonAction Enumeration
///----------------------------------------------------------------------------------------------------
enum class EAddonAction : uint32_t
{
	None,
	Load,
	Unload,
	Uninstall,
	CheckUpdate,
	Update,
	EnumInterfaces,
	Destroy
};

///----------------------------------------------------------------------------------------------------
/// EAddonFlags Enumeration
///----------------------------------------------------------------------------------------------------
enum class EAddonFlags : uint32_t
{
	None            = 0,
	FileLocked      = 1 << 0,
	StateLocked     = 1 << 1,
	UpdateAvailable = 1 << 2,
	RunningAction   = 1 << 3,
	Uninstalled     = 1 << 4,
	MissingReqs     = 1 << 5
};
DEFINE_ENUM_FLAG_OPERATORS(EAddonFlags)

#endif
