#ifndef GUI_KEYBINDSWINDOW_H
#define GUI_KEYBINDSWINDOW_H

#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_extensions.h"

#include "../../IWindow.h"

#include "../../../Keybinds/KeybindHandler.h"

namespace GUI
{
	class KeybindsWindow : public IWindow
	{
		public:
		void Render();
		void MenuOption(int aCategory);
	};
}

#endif