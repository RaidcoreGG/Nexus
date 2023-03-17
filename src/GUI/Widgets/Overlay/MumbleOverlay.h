#ifndef GUI_MUMBLEOVERLAY_H
#define GUI_MUMBLEOVERLAY_H

#include "../../../Shared.h"
#include "../../../State.h"

#include "../../../nlohmann/json.hpp"

#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_extensions.h"

#include "../../IWindow.h"

namespace GUI
{
	class MumbleOverlay : public IWindow
	{
	public:
		void Render();
	};
}

#endif