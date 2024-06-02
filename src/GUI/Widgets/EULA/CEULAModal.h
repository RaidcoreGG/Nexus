#ifndef GUI_EULAMODAL_H
#define GUI_EULAMODAL_H

#include "GUI/IWindow.h"

#include "Textures/Texture.h"

namespace GUI
{
	class CEULAModal : public IWindow
	{
	public:
		Texture* Background;
		Texture* TitleBar;
		Texture* TitleBarHover;
		Texture* TitleBarEnd;
		Texture* TitleBarEndHover;
		Texture* BtnClose;
		Texture* BtnCloseHover;

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