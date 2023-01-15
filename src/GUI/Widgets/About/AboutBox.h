#ifndef GUI_ABOUTBOX_H
#define GUI_ABOUTBOX_H

#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_extensions.h"

#include "../../IWindow.h"

namespace GUI
{
	class AboutBox : public IWindow
	{
	public:
		void Render();
		void MenuOption(int aCategory);
	};
}

#endif