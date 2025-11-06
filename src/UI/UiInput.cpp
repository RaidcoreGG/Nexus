///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  UiInput.cpp
/// Description  :  Contains the input handling for the User Interface.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "UiInput.h"

#include <cstdint>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "Core/Preferences/PrefConst.h"

CUiInput::CUiInput(CSettings* aSettings)
{
	this->Settings = aSettings;

	this->RequiredModifiers = this->Settings->Get<EModifiers>(OPT_UI_MODS, EModifiers::None);
	this->FilterClicks      = this->Settings->Get<bool>(OPT_UI_CLICK_MODSONLY, false);

	this->Settings->Subscribe<EModifiers>(OPT_UI_MODS, [&](EModifiers aModifiers)
	{
		this->RequiredModifiers = aModifiers;
	});

	this->Settings->Subscribe<bool>(OPT_UI_CLICK_MODSONLY, [&](bool aFilterClicks)
	{
		this->FilterClicks = aFilterClicks;
	});
}

CUiInput::~CUiInput()
{
	
}

UINT CUiInput::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ImGuiIO& io = ImGui::GetIO();

	EModifiers activeModifiers = EModifiers::None;
	if (GetAsyncKeyState(VK_MENU))
	{
		activeModifiers |= EModifiers::Alt;
	}
	if (GetAsyncKeyState(VK_CONTROL))
	{
		activeModifiers |= EModifiers::Ctrl;
	}
	if (GetAsyncKeyState(VK_SHIFT))
	{
		activeModifiers |= EModifiers::Shift;
	}

	bool reqModsDown = (this->RequiredModifiers & activeModifiers) == this->RequiredModifiers;

	switch (uMsg)
	{
		/* Don't relay mouse movement to ImGui, if the cursor is hidden. */
		case WM_MOUSEMOVE:
		{
			if (!Inputs::IsCursorHidden())
			{
				io.MousePos = ImVec2(static_cast<float>(LOWORD(lParam)), static_cast<float>(HIWORD(lParam)));
			}
			break;
		}

		/* LMB and RMB. */
		case WM_LBUTTONDBLCLK: [[fallthrough]];
		case WM_LBUTTONDOWN:
		{
			if (io.WantCaptureMouse && !Inputs::IsCursorHidden())
			{
				if (!this->FilterClicks)
				{
					io.MouseDown[0] = true;
					return 0;
				}
				else if (this->FilterClicks && reqModsDown)
				{
					io.MouseDown[0] = true;
					return 0;
				}
				break;
			}
			else //if (!io.WantCaptureMouse)
			{
				ImGui::ClearActiveID();
			}
			break;
		}
		case WM_RBUTTONDBLCLK: [[fallthrough]];
		case WM_RBUTTONDOWN:
		{
			if (io.WantCaptureMouse && !Inputs::IsCursorHidden())
			{
				if (!this->FilterClicks)
				{
					io.MouseDown[1] = true;
					return 0;
				}
				else if (this->FilterClicks && reqModsDown)
				{
					io.MouseDown[1] = true;
					return 0;
				}
				break;
			}
			break;
		}

		/* Always pass through LMB and RMB releases. */
		case WM_LBUTTONUP:
		{
			io.MouseDown[0] = false;
			break;
		}
		case WM_RBUTTONUP:
		{
			io.MouseDown[1] = false;
			break;
		}

		/* Scrollwheel only when ImGui wants to capture and the cursor isn't hidden. */
		case WM_MOUSEWHEEL:
		{
			if (io.WantCaptureMouse && !Inputs::IsCursorHidden())
			{
				io.MouseWheel += static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / static_cast<float>(WHEEL_DELTA);
				return 0;
			}
			break;
		}
		case WM_MOUSEHWHEEL:
		{
			if (io.WantCaptureMouse && !Inputs::IsCursorHidden())
			{
				io.MouseWheelH += static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / static_cast<float>(WHEEL_DELTA);
				return 0;
			}
			break;
		}

		/* Keys. */
		case WM_KEYDOWN: [[fallthrough]];
		case WM_SYSKEYDOWN:
		{
			if (io.WantTextInput)
			{
				if (wParam < 256)
				{
					io.KeysDown[wParam] = true;
				}
				return 0;
			}
			break;
		}
		case WM_KEYUP: [[fallthrough]];
		case WM_SYSKEYUP:
		{
			if (wParam < 256)
			{
				io.KeysDown[wParam] = false;
			}
			break;
		}

		/* Text input. */
		case WM_CHAR:
		{
			if (io.WantTextInput)
			{
				// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
				if (wParam > 0 && wParam < 0x10000)
				{
					io.AddInputCharacterUTF16(static_cast<uint16_t>(wParam));
				}
				return 0;
			}
			break;
		}
	}

	return uMsg;
}
