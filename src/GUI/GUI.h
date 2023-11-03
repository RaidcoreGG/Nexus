#ifndef GUI_H
#define GUI_H

#include <filesystem>
#include <map>
#include <mutex>
#include <vector>

#include "../State.h"
#include "../Renderer.h"
#include "../Paths.h"
#include "../Shared.h"
#include "../Consts.h"
#include "../Branch.h"

#include "../Keybinds/KeybindHandler.h"
#include "../Loader/Loader.h"
#include "../Settings/Settings.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_extensions.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui_impl_dx11.h"

#include "Widgets/Menu/Menu.h"
#include "Widgets/Addons/AddonsWindow.h"
#include "Widgets/Options/OptionsWindow.h"
#include "Widgets/Log/LogWindow.h"
#include "Widgets/Debug/DebugWindow.h"
#include "Widgets/About/AboutBox.h"
#include "Widgets/QuickAccess/QuickAccess.h"
#include "Widgets/Menu/MenuItem.h"

#include "../resource.h"
#include "../Textures/Texture.h"
#include "EFontIdentifier.h"

namespace GUI
{
	extern std::mutex					Mutex;
	extern std::vector<GUI_RENDER>		RegistryPreRender;
	extern std::vector<GUI_RENDER>		RegistryRender;
	extern std::vector<GUI_RENDER>		RegistryPostRender;
	extern std::vector<GUI_RENDER>		RegistryOptionsRender;
	extern std::vector<IWindow*>		Windows;
	extern std::map<EFont, ImFont*>		FontIndex;
	extern float						FontSize;
	extern bool							CloseMenuAfterSelecting;

	extern bool							IsUIVisible;

	void Initialize();
	void Shutdown();

	/* Returns 0 if message was processed. */
	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void Render();

	void AddWindow(IWindow* aWindowPtr);

	void ResizeFonts();

	/* Registers the provided RenderCallback. */
	void Register(ERenderType aRenderType, GUI_RENDER aRenderCallback);
	/* Unregisters the provided RenderCallback. */
	void Unregister(GUI_RENDER aRenderCallback);

	/* Removes all subscribers that are within the provided address space. */
	int Verify(void* aStartAddress, void* aEndAddress);
}

#endif