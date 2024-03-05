#ifndef GUI_CHANGELOGWINDOW_H
#define GUI_CHANGELOGWINDOW_H

#include <string>

#include "GUI/IWindow.h"

namespace GUI
{
	class CChangelogWindow : public IWindow
	{
	public:
		CChangelogWindow(std::string aName);
		void Render();
	};
}

#endif