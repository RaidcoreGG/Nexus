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

	void InitialSetup();

	void ProcessKeybind(const wchar_t* aIdentifier);

	void SetScale(unsigned aScale);

	void Render();
	void RenderMenu();
	void AddWindow(IWindow* aWindowPtr);
}

#endif