#ifndef GUI_MENU_H
#define GUI_MENU_H

#include <mutex>
#include <vector>

#include "MenuItem.h"
#include "Services/Textures/Texture.h"

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
		void AddMenuItem(std::string aLabel, std::string aTextureIdentifier, unsigned int aResourceID, bool* aToggle);
	};
}

#endif