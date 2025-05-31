///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  NexusLinkData.h
/// Description  :  Contains the definition for the NexusLinkData_t struct.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef NEXUSLINKDATA_H
#define NEXUSLINKDATA_H

constexpr const char* DL_NEXUS_LINK = "DL_NEXUS_LINK";

///----------------------------------------------------------------------------------------------------
/// NexusLinkData_t Struct
///----------------------------------------------------------------------------------------------------
struct NexusLinkData_t
{
	unsigned	Width;
	unsigned	Height;
	float		Scaling;

	bool		IsMoving;
	bool		IsCameraMoving;
	bool		IsGameplay;

	void*		Font;
	void*		FontBig;
	void*		FontUI;

	signed int	QuickAccessIconsCount;
	signed int	QuickAccessMode;
	bool		QuickAccessIsVertical;
};

#endif