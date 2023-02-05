#ifndef GUI_DEBUGWINDOW_H
#define GUI_DEBUGWINDOW_H

#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_extensions.h"

#include "../../IWindow.h"

namespace GUI
{
	class DebugWindow : public IWindow
	{
		public:
		void Render();
		void MenuOption(int aCategory);
	};
}

#endif