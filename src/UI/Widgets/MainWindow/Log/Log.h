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
#include <mutex>

#include "imgui/imgui.h"

#include "Services/Logging/LogBase.h"
#include "Services/Logging/LogMsg.h"
#include "UI/Controls/CtlSubWindow.h"

class CLogWindow : public virtual ISubWindow, public virtual ILogger
{
	public:
	CLogWindow();
	void RenderContent() override;

	///----------------------------------------------------------------------------------------------------
	/// MsgProc:
	/// 	Message processing function.
	///----------------------------------------------------------------------------------------------------
	void MsgProc(const LogMsg_t* aLogEntry) override;

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
		const LogMsg_t*          Entry = nullptr;
		std::vector<MessagePart> Parts;
	};

	struct LogChannel
	{
		bool        IsSelected;
		std::string Name;
	};

	int                           MaxShownCount     = 400;
	ELogLevel                     FilterLevel       = ELogLevel::ALL;
	bool                          SelectedLevelOnly = false;

	std::mutex                    Mutex;
	std::vector<DisplayLogEntry*> LogEntries;
	std::vector<LogChannel>       Channels;
};

#endif
