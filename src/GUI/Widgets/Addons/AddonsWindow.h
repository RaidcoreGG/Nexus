#ifndef GUI_ADDONSWINDOW_H
#define GUI_ADDONSWINDOW_H

#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_extensions.h"

#include "../../IWindow.h"

namespace GUI
{
	class AddonsWindow : public IWindow
	{
	public:
		void Render();
		void MenuOption(int aCategory);
	};
}

#endif