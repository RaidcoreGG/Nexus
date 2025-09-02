///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  NexusLink.h
/// Description  :  Contains the definition for the NexusLinkData_t struct.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef NEXUSLINK_H
#define NEXUSLINK_H

#include <cstdint>

constexpr const char* DL_NEXUS_LINK = "DL_NEXUS_LINK";

///----------------------------------------------------------------------------------------------------
/// NexusLinkData_t Struct
///----------------------------------------------------------------------------------------------------
struct NexusLinkData_t
{
	uint32_t Width;
	uint32_t Height;
	float    Scaling;

	bool     IsMoving;
	bool     IsCameraMoving;
	bool     IsGameplay;

	void*    Font;    /* ImFont* */
	void*    FontBig; /* ImFont* */
	void*    FontUI;  /* ImFont* */

	int32_t  QuickAccessIconsCount;
	int32_t  QuickAccessMode;
	bool     QuickAccessIsVertical;
};

#endif
