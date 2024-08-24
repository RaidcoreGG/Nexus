///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Shortcut.h
/// Description  :  Contains the structs holding information about a shortcut.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef QUICKACCESS_SHORTCUT_H
#define QUICKACCESS_SHORTCUT_H

#include <string>
#include <map>

#include "Services/Textures/Texture.h"
#include "UI/FuncDefs.h"

struct ContextItem
{
	std::string TargetShortcut;
	GUI_RENDER Callback;
};

struct Shortcut
{
	int										TextureGetAttempts;
	std::string								TextureNormalIdentifier;
	std::string								TextureHoverIdentifier;

	Texture* TextureNormal;
	Texture* TextureHover;
	std::string								InputBind;
	std::string								TooltipText;
	bool									IsHovering;
	bool									HasNotification;
	std::map<std::string, ContextItem>		ContextItems;
};

#endif
