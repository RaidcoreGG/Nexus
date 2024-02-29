#ifndef GUI_DEBUGWINDOW_H
#define GUI_DEBUGWINDOW_H

#include <string>

#include "GUI/IWindow.h"
#include "GUI/Widgets/Overlay/CMumbleOverlay.h"

namespace GUI
{
	class CDebugWindow : public IWindow
	{
		public:
		CDebugWindow(std::string aName);
		~CDebugWindow() = default;

		void Render();

		CMumbleOverlay* MumbleWindow;
	};
}

#endif