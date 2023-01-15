#ifndef GUI_LOGWINDOW_H
#define GUI_LOGWINDOW_H

#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_extensions.h"

#include "../../IWindow.h"
#include "../../../Logging/ILogger.h"

#include <vector>

namespace GUI
{
	class LogWindow : public IWindow, public virtual ILogger
	{
	public:
		void Render();
		void MenuOption(const wchar_t* aCategory);
		void LogMessage(LogEntry aLogEntry);
	};
}

#endif