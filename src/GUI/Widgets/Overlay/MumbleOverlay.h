#ifndef GUI_MUMBLEOVERLAY_H
#define GUI_MUMBLEOVERLAY_H

#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_extensions.h"

#include "../../IWindow.h"

namespace GUI
{
	class MumbleOverlay : public IWindow
	{
	public:
		void Render();
		void MenuOption(const wchar_t* aCategory);
	};
}

#endif