///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Log.h
/// Description  :  Contains the content of the log window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef MAINWINDOW_LOG_H
#define MAINWINDOW_LOG_H

#include <vector>
#include <string>

#include "imgui/imgui.h"

#include "Services/Logging/ILogger.h"
#include "Services/Logging/LogEntry.h"
#include "UI/Controls/CtlSubWindow.h"

class CLogWindow : public ISubWindow, public virtual ILogger
{
	public:
	CLogWindow();
	void RenderContent() override;
	void LogMessage(LogEntry* aLogEntry) override;

	private:
	enum class EMessagePartType {
		None,
		Text,
		ColorPush,
		ColorPop,
		LineBreak
	};

	struct MessagePart
	{
		EMessagePartType Type;
		std::string      Text;
		ImVec4           Color;
	};

	struct DisplayLogEntry
	{
		LogEntry* Entry;
		std::vector<MessagePart> Parts;
	};

	struct LogChannel
	{
		bool        IsSelected;
		std::string Name;
	};

	int                           MaxShownCount = 400;
	ELogLevel                     FilterLevel = ELogLevel::ALL;
	bool                          SelectedLevelOnly;

	std::vector<DisplayLogEntry*> LogEntries;
	std::vector<LogChannel>       Channels;
};

#endif
