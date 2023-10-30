#ifndef GUI_QUICKACCESS_H
#define GUI_QUICKACCESS_H

#include <thread>
#include <map>
#include <string>
#include <mutex>

#include "../../../Shared.h"
#include "../../../Paths.h"
#include "../../../State.h"
#include "../../../Renderer.h"
#include "../../../Consts.h"

#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_extensions.h"

#include "../../../Textures/Texture.h"
#include "../../../Textures/TextureLoader.h"
#include "../../../Keybinds/KeybindHandler.h"

#include "FuncDefs.h"
#include "Shortcut.h"
#include "EQAPosition.h"

namespace GUI
{
	namespace QuickAccess
	{
		extern float Opacity;

		extern std::mutex							Mutex;
		extern std::map<std::string, Shortcut>		Registry;
		extern std::map<std::string, GUI_RENDER>	RegistrySimple;

		extern std::thread		AnimationThread;
		extern bool				IsAnimating;
		extern bool				IsFadingIn;
		extern bool				IsHovering;

		extern bool				VerticalLayout;
		extern EQAPosition		Location;
		extern ImVec2			Offset;

		void Render();

		void AddShortcut(const char* aIdentifier, const char* aTextureIdentifier, const char* aTextureHoverIdentifier, const char* aKeybindIdentifier, const char* aTooltipText);
		void RemoveShortcut(const char* aIdentifier);

		void AddSimpleShortcut(const char* aIdentifier, GUI_RENDER aShortcutRenderCallback);
		void RemoveSimpleShortcut(const char* aIdentifier);

		int Verify(void* aStartAddress, void* aEndAddress);
	}
}

#endif