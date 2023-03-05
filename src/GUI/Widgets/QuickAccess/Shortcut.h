#ifndef QUICKACCESS_SHORTCUT_H
#define QUICKACCESS_SHORTCUT_H

#include <string>

#include "../../../Textures/Texture.h"

namespace GUI
{
	struct Shortcut
	{
		Texture					TextureNormal;
		Texture					TextureHover;
		std::string				Keybind;
		std::string				TooltipText;
		bool					IsHovering;
	};
}

#endif