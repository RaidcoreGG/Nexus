#ifndef GUI_QUICKACCESS_H
#define GUI_QUICKACCESS_H

#include <mutex>
#include <map>
#include <string>

#include "GUI/FuncDefs.h"

#include "Shortcut.h"
#include "EQAPosition.h"
#include "EQAVisibility.h"

#include "imgui/imgui.h"
#include "imgui/imgui_extensions.h"

namespace GUI
{
	namespace QuickAccess
	{
		extern float Opacity;

		extern std::mutex								Mutex;
		extern std::map<std::string, Shortcut>			Registry;
		extern std::map<std::string, SimpleShortcut>	OrphanedCallbacks;

		extern std::thread		AnimationThread;
		extern bool				IsAnimating;
		extern bool				IsFadingIn;
		extern bool				IsHovering;

		extern bool				VerticalLayout;
		extern EQAVisibility	Visibility;
		extern EQAPosition		Location;
		extern ImVec2			Offset;

		extern Texture*			IconNotification;

		void Render();
		void RenderContextMenu(const std::string& aIdentifier, const Shortcut& aShortcut, bool* aIsActive);

		void AddShortcut(const char* aIdentifier, const char* aTextureIdentifier, const char* aTextureHoverIdentifier, const char* aInputBindIdentifier, const char* aTooltipText);
		void RemoveShortcut(const char* aIdentifier);
		void NotifyShortcut(const char* aIdentifier);
		void SetNotificationShortcut(const char* aIdentifier, bool aState);

		void AddSimpleShortcut(const char* aIdentifier, GUI_RENDER aShortcutRenderCallback);
		void AddSimpleShortcut2(const char* aIdentifier, const char* aTargetShortcutIdentifier, GUI_RENDER aShortcutRenderCallback);
		void RemoveSimpleShortcut(const char* aIdentifier);

		std::string EQAVisibilityToString(EQAVisibility aQAVisibility);
		std::string EQAPositionToString(EQAPosition aQAPosition);

		int Verify(void* aStartAddress, void* aEndAddress);
	}
}

#endif