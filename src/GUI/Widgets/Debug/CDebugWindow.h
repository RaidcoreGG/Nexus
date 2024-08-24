#ifndef GUI_DEBUGWINDOW_H
#define GUI_DEBUGWINDOW_H

#include <string>

#include "UI/Controls/CtlWindow.h"
#include "GUI/Widgets/Overlay/CMumbleOverlay.h"

#include "imgui/imgui_memory_editor.h"

namespace GUI
{
	class CDebugWindow : public IWindow
	{
		public:
		CDebugWindow(std::string aName);
		~CDebugWindow() = default;

		void Render();

		CMumbleOverlay* MumbleWindow;
		ImGui::MemoryEditor MemoryViewer;
		bool IsMetricsWindowVisible;

		private:
		void DbgEventsTab();
		void DbgInputBindsTab();
		void DbgDataLinkTab();
		void DbgTexturesTab();
		void DbgShortcutsTab();
		void DbgLoaderTab();
		void DbgAPITab();
		void DbgFontsTab();
	};
}

#endif