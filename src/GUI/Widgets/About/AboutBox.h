#ifndef GUI_ABOUTBOX_H
#define GUI_ABOUTBOX_H

#include <string>

#include "GUI/IWindow.h"

namespace GUI
{
	class AboutBox : public IWindow
	{
	public:
		AboutBox(std::string aName);
		void Render();
	};
}

#endif