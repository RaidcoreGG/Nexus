///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  EAddonInterface.h
/// Description  :  Contains the addon interfaces.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef EADDONINTERFACE_H
#define EADDONINTERFACE_H

///----------------------------------------------------------------------------------------------------
/// EAddonInterface Enumeration
///----------------------------------------------------------------------------------------------------
enum class EAddonInterface
{
	None        = 0,
	Nexus       = 1 << 0,
	ArcDPS      = 1 << 1,

	/// Note:
	/// 	For Addon Loader the interface is not sufficient, you also need to conform to the file name, which is stupid.
	/// 	Revisit this at some point maybe, but it's not worth the effort just to support 2 addons on an abandoned loader.
	AddonLoader = 1 << 2,
	D3D11Proxy  = 1 << 3,
	DXGIProxy   = 1 << 4
};

DEFINE_ENUM_FLAG_OPERATORS(EAddonInterface)

#endif
