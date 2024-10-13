///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  DisplayAddon.h
/// Description  :  Contains the definition for an addon listing.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef MAINWINDOW_DISPLAYADDON_H
#define MAINWINDOW_DISPLAYADDON_H

#include <map>
#include <string>

#include "Loader/Addon.h"
#include "Loader/LibraryAddon.h"
#include "UI/DisplayBinds.h"
#include "UI/FuncDefs.h"

enum class EAddonType
{
	Nexus,
	Library,
	ArcDPS
};

struct AddonItemData
{
	EAddonType                             Type;
	union
	{
		Addon*                             NexusAddon;
		LibraryAddon*                      LibraryAddon;
		//ArcDPS
	};
	std::map<std::string, InputBindPacked> InputBinds;
	GUI_RENDER                             OptionsRender;
};

#endif
