///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IbCapture.cpp
/// Description  :  Provides functions for capturing InputBinds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "IbCapture.h"

#include "Util/Inputs.h"

namespace Engine::Input
{
	void CInputBindCapture::StartCapturing()
	{
		/* Already capturing. */
		if (this->IsCapturing)
		{
			return;
		}

		/* Reset the previously captured bind. */
		this->Capture = Bind_t{};

		/* Set the state. */
		this->IsCapturing = true;
	}

	void CInputBindCapture::EndCapturing()
	{
		/* Set the state. */
		this->IsCapturing = false;
	}

	Bind_t CInputBindCapture::GetCapture() const
	{
		return this->Capture;
	}

	bool CInputBindCapture::ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		bool isModifier = false;
		bool isCapturing = this->IsCapturing;

		/* Preprocess modifiers. */
		switch (uMsg)
		{
			case WM_ACTIVATEAPP:
			{
				this->ModifiersHeld = EModifiers::None;
				this->Capture.Modifiers = EModifiers::None;
				return false;
			}

			case WM_SYSKEYDOWN:
			case WM_KEYDOWN:
			{
				switch (wParam)
				{
					case VK_MENU:
					{
						isModifier = true;
						this->ModifiersHeld |= EModifiers::Alt;
						break;
					}
					case VK_CONTROL:
					{
						isModifier = true;
						this->ModifiersHeld = EModifiers::Ctrl;
						break;
					}
					case VK_SHIFT:
					{
						isModifier = true;
						this->ModifiersHeld = EModifiers::Shift;
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
						this->ModifiersHeld &= ~EModifiers::Alt;
						break;
					}
					case VK_CONTROL:
					{
						this->ModifiersHeld &= EModifiers::Ctrl;
						break;
					}
					case VK_SHIFT:
					{
						this->ModifiersHeld &= EModifiers::Shift;
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
					return false;
				}

				/* If it's a modifier, ignore all other set modifiers, to allow for the last pressed one to be a standalone bind. */
				if (isModifier)
				{
					this->Capture.Modifiers = EModifiers::None;
				}
				else
				{
					this->Capture.Modifiers = this->ModifiersHeld;
				}

				/* Key button -> Key bind */
				this->Capture.Device = EDevice::Keyboard;
				this->Capture.Code = keylp.GetScanCode();

				/* Signal to not process further, if capturing. */
				return isCapturing;
			}

			case WM_MBUTTONDOWN:
			{
				this->Capture.Modifiers = this->ModifiersHeld;

				/* Mouse button -> Mouse bind */
				this->Capture.Device = EDevice::Mouse;

				this->Capture.Code = (unsigned short)EMouseButtons::MMB;

				/* Signal to not process further, if capturing. */
				return isCapturing;
			}
			case WM_XBUTTONDOWN:
			{
				this->Capture.Modifiers = this->ModifiersHeld;

				/* Mouse button -> Mouse bind */
				this->Capture.Device = EDevice::Mouse;

				if (HIWORD(wParam) == XBUTTON1)
				{
					this->Capture.Code = (unsigned short)EMouseButtons::M4;
				}
				else if (HIWORD(wParam) == XBUTTON2)
				{
					this->Capture.Code = (unsigned short)EMouseButtons::M5;
				}

				/* Signal to not process further, if capturing. */
				return isCapturing;
			}
		}

		/* Let any other input be processed further. */
		return false;
	}

	const Bind_t& CInputBindCapture::GetCaptureRef() const
	{
		return this->Capture;
	}
}
