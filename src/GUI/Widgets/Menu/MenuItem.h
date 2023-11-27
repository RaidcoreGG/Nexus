#ifndef GUI_MENUITEM_H
#define GUI_MENUITEM_H

#include <string>

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