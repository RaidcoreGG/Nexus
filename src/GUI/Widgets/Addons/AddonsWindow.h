#ifndef GUI_ADDONSWINDOW_H
#define GUI_ADDONSWINDOW_H

#include <string>

#include "GUI/IWindow.h"

namespace GUI
{
	class AddonsWindow : public IWindow
	{
	public:
		AddonsWindow(std::string aName);
		void Render();
	};
}

#endif