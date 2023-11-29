#ifndef GUI_MENU_H
#define GUI_MENU_H

#include <mutex>
#include <vector>

#include "MenuItem.h"
#include "Textures/Texture.h"

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

		void ReceiveTextures(const char* aIdentifier, Texture* aTexture);
	};
}

#endif