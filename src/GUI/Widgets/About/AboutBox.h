#ifndef GUI_ABOUTBOX_H
#define GUI_ABOUTBOX_H

#include "../../../Shared.h"
#include "../../../Paths.h"
#include "../../../State.h"
#include "../../../Branch.h"

#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_extensions.h"

#include "../../IWindow.h"

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