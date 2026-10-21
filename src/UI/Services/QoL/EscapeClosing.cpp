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

#include "Core/Context.h"
#include "Core/Preferences/PrefConst.h"
#include "Core/Preferences/PrefContext.h"
#include "Util/Inputs.h"

CEscapeClosing::CEscapeClosing() : IRefCleaner("EscapeClosing")
{
	CContext* ctx = CContext::GetContext();
	CSettings* settingsctx = ctx->GetSettingsCtx();

	settingsctx->Subscribe<bool>(OPT_CLOSEESCAPE, [&](bool aNewValue)
	{
		this->Enabled = aNewValue;
	});
}

CEscapeClosing::~CEscapeClosing()
{
}

UINT CEscapeClosing::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!this->Enabled)
	{
		return uMsg;
	}

	if (!(uMsg == WM_KEYDOWN && wParam == VK_ESCAPE))
	{
		return uMsg;
	}

	KeystrokeMessageFlags& keylp = LParamToKMF(lParam);

	/* If this bit is 1, the key was previously down -> We only react on change, not repeat.*/
	if (keylp.PreviousKeyState)
	{
		return uMsg;
	}

	ImVector<ImGuiWindow*> windows = ImGui::GetCurrentContext()->Windows;

	/* Iterate over registered window pointers, since they might even be all hidden. */
	for (auto& [wndName, boolptr] : this->Registry)
	{
		if (*boolptr == false) { continue; }

		/* Iterate backwards, as the last one is the topmost. */
		for (int i = windows.Size - 1; i > 0; i--)
		{
			/* If the registered windowname matches the imgui window name -> hide.*/
			if (strcmp(windows[i]->Name, wndName.c_str()) == 0)
			{
				*boolptr = false;
				return 0;
			}
		}
	}

	return uMsg;
}

void CEscapeClosing::Register(const char* aWindowName, bool* aIsVisible)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(aWindowName);

	if (it == this->Registry.end())
	{
		this->Registry.emplace(aWindowName, aIsVisible);
	}
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

uint32_t CEscapeClosing::CleanupRefs(void* aStartAddress, void* aEndAddress)
{
	uint32_t refCounter = 0;

	const std::lock_guard<std::mutex> lock(this->Mutex);

	for (auto it = this->Registry.begin(); it != this->Registry.end();)
	{
		if (it->second >= aStartAddress && it->second <= aEndAddress)
		{
			it = this->Registry.erase(it);
			refCounter++;
		}
		else
		{
			it++;
		}
	}

	return refCounter;
}
