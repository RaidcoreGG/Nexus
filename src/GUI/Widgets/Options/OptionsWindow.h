#ifndef GUI_KEYBINDSWINDOW_H
#define GUI_KEYBINDSWINDOW_H

#include <string>

#include "GUI/IWindow.h"

namespace GUI
{
	class OptionsWindow : public IWindow
	{
		public:
			OptionsWindow(std::string aName);
			void Render();
	};
}

#endif