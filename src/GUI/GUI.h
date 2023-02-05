#ifndef GUI_H
#define GUI_H

#include "../imgui/imgui.h"
#include "../imgui/imgui_extensions.h"

#include "Widgets/Addons/AddonsWindow.h"
#include "Widgets/Keybinds/KeybindsWindow.h"
#include "Widgets/Log/LogWindow.h"
#include "Widgets/Overlay/MumbleOverlay.h"
#include "Widgets/Debug/DebugWindow.h"
#include "Widgets/About/AboutBox.h"

namespace GUI
{
	void Initialize();
	void Shutdown();

	bool WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void ProcessKeybind(std::string aIdentifier);

	void Render();
	void RenderMenu();
	void AddWindow(IWindow* aWindowPtr);
}

#endif