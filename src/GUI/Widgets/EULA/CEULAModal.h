#ifndef GUI_EULAMODAL_H
#define GUI_EULAMODAL_H

#include "GUI/IWindow.h"

#include "Services/Textures/Texture.h"

namespace GUI
{
	class CEULAModal : public IWindow
	{
	public:
		Texture* Background	= nullptr;
		Texture* TitleBar = nullptr;
		Texture* TitleBarHover = nullptr;
		Texture* TitleBarEnd = nullptr;
		Texture* TitleBarEndHover = nullptr;
		Texture* BtnClose = nullptr;
		Texture* BtnCloseHover = nullptr;

		bool TitleBarControlled;
		bool CloseHovered;

		CEULAModal();
		void Render();

	private:
		float windowWidth = 620.0f;
		float windowHeight = 480.0f;
		float contentWidth = 540.0f;
		float contentHeight = 424.0f;
	};
}

#endif