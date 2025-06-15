///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  DisplayAddon.h
/// Description  :  Contains the definition for an addon listing.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef MAINWINDOW_DISPLAYADDON_H
#define MAINWINDOW_DISPLAYADDON_H

#include <unordered_map>
#include <string>

#include "Core/Addons/Library/LibAddon.h"
#include "Engine/Loader/Addon.h"
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
	Addon_t*                                           NexusAddon;
	LibraryAddon_t                                     LibraryAddon;
	std::unordered_map<std::string, InputBindPacked_t> InputBinds;
	GUI_RENDER                                         OptionsRender;
	bool                                               IsHovered;
	bool                                               IsInstalling;

	AddonItemData_t() = default;
	~AddonItemData_t() = default;
};

#endif
