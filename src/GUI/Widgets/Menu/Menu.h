#ifndef GUI_MENU_H
#define GUI_MENU_H

#include <mutex>
#include <vector>

#include "../../../Shared.h"
#include "../../../Paths.h"
#include "../../../State.h"

#include "../../../resource.h"
#include "../../../Textures/Texture.h"

#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_extensions.h"

#include "../../IWindow.h"
#include "MenuItem.h"

namespace GUI
{
	namespace Menu
	{
		extern bool Visible;

		extern std::mutex Mutex;
		extern std::vector<MenuItem*> MenuItems;

		extern Texture* MenuBG;
		extern Texture* MenuButton;
		extern Texture* MenuButtonHover;

		void Render();
		void AddMenuItem(std::string aLabel, bool* aToggle);

		void ReceiveTextures(std::string aIdentifier, Texture* aTexture);
	};
}

#endif