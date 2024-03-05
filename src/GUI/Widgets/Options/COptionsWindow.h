#ifndef GUI_KEYBINDSWINDOW_H
#define GUI_KEYBINDSWINDOW_H

#include <string>

#include "GUI/IWindow.h"

namespace GUI
{
	class COptionsWindow : public IWindow
	{
		public:
			COptionsWindow(std::string aName);
			void Render();
	};
}

#endif