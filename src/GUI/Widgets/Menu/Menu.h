#ifndef GUI_MENU_H
#define GUI_MENU_H

#include <mutex>
#include <vector>

#include "../../../Shared.h"
#include "../../../Paths.h"
#include "../../../State.h"

#include "../../../resource.h"

#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_extensions.h"

#include "../../IWindow.h"
#include "MenuItem.h"

namespace GUI
{
	class MenuWindow : public IWindow
	{
		public:
		std::mutex Mutex;
		std::vector<MenuItem*> MenuItems;

		void Render();
		void AddMenuItem(const char* aLabel, bool* aToggle);
	};
}

#endif