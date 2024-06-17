#include "CEULAModal.h"

#include <Windows.h>
#include <shellapi.h>

#include "Renderer.h"
#include "Shared.h"
#include "Index.h"
#include "Consts.h"

#include "GUI/GUI.h"
#include "Services/Settings/Settings.h"
#include "Events/EventHandler.h"
#include "Services/Textures/TextureLoader.h"

#include "imgui/imgui.h"
#include "imgui/imgui_extensions.h"

#include "resource.h"

namespace GUI
{
	CEULAModal::CEULAModal()
	{
		Name = "CEULAModal";
	}

	void CEULAModal::Render()
	{
		if (!Background) { Background = TextureService->GetOrCreate("TEX_EULA_BACKGROUND", RES_TEX_EULA_BACKGROUND, NexusHandle); }
		if (!TitleBar) { TitleBar = TextureService->GetOrCreate("TEX_EULA_TITLEBAR", RES_TEX_EULA_TITLEBAR, NexusHandle); }
		if (!TitleBarHover) { TitleBarHover = TextureService->GetOrCreate("TEX_EULA_TITLEBAR_HOVER", RES_TEX_EULA_TITLEBAR_HOVER, NexusHandle); }
		if (!TitleBarEnd) { TitleBarEnd = TextureService->GetOrCreate("TEX_TITLEBAREND", RES_TEX_TITLEBAREND, NexusHandle); }
		if (!TitleBarEndHover) { TitleBarEndHover = TextureService->GetOrCreate("TEX_TITLEBAREND_HOVER", RES_TEX_TITLEBAREND_HOVER, NexusHandle); }
		if (!BtnClose) { BtnClose = TextureService->GetOrCreate("TEX_BTNCLOSE", RES_TEX_BTNCLOSE, NexusHandle); }
		if (!BtnCloseHover) { BtnCloseHover = TextureService->GetOrCreate("TEX_BTNCLOSE_HOVER", RES_TEX_BTNCLOSE_HOVER, NexusHandle); }

		if (!(
			Background &&
			TitleBar && TitleBarHover &&
			TitleBarEnd && TitleBarEndHover &&
			BtnClose && BtnCloseHover
			))
		{
			return;
		}

		ImGui::PushFont(FontUI);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 4));
		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));

		ImGui::OpenPopup("Legal Agreement", ImGuiPopupFlags_AnyPopupLevel);
		ImVec2 center(Renderer::Width * 0.5f, Renderer::Height * 0.5f);
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(windowWidth * Renderer::Scaling, windowHeight * Renderer::Scaling));
		if (ImGui::BeginPopupModal("Legal Agreement", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | (TitleBarControlled ? 0 : ImGuiWindowFlags_NoMove)))
		{
			float btnHeight = 22.0f * Renderer::Scaling;

			ImGui::SetCursorPos(ImVec2(0, 0));
			ImGui::Image(Background->Resource, ImVec2(windowWidth * Renderer::Scaling, windowHeight * Renderer::Scaling));

			ImGui::SetCursorPos(ImVec2(28.0f, 8.0f + (64.0f * Renderer::Scaling)));

			ImGui::BeginChild("##eulacontainer", ImVec2(ImVec2(contentWidth * Renderer::Scaling, (contentHeight * Renderer::Scaling))));

			bool close = false;

			ImGui::TextWrapped("This is an unofficial library and Raidcore is in no way associated with ArenaNet nor with any of its partners. Modifying Guild Wars 2 through any third party software is not supported by ArenaNet nor by any of its partners.");

			ImGui::TextWrapped("By using this software you are agreeing to the terms and conditions as laid out on:");

			if (ImGui::TextURL("https://raidcore.gg/Legal", false, false))
			{
				ShellExecuteA(0, 0, "https://raidcore.gg/Legal", 0, 0, SW_SHOW);
			}

			ImGui::Text("If you do not agree to these terms, do not use the software.");

			ImGui::TextWrapped("By clicking \"I do NOT agree\" your game will close and Nexus will attempt to uninstall.");
			ImGui::TextWrapped("If you see this prompt again after restarting, you will have to manually remove \"%s\" while the game is closed.", Index::F_HOST_DLL.string().c_str());

			ImVec2 remainingPos = ImGui::GetCursorPos();
			float remainingHeight = remainingPos.y;
			float y = (remainingHeight - btnHeight) / 2.0f;

			float btnWidth = ImGui::CalcTextSize("I do NOT agree").x + 16.0f;

			ImGui::SetCursorPos(ImVec2((contentWidth * Renderer::Scaling / 2.0f) - 4.0f - btnWidth + remainingPos.x, y + remainingPos.y));
			if (ImGui::GW2::Button("I agree", ImVec2(btnWidth, btnHeight)))
			{
				HasAcceptedEULA = true;
				Settings::Settings[OPT_ACCEPTEULA] = true;
				Settings::Save();
				close = true;
			}
			ImGui::SameLine();
			ImGui::SetCursorPos(ImVec2((contentWidth * Renderer::Scaling / 2.0f) + 4.0f + remainingPos.x, y + remainingPos.y));
			if (ImGui::GW2::Button("I do NOT agree", ImVec2(btnWidth, btnHeight)))
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

				exit(0);
			}

			if (close)
			{
				ImGui::CloseCurrentPopup();
				delete this;
				GUI::EULAWindow = nullptr;
			}

			ImGui::EndChild();

			ImGui::SetCursorPos(ImVec2(0, 0));

			ImGui::Image(TitleBarControlled ? TitleBarHover->Resource : TitleBar->Resource, ImVec2(Renderer::Scaling * 600.0f, Renderer::Scaling * 64.0f));
			bool barHov = ImGui::IsItemHovered();

			ImGui::SetCursorPos(ImVec2((windowWidth * Renderer::Scaling) - (Renderer::Scaling * 128.0f), 0));
			ImGui::Image(TitleBarControlled ? TitleBarEndHover->Resource : TitleBarEnd->Resource, ImVec2(Renderer::Scaling * 128.0f, Renderer::Scaling * 64.0f));
			bool endHov = ImGui::IsItemHovered();

			TitleBarControlled = barHov || endHov;

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

			ImGui::SetCursorPos(ImVec2(((windowWidth - 20.0f) * Renderer::Scaling) - (Renderer::Scaling * 32.0f), 15.0f * Renderer::Scaling));
			if (ImGui::ImageButton(CloseHovered ? BtnCloseHover->Resource : BtnClose->Resource, ImVec2(Renderer::Scaling * 32.0f, Renderer::Scaling * 32.0f)))
			{
				Visible = false;
			}
			CloseHovered = ImGui::IsItemHovered();

			ImGui::PopStyleColor(3);
			ImGui::PopStyleVar();

			ImGui::PushFont(FontBig);
			ImGui::SetCursorPos(ImVec2(28.0f, 20.0f * Renderer::Scaling));
			ImGui::TextColored(ImVec4(1.0f, .933f, .733f, 1.0f), Language->Translate("Legal Agreement"));
			ImGui::PopFont();

			ImGui::EndPopup();
		}

		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar(6);
		ImGui::PopFont();
	}
}