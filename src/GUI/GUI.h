#ifndef GUI_H
#define GUI_H

#include "../imgui/imgui.h"
#include "../imgui/imgui_extensions.h"

//#include "Addons.h"
//#include "Keybinds.h"
//#include "Log.h"
#include "About.h"

namespace GUI
{
	//extern Addons		AddonsWindow;
	//extern Keybinds	KeybindsWindow;
	//extern Log		LogWindow;
	extern About		AboutWindow;

	void Initialize();
	void Shutdown();

	void Render();
	void ShowMenu();
}

#endif