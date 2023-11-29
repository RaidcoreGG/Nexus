#ifndef GUI_DEBUGWINDOW_H
#define GUI_DEBUGWINDOW_H

#include <string>

#include "GUI/IWindow.h"
#include "GUI/Widgets/Overlay/MumbleOverlay.h"

namespace GUI
{
	class DebugWindow : public IWindow
	{
		public:
		DebugWindow(std::string aName);
		~DebugWindow() = default;

		void Render();

		MumbleOverlay* MumbleWindow;
	};
}

#endif