#ifndef GUI_H
#define GUI_H

#include <Windows.h>
#include <mutex>
#include <vector>
#include <map>

#include "FuncDefs.h"

#include "IWindow.h"
#include "EFontIdentifier.h"
#include "Fonts/FontManager.h"

#include "imgui/imgui.h"

#include "Services/Mumble/Definitions/Mumble.h"
#include "Loader/NexusLinkData.h"

#include "Widgets/Addons/CAddonsWindow.h"
#include "Widgets/Options/COptionsWindow.h"
#include "Widgets/Log/CLogWindow.h"
#include "Widgets/Debug/CDebugWindow.h"
#include "Widgets/About/CAboutBox.h"
#include "Widgets/EULA/CEULAModal.h"

#include "Inputs/Keybinds/Keybind.h"

namespace GUI
{
	extern CFontManager&				FontManager;

	extern CAddonsWindow*				AddonsWindow;
	extern COptionsWindow*				OptionsWindow;
	extern CLogWindow*					LogWindow;
	extern CDebugWindow*				DebugWindow;
	extern CAboutBox*					AboutWindow;
	extern CEULAModal*					EULAWindow;

	extern Mumble::Data*				MumbleLink;
	extern NexusLinkData*				NexusLink;

	extern std::mutex					Mutex;
	extern std::vector<GUI_RENDER>		RegistryPreRender;
	extern std::vector<GUI_RENDER>		RegistryRender;
	extern std::vector<GUI_RENDER>		RegistryPostRender;
	extern std::vector<GUI_RENDER>		RegistryOptionsRender;

	extern std::map<EFont, ImFont*>		FontIndex;
	extern float						FontSize;
	extern bool							CloseMenuAfterSelecting;
	extern bool							CloseOnEscape;
	extern bool							LinkArcDPSStyle;

	extern bool							IsUIVisible;

	extern bool							HasAcceptedEULA;
	extern bool							NotifyChangelog;

	extern bool							ShowAddonsWindowAfterDUU;

	void Initialize();
	void Shutdown();

	/* Returns 0 if message was processed. */
	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void Render();

	void ImportArcDPSStyle();

	void Rescale();

	/* Registers the provided RenderCallback. */
	void Register(ERenderType aRenderType, GUI_RENDER aRenderCallback);
	/* Deregisters the provided RenderCallback. */
	void Deregister(GUI_RENDER aRenderCallback);

	/* Removes all subscribers that are within the provided address space. */
	int Verify(void* aStartAddress, void* aEndAddress);
}

#endif