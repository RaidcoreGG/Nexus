///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  QaShortcut.h
/// Description  :  Contains the structs holding information about a shortcut.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef QASHORTCUT_H
#define QASHORTCUT_H

#include <string>
#include <map>

#include "Engine/Textures/TxTexture.h"
#include "UI/FuncDefs.h"

struct ContextItem_t
{
	std::string                          TargetShortcut;
	GUI_RENDER                           Callback;
};

struct Shortcut_t
{
	bool                                 IsValid;

	int                                  TextureGetAttempts;
	std::string                          TextureNormalIdentifier;
	std::string                          TextureHoverIdentifier;

	Texture_t*                           TextureNormal;
	Texture_t*                           TextureHover;
	std::string                          IBIdentifier;
	std::string                          IBText;
	std::string                          TooltipText;
	bool                                 IsHovering;
	std::vector<std::string>             Notifications;
	std::map<std::string, ContextItem_t> ContextItems;
};

#endif
