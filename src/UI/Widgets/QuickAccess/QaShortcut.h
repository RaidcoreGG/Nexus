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

#include "Services/Textures/TxTexture.h"
#include "UI/FuncDefs.h"

struct ContextItem
{
	std::string                        TargetShortcut;
	GUI_RENDER                         Callback;
};

struct Shortcut
{
	bool                               IsValid;

	int                                TextureGetAttempts;
	std::string                        TextureNormalIdentifier;
	std::string                        TextureHoverIdentifier;

	Texture*                           TextureNormal;
	Texture*                           TextureHover;
	std::string                        IBIdentifier;
	std::string                        IBText;
	std::string                        TooltipText;
	bool                               IsHovering;
	bool                               HasNotification;
	std::map<std::string, ContextItem> ContextItems;
};

#endif
