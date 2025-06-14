///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  MouseResetFix.h
/// Description  :  Fix for the cursor reseting to (0,0). Fix for the cursor moving while hidden.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef MOUSERESETFIX_H
#define MOUSERESETFIX_H

#include <windows.h>

#include "Core/Context.h"
#include "Core/Preferences/PrefConst.h"

///----------------------------------------------------------------------------------------------------
/// (WndProc) MouseResetFix:
/// 	Returns 0 if message was processed or non-zero, if it should be passed to the next callback.
///----------------------------------------------------------------------------------------------------
inline UINT MouseResetFix(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CContext*  s_Context     = CContext::GetContext();
	static CSettings* s_SettingsCtx = s_Context->GetSettingsCtx();

	bool lockCursor  = s_SettingsCtx ? s_SettingsCtx->Get<bool>(OPT_CAMCTRL_LOCKCURSOR, false)  : false;
	bool resetCursor = s_SettingsCtx ? s_SettingsCtx->Get<bool>(OPT_CAMCTRL_RESETCURSOR, false) : false;

	static bool s_IsConfining = false;
	static POINT s_LastPos = {};

	CURSORINFO ci{};
	ci.cbSize = sizeof(CURSORINFO);
	GetCursorInfo(&ci);

	/* Store last visible pos on first click. */
	switch (uMsg)
	{
		case WM_LBUTTONDOWN:
		{
			if (ci.flags != 0 && (wParam & MK_RBUTTON) != MK_RBUTTON)
			{
				s_LastPos = ci.ptScreenPos;
			}
			break;
		}
		case WM_RBUTTONDOWN:
		{
			if (ci.flags != 0 && (wParam & MK_LBUTTON) != MK_LBUTTON)
			{
				s_LastPos = ci.ptScreenPos;
			}
			break;
		}
	}

	switch (uMsg)
	{
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MOUSEMOVE:
		{
			if (s_IsConfining && (ci.flags != 0))
			{
				if (lockCursor)
				{
					ClipCursor(NULL);
				}

				if (resetCursor)
				{
					SetCursorPos(s_LastPos.x, s_LastPos.y);
				}

				s_IsConfining = false;
			}
			else if (!s_IsConfining && (ci.flags == 0) && ((wParam & MK_LBUTTON) || (wParam & MK_RBUTTON)))
			{
				RECT rect{
					s_LastPos.x,
					s_LastPos.y,
					s_LastPos.x + 1,
					s_LastPos.y + 1
				};

				if (lockCursor)
				{
					ClipCursor(&rect);
				}

				s_IsConfining = true;
			}

			break;
		}
		case WM_ACTIVATEAPP:
		{
			if (s_IsConfining)
			{
				if (lockCursor)
				{
					ClipCursor(NULL);
				}

				if (resetCursor)
				{
					SetCursorPos(s_LastPos.x, s_LastPos.y);
				}

				s_IsConfining = false;
			}

			break;
		}
	}

	/* Never intercept input. */
	return 1;
}

#endif
