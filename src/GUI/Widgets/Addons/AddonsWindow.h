#ifndef GUI_ADDONSWINDOW_H
#define GUI_ADDONSWINDOW_H

#include <string>
#include <vector>
#include <mutex>

#include "GUI/IWindow.h"
#include "Textures/Texture.h"
#include "Loader/LibraryAddon.h"

namespace GUI
{
	class AddonsWindow : public IWindow
	{
	public:
		Texture* Background;
		Texture* TitleBar;
		Texture* TitleBarHover;
		Texture* TitleBarEnd;
		Texture* TitleBarEndHover;
		Texture* BtnClose;
		Texture* BtnCloseHover;
		Texture* TabBtn;
		Texture* TabBtnHover;

		bool TitleBarControlled;
		bool CloseHovered;

		int TabIndex;
		bool Tab1Hovered;
		bool Tab2Hovered;

		AddonsWindow(std::string aName);
		void Render();
	};
}

#endif