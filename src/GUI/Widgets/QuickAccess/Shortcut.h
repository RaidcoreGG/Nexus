#ifndef QUICKACCESS_SHORTCUT_H
#define QUICKACCESS_SHORTCUT_H

#include <string>
#include <map>

#include "Services/Textures/Texture.h"
#include "GUI/FuncDefs.h"

namespace GUI
{
	struct SimpleShortcut
	{
		std::string TargetShortcut;
		GUI_RENDER Callback;
	};

	struct Shortcut
	{
		int											TextureGetAttempts;
		std::string									TextureNormalIdentifier;
		std::string									TextureHoverIdentifier;

		Texture*									TextureNormal;
		Texture*									TextureHover;
		std::string									InputBind;
		std::string									TooltipText;
		bool										IsHovering;
		bool										HasNotification;
		std::map<std::string, SimpleShortcut>		ContextItems;
	};
}

#endif