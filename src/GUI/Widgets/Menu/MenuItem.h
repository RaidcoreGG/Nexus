#ifndef GUI_MENUITEM_H
#define GUI_MENUITEM_H

#include <string>

#include "../../../Renderer.h"

#include "../../../Textures/Texture.h"

#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_extensions.h"

namespace GUI
{
	namespace Menu
	{
		struct MenuItem
		{
			std::string		Label;
			bool*			Toggle;
			bool			IsHovering;

			bool			Render();
		};
	}
}

#endif