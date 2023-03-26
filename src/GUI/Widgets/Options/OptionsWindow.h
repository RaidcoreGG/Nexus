#ifndef GUI_KEYBINDSWINDOW_H
#define GUI_KEYBINDSWINDOW_H

#include <regex>

#include "../../../Shared.h"
#include "../../../Paths.h"
#include "../../../State.h"
#include "../../../Renderer.h"

#include "../../../Keybinds/KeybindHandler.h"
#include "../../../API/APIController.h"

#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_extensions.h"

#include "../../IWindow.h"

namespace GUI
{
	class OptionsWindow : public IWindow
	{
		public:
		void Render();
	};
}

#endif