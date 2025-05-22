///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IbCapture.cpp
/// Description  :  Provides functions for capturing InputBinds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "IbCapture.h"

#include "Util/Inputs.h"

bool CInputBindCapture::StartCapturing()
{
	/* Already capturing. */
	if (this->IsCapturing)
	{
		return false;
	}

	/* Reset the previously captured bind. */
	this->Capture = InputBind{};

	/* Set the state. */
	this->IsCapturing = true;

	return true;
}

void CInputBindCapture::EndCapturing()
{
	/* Set the state. */
	this->IsCapturing = false;
}

InputBind CInputBindCapture::GetCapture() const
{
	return this->Capture;
}

bool CInputBindCapture::ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!this->IsCapturing)
	{
		return false;
	}

	bool isModifier = false;

	/* Preprocess modifiers. */
	switch (uMsg)
	{
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			switch (wParam)
			{
				case VK_MENU:
				{
					isModifier = true;
					this->IsAltHeld = true;
					break;
				}
				case VK_CONTROL:
				{
					isModifier = true;
					this->IsCtrlHeld = true;
					break;
				}
				case VK_SHIFT:
				{
					isModifier = true;
					this->IsShiftHeld = true;
					break;
				}
			}

			break;
		}

		case WM_SYSKEYUP:
		case WM_KEYUP:
		{
			switch (wParam)
			{
				case VK_MENU:
				{
					isModifier = true;
					this->IsAltHeld = false;
					break;
				}
				case VK_CONTROL:
				{
					isModifier = true;
					this->IsCtrlHeld = false;
					break;
				}
				case VK_SHIFT:
				{
					isModifier = true;
					this->IsShiftHeld = false;
					break;
				}
			}

			break;
		}
	}

	/* actual input processing */
	switch (uMsg)
	{
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			if (wParam > 255) { break; }

			KeystrokeMessageFlags keylp = LParamToKMF(lParam);

			/* If it was already previously set, this is a repeat. */
			if (keylp.PreviousKeyState)
			{
				/* Do not process futher, but also do not override the actual last input. */
				return true;
			}

			/* If it's a modifier, ignore all other set modifiers, to allow for the last pressed one to be a standalone bind. */
			if (isModifier)
			{
				this->Capture.Alt = false;
				this->Capture.Ctrl = false;
				this->Capture.Shift = false;
			}
			else
			{
				this->Capture.Alt = this->IsAltHeld;
				this->Capture.Ctrl = this->IsCtrlHeld;
				this->Capture.Shift = this->IsShiftHeld;
			}

			/* Key button -> Key bind */
			this->Capture.Device = EInputDevice::Keyboard;
			this->Capture.Code = keylp.GetScanCode();

			/* Signal to not process further. */
			return true;
		}

		case WM_SYSKEYUP:
		case WM_KEYUP:
		{
			/* Let key releases be processed further. */
			return false;
		}

		case WM_MBUTTONDOWN:
		case WM_XBUTTONDOWN:
		{
			this->Capture.Alt   = this->IsAltHeld;
			this->Capture.Ctrl  = this->IsCtrlHeld;
			this->Capture.Shift = this->IsShiftHeld;

			/* Mouse button -> Mouse bind */
			this->Capture.Device = EInputDevice::Mouse;

			if (uMsg == WM_MBUTTONDOWN)
			{
				this->Capture.Code = (unsigned short)EMouseButtons::MMB;
			} /* else it's implicitly an XBUTTON */
			else if (HIWORD(wParam) == XBUTTON1)
			{
				this->Capture.Code = (unsigned short)EMouseButtons::M4;
			}
			else if (HIWORD(wParam) == XBUTTON2)
			{
				this->Capture.Code = (unsigned short)EMouseButtons::M5;
			}

			/* Signal to not process further. */
			return true;
		}
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
		{
			/* Let mouse releases be processed further. */
			return false;
		}
	}

	/* Let any other input be processed further. */
	return false;
}
