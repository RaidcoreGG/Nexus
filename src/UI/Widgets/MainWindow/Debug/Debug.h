///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Debug.h
/// Description  :  Contains the content of the debug window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef MAINWINDOW_DEBUG_H
#define MAINWINDOW_DEBUG_H

#include "imgui/imgui_memory_editor.h"

#include "UI/Controls/CtlSubWindow.h"
#include "UI/Overlay/MumbleInspector/MumbleInspector.h"

class CDebugWindow : public ISubWindow
{
	public:
	CDebugWindow();
	void RenderContent() override;
	void RenderSubWindows() override;

	void ToggleMumbleOverlay();

	private:
	CMumbleOverlay*     MumbleWindow           = new CMumbleOverlay();
	bool                IsMetricsWindowVisible = false;

	ImGui::MemoryEditor MemoryViewer;
	void*               MV_Ptr                 = nullptr;
	size_t              MV_Size                = 0;

	void TabEvents();
	void TabInputBinds();
	void TabDataLink();
	void TabTextures();
	void TabQuickAccess();
	void TabLoader();
	void TabFonts();
};

#endif
