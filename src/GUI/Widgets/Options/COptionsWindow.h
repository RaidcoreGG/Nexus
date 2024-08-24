#ifndef GUI_OPTIONSWINDOW_H
#define GUI_OPTIONSWINDOW_H

#include <string>

#include "UI/Controls/CtlWindow.h"

namespace GUI
{
	class COptionsWindow : public IWindow
	{
		public:
			COptionsWindow(std::string aName);
			void Render();
	};
}

#endif