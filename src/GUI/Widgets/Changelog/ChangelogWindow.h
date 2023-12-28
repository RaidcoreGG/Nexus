#ifndef GUI_CHANGELOGWINDOW_H
#define GUI_CHANGELOGWINDOW_H

#include <string>

#include "GUI/IWindow.h"

namespace GUI
{
	class ChangelogWindow : public IWindow
	{
	public:
		ChangelogWindow(std::string aName);
		void Render();
	};
}

#endif