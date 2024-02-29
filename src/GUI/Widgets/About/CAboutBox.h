#ifndef GUI_ABOUTBOX_H
#define GUI_ABOUTBOX_H

#include <string>

#include "GUI/IWindow.h"

namespace GUI
{
	class CAboutBox : public IWindow
	{
	public:
		CAboutBox(std::string aName);
		void Render();
	};
}

#endif