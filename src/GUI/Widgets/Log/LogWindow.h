#ifndef GUI_LOGWINDOW_H
#define GUI_LOGWINDOW_H

#include <string>

#include "GUI/IWindow.h"
#include "Logging/ILogger.h"
#include "Logging//ELogLevel.h"
#include "Logging/LogEntry.h"

namespace GUI
{
	class LogWindow : public IWindow, public virtual ILogger
	{
	public:
		LogWindow(std::string aName, ELogLevel aLogLevel);

		void Render();
		void LogMessage(LogEntry aLogEntry);
	};
}

#endif