#ifndef GUI_EULAMODAL_H
#define GUI_EULAMODAL_H

#include "GUI/IWindow.h"

namespace GUI
{
	class CEULAModal : public IWindow
	{
	public:
		CEULAModal();
		void Render();
	};
}

#endif