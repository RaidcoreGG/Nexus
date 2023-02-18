#ifndef GUI_KEYBINDSWINDOW_H
#define GUI_KEYBINDSWINDOW_H

#include "../../../Shared.h"
#include "../../../Paths.h"
#include "../../../State.h"

#include "../../../Keybinds/KeybindHandler.h"

#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_extensions.h"

#include "../../IWindow.h"

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