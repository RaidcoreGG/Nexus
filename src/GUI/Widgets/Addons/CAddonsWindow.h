#ifndef GUI_ADDONSWINDOW_H
#define GUI_ADDONSWINDOW_H

#include <string>
#include <vector>
#include <mutex>

#include "GUI/IWindow.h"
#include "Services/Textures/Texture.h"
#include "Loader/LibraryAddon.h"

namespace GUI
{
	class CAddonsWindow : public IWindow
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
		Texture* BtnRefresh;
		Texture* BtnRefreshHover;

		bool TitleBarControlled;
		bool CloseHovered;

		int TabIndex;
		bool Tab1Hovered;
		bool Tab2Hovered;
		bool Tab3Hovered;
		bool Tab4Hovered;

		CAddonsWindow(std::string aName);
		void Render();

	private:
		float windowWidth = 620.0f;
		float windowHeight = 480.0f;
		float contentWidth = 540.0f;
		float contentHeight = 424.0f;

		bool showInstalled = false;
		bool refreshHovered = false;

		int queuedForCheck = 0;
		int checkedForUpdates = -1;
		int updatedCount = 0;
	};
}

#endif