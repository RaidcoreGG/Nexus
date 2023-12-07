#ifndef GUI_EULAMODAL_H
#define GUI_EULAMODAL_H

#include "GUI/IWindow.h"

namespace GUI
{
	class EULAModal : public IWindow
	{
	public:
		EULAModal();
		void Render();
	};
}

#endif