#ifndef GUI_LOGWINDOW_H
#define GUI_LOGWINDOW_H

#include "../../../Shared.h"
#include "../../../Paths.h"
#include "../../../State.h"

#include "../../../Logging/LogHandler.h"
#include "../../../Logging/ILogger.h"

#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_extensions.h"

#include "../../IWindow.h"

#include <vector>

namespace GUI
{
	class LogWindow : public IWindow, public virtual ILogger
	{
	public:
		void Render();
		void MenuOption(int aCategory);
		void LogMessage(LogEntry aLogEntry);
	};
}

#endif