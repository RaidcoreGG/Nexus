#ifndef GUI_LOGWINDOW_H
#define GUI_LOGWINDOW_H

#include <string>
#include <vector>

#include "GUI/IWindow.h"
#include "Services/Logging/ILogger.h"
#include "Services/Logging//ELogLevel.h"
#include "Services/Logging/LogEntry.h"

namespace GUI
{
	class CLogWindow : public IWindow, public virtual ILogger
	{
	public:
		CLogWindow(std::string aName, ELogLevel aLogLevel);

		void Render();
		void LogMessage(LogEntry aLogEntry);

	private:
		std::vector<LogEntry>		LogEntries;
		std::vector<std::string>	Channels;
	};
}

#endif