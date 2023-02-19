#ifndef GUI_H
#define GUI_H

#include "../State.h"
#include "../Renderer.h"
#include "../Paths.h"
#include "../Shared.h"

#include "../Keybinds/KeybindHandler.h"
#include "../Loader/Loader.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_extensions.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui_impl_dx11.h"

#include "Widgets/Addons/AddonsWindow.h"
#include "Widgets/Keybinds/KeybindsWindow.h"
#include "Widgets/Log/LogWindow.h"
#include "Widgets/Overlay/MumbleOverlay.h"
#include "Widgets/Debug/DebugWindow.h"
#include "Widgets/About/AboutBox.h"

#include "../resource.h"

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