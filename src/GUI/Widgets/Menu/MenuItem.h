#ifndef GUI_MENUITEM_H
#define GUI_MENUITEM_H

#include <string>
#include "Textures/Texture.h"

namespace GUI
{
	namespace Menu
	{
		struct MenuItem
		{
			std::string		Label;
			std::string		TextureIdentifier;
			unsigned int	ResourceID;
			bool*			Toggle;
			Texture*		Icon;
			bool			IsHovering;

			bool			Render();
		};
	}
}

#endif