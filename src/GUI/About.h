#ifndef GUI_ABOUT_H
#define GUI_ABOUT_H

#include "../imgui/imgui.h"
#include "../imgui/imgui_extensions.h"

#include "IWindow.h"

namespace GUI
{
	class About : public IWindow
	{
	public:
		void Show();
	};
}

#endif