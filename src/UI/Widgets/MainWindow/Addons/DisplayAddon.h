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
	None,
	Nexus,
	Library,
	Arc,
	LibraryArc
};

struct AddonItemData_t
{
	EAddonType                                         Type;
	union
	{
		Addon_t*                                       NexusAddon;
		LibraryAddon_t*                                LibraryAddon_t;
	};
	std::unordered_map<std::string, InputBindPacked_t> InputBinds;
	GUI_RENDER                                         OptionsRender;
	bool                                               IsHovered;
};

#endif
