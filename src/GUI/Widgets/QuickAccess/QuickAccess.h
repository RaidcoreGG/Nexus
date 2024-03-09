#ifndef GUI_QUICKACCESS_H
#define GUI_QUICKACCESS_H

#include <mutex>
#include <map>
#include <string>

#include "GUI/FuncDefs.h"

#include "Shortcut.h"
#include "EQAPosition.h"

#include "imgui.h"
#include "imgui_extensions.h"

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

		extern bool				AlwaysShow;
		extern bool				VerticalLayout;
		extern EQAPosition		Location;
		extern ImVec2			Offset;

		extern Texture*			IconNotification;

		void Render();

		void AddShortcut(const char* aIdentifier, const char* aTextureIdentifier, const char* aTextureHoverIdentifier, const char* aKeybindIdentifier, const char* aTooltipText);
		void RemoveShortcut(const char* aIdentifier);
		void NotifyShortcut(const char* aIdentifier);
		void SetNotificationShortcut(const char* aIdentifier, bool aState);

		void AddSimpleShortcut(const char* aIdentifier, GUI_RENDER aShortcutRenderCallback);
		void RemoveSimpleShortcut(const char* aIdentifier);

		int Verify(void* aStartAddress, void* aEndAddress);
	}
}

#endif