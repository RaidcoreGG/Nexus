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

namespace GUI
{
	namespace QuickAccess
	{
		extern float Opacity;

		extern std::mutex Mutex;
		extern std::map<std::string, Shortcut> Registry;
		extern std::map<std::string, QUICKACCESS_SHORTCUTRENDERCALLBACK> RegistrySimple;

		extern std::thread AnimationThread;
		extern bool IsAnimating;
		extern bool IsFadingIn;
		extern bool IsHovering;

		void Render();

		void AddShortcut(std::string aIdentifier, std::string aTextureIdentifier, std::string aTextureHoverIdentifier, std::string aKeybindIdentifier, std::string aTooltipText);
		void RemoveShortcut(std::string aIdentifier);

		void AddSimpleShortcut(std::string aIdentifier, QUICKACCESS_SHORTCUTRENDERCALLBACK aShortcutRenderCallback);
		void RemoveSimpleShortcut(std::string aIdentifier);
	}
}

#endif