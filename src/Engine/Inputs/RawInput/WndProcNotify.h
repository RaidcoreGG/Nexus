///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  WndProcNotify.h
/// Description  :  Window message notification.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <windows.h>

#include "Engine/Notifier/Notifier.h"

constexpr int32_t WNDPROC_PRIO_LOADER     = 100;
constexpr int32_t WNDPROC_PRIO_UI         = 300;
constexpr int32_t WNDPROC_PRIO_INPUTBINDS = 400;

///----------------------------------------------------------------------------------------------------
/// IWndProcNotifyHandler Interface
///----------------------------------------------------------------------------------------------------
class IWndProcNotifyHandler : public INotifyHandler<IWndProcNotifyHandler>
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// OnWndProcNotify:
	/// 	Returns 0 if the message was consumed. The message may be modified for later subscribers and the game.
	///----------------------------------------------------------------------------------------------------
	virtual UINT OnWndProcNotify(HWND& hWnd, UINT& uMsg, WPARAM& wParam, LPARAM& lParam) = 0;
};

///----------------------------------------------------------------------------------------------------
/// IWndProcNotifier Interface
///----------------------------------------------------------------------------------------------------
class IWndProcNotifier : public INotifier<IWndProcNotifyHandler>
{
	protected:
	///----------------------------------------------------------------------------------------------------
	/// NotifyWndProc:
	/// 	Returns 0 if a subscriber consumed the message.
	///----------------------------------------------------------------------------------------------------
	UINT NotifyWndProc(HWND& hWnd, UINT& uMsg, WPARAM& wParam, LPARAM& lParam)
	{
		for (const auto& entry : this->Subscribers())
		{
			if (entry.Subscriber->OnWndProcNotify(hWnd, uMsg, wParam, lParam) == NOTIFY_CONSUMED)
			{
				return NOTIFY_CONSUMED;
			}
		}

		return 1;
	}
};
