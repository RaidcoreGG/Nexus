#ifndef GUI_ALERTS_H
#define GUI_ALERTS_H

#include <mutex>
#include <vector>
#include <string>

#include "GUI/FuncDefs.h"

#include "imgui.h"
#include "imgui_extensions.h"

namespace GUI
{
	namespace Alerts
	{
		extern float						Opacity;

		extern std::mutex					Mutex;
		extern std::vector<std::string>		QueuedAlerts;

		extern std::thread					AnimationThread;
		extern bool							IsAnimating;

		void Render();

		void Notify(const char* aMessage);
	}
}

#endif
