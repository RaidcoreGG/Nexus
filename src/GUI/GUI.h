#ifndef GUI_H
#define GUI_H

#include "../imgui/imgui.h"
#include "../imgui/imgui_extensions.h"

#include "Widgets/Addons/AddonsWindow.h"
//#include "Keybinds.h"
#include "Widgets/Log/LogWindow.h"
#include "Widgets/About/AboutBox.h"
#include "Widgets/Overlay/MumbleOverlay.h"

namespace GUI
{
	void Initialize();
	void Shutdown();

	bool WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void ProcessKeybind(const wchar_t* aIdentifier);

	void Render();
	void RenderMenu();
	void AddWindow(IWindow* aWindowPtr);
}

#endif