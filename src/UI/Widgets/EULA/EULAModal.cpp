///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  EULAModal.cpp
/// Description  :  Contains the logic for the EULA popup modal.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "EULAModal.h"

#include <shellapi.h>

#include "imgui/imgui.h"
#include "imgui/imgui_extensions.h"

#include "Hooks.h"
#include "Index.h"
#include "Renderer.h"

CEULAModal::CEULAModal(HWND aWindowHandle, CLocalization* aLocalization)
{
	this->WindowHandle = aWindowHandle;
	this->Language = aLocalization;
}

CEULAModal::~CEULAModal()
{
	this->Language = nullptr;
}

bool CEULAModal::Render()
{
	bool accepted = false;
	bool close = false;

	ImGui::OpenPopup(this->Language->Translate("Legal Agreement"), ImGuiPopupFlags_AnyPopupLevel);
	ImVec2 center(Renderer::Width * 0.5f, Renderer::Height * 0.5f);
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal(this->Language->Translate("Legal Agreement"), 0, ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::TextWrapped("This is an unofficial library and Raidcore is in no way associated with ArenaNet nor with any of its partners.");
		ImGui::TextWrapped("Modifying Guild Wars 2 through any third party software is not supported by ArenaNet nor by any of its partners.");
		ImGui::TextWrapped("By using this software you are agreeing to the terms and conditions as laid out on:");

		if (ImGui::TextURL("https://raidcore.gg/Legal", false, false))
		{
			ShellExecuteA(0, 0, "https://raidcore.gg/Legal", 0, 0, SW_SHOW);
		}

		ImGui::Text("If you do not agree to these terms, do not use the software.");

		ImGui::TextWrapped("By clicking \"I do NOT agree\" your game will close and Nexus will attempt to uninstall.");
		ImGui::TextWrapped("If you see this prompt again after restarting, you will have to manually remove \"%s\" while the game is closed.", Index::F_HOST_DLL.string().c_str());

		ImGuiStyle& style = ImGui::GetStyle();
		ImVec2 pos = ImGui::GetCursorPos();

		float btnWidth = ImGui::CalcTextSize("I do NOT agree").x + (style.WindowPadding.x * 2);
		float width = ImGui::GetWindowWidth();

		ImGui::SetCursorPos(ImVec2((width / 2) - btnWidth - style.ItemSpacing.x, pos.y));
		if (ImGui::Button("I agree", ImVec2(btnWidth, 0)))
		{
			accepted = true;
			close = true;
		}

		ImGui::SetCursorPos(ImVec2((width / 2) + style.ItemSpacing.x, pos.y));
		if (ImGui::Button("I do NOT agree", ImVec2(btnWidth, 0)))
		{
			std::string strHost = Index::F_HOST_DLL.string();

			SHFILEOPSTRUCT fileOp;
			fileOp.hwnd = NULL;
			fileOp.wFunc = FO_DELETE;
			fileOp.pFrom = strHost.c_str();
			fileOp.pTo = NULL;
			fileOp.fFlags = FOF_ALLOWUNDO | FOF_NOERRORUI | FOF_NOCONFIRMATION | FOF_SILENT;
			int result = SHFileOperationA(&fileOp);

			close = true;

			/* inject close message into wndproc for clean shutdown */
			Hooks::WndProc(this->WindowHandle, WM_CLOSE, 0, 0);
		}

		if (close)
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	return accepted;
}
