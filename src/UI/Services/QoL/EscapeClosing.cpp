///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  EscapeClosing.cpp
/// Description  :  Contains the functionality to close windows on escape.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------
#include "EscapeClosing.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "Consts.h"
#include "Context.h"
#include "Engine/Settings/Settings.h"
#include "Util/Inputs.h"

CEscapeClosing::CEscapeClosing()
{
	CContext* ctx = CContext::GetContext();
	CSettings* settingsCtx = ctx->GetSettingsCtx();

	this->Enabled = settingsCtx->Get<bool>(OPT_CLOSEESCAPE, true);
}

UINT CEscapeClosing::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!this->Enabled)
	{
		return uMsg;
	}

	if (uMsg == WM_KEYDOWN && wParam == VK_ESCAPE)
	{
		KeystrokeMessageFlags keylp = LParamToKMF(lParam);

		if (!keylp.PreviousKeyState)
		{
			ImVector<ImGuiWindow*> windows = ImGui::GetCurrentContext()->Windows;

			for (int i = windows.Size - 1; i > 0; i--)
			{
				for (auto& [wndName, boolptr] : this->Registry)
				{
					if (strcmp(windows[i]->Name, wndName.c_str()) == 0 && *boolptr)
					{
						*boolptr = false;
						return 0;
					}
				}
			}
		}
	}

	return uMsg;
}

void CEscapeClosing::Register(const char* aWindowName, bool* aIsVisible)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	this->Registry[aWindowName] = aIsVisible;
}

void CEscapeClosing::Deregister(const char* aWindowName)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	this->Registry.erase(aWindowName);
}

void CEscapeClosing::Deregister(bool* aIsVisible)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = std::find_if(this->Registry.begin(), this->Registry.end(), [aIsVisible](std::pair<std::string, bool*> entry) { return  aIsVisible == entry.second; });
	
	if (it != this->Registry.end())
	{
		this->Registry.erase(it);
	}
}

int CEscapeClosing::Verify(void* aStartAddress, void* aEndAddress)
{
	int refCounter = 0;

	const std::lock_guard<std::mutex> lock(this->Mutex);

	for (auto& [windowname, boolptr] : this->Registry)
	{
		if (boolptr >= aStartAddress && boolptr <= aEndAddress)
		{
			this->Registry.erase(windowname);
			refCounter++;
		}
	}

	return refCounter;
}
