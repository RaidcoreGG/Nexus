#ifndef GUI_DEBUGWINDOW_H
#define GUI_DEBUGWINDOW_H

#include "../../../Shared.h"
#include "../../../State.h"

#include "../../../Events/EventHandler.h"
#include "../../../Keybinds/KeybindHandler.h"
#include "../../../DataLink/DataLink.h"
#include "../../../Textures/TextureLoader.h"
#include "../../../GUI/Widgets/QuickAccess/QuickAccess.h"
#include "../../../Loader/Loader.h"

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