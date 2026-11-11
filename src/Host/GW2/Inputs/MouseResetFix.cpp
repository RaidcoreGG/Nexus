///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  MouseResetFix.cpp
/// Description  :  Fix for the cursor reseting to (0,0). Fix for the cursor moving while hidden.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "MouseResetFix.h"

CMouseResetFix::CMouseResetFix()
{
	/* After the game binds, so game only messages arrive in the normal range. */
	this->Attach(WNDPROC_PRIO_MOUSEFIX);
}

CMouseResetFix::~CMouseResetFix()
{
	this->Detach();
}

UINT CMouseResetFix::OnWndProcNotify(HWND& hWnd, UINT& uMsg, WPARAM& wParam, LPARAM& lParam)
{
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
				this->LastPos = ci.ptScreenPos;
			}
			break;
		}
		case WM_RBUTTONDOWN:
		{
			if (ci.flags != 0 && (wParam & MK_LBUTTON) != MK_LBUTTON)
			{
				this->LastPos = ci.ptScreenPos;
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
			if (this->IsConfining && (ci.flags != 0))
			{
				this->Release();
			}
			else if (!this->IsConfining && (ci.flags == 0) && ((wParam & MK_LBUTTON) || (wParam & MK_RBUTTON)))
			{
				RECT rect{
					this->LastPos.x,
					this->LastPos.y,
					this->LastPos.x + 1,
					this->LastPos.y + 1
				};

				if (this->LockCursor.Get())
				{
					ClipCursor(&rect);
				}

				this->IsConfining = true;
			}

			break;
		}
		case WM_ACTIVATEAPP:
		{
			if (this->IsConfining)
			{
				this->Release();
			}

			break;
		}
	}

	return 1;
}

void CMouseResetFix::Release()
{
	if (this->LockCursor.Get())
	{
		ClipCursor(NULL);
	}

	if (this->ResetCursor.Get())
	{
		SetCursorPos(this->LastPos.x, this->LastPos.y);
	}

	this->IsConfining = false;
}
