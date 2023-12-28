#ifndef GUI_H
#define GUI_H

#include <Windows.h>
#include <mutex>
#include <vector>
#include <map>

#include "FuncDefs.h"

#include "IWindow.h"
#include "EFontIdentifier.h"

#include "imgui/imgui.h"

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
	extern bool							CloseOnEscape;

	extern bool							IsUIVisible;

	extern bool							HasAcceptedEULA;
	extern bool							NotifyChangelog;

	void Initialize();
	void Shutdown();

	/* Returns 0 if message was processed. */
	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void Render();

	void AddWindow(IWindow* aWindowPtr);

	/* Registers the provided RenderCallback. */
	void Register(ERenderType aRenderType, GUI_RENDER aRenderCallback);
	/* Unregisters the provided RenderCallback. */
	void Unregister(GUI_RENDER aRenderCallback);

	/* Removes all subscribers that are within the provided address space. */
	int Verify(void* aStartAddress, void* aEndAddress);
}

#endif