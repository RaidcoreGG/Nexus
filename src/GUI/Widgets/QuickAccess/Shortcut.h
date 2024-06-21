#ifndef QUICKACCESS_SHORTCUT_H
#define QUICKACCESS_SHORTCUT_H

#include <string>

#include "Services/Textures/Texture.h"

namespace GUI
{
	struct Shortcut
	{
		int						TextureGetAttempts;
		std::string				TextureNormalIdentifier;
		std::string				TextureHoverIdentifier;

		Texture*				TextureNormal;
		Texture*				TextureHover;
		std::string				Keybind;
		std::string				TooltipText;
		bool					IsHovering;
		bool					HasNotification;
	};
}

#endif