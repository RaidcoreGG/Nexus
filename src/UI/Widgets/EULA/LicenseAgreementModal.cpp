///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LicenseAgreementModal.cpp
/// Description  :  Modal for license agreement.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "LicenseAgreementModal.h"

#include <windows.h>
#include <shellapi.h>

#include "imgui.h"
#include "imgui_extensions.h"

#include "Index.h"
#include "Context.h"
#include "Consts.h"

CLicenseAgreementModal::CLicenseAgreementModal()
{
	this->SetID("LicenseAgreementModal");
	this->SetDisplayName("License Agreement");
}

void CLicenseAgreementModal::RenderContent()
{
	ImGui::TextWrapped("This is an unofficial library and Raidcore is in no way associated with ArenaNet nor with any of its partners.");
	ImGui::TextWrapped("Modifying Guild Wars 2 through any third party software is not supported by ArenaNet.");

	ImGui::NewLine();

	ImGui::TextWrapped("You are using this program at your own risk.");

	ImGui::NewLine();

	ImGui::TextWrapped("By using this software you are agreeing to the terms and conditions as laid out on:");

	if (ImGui::TextURL("https://raidcore.gg/Legal", false, false))
	{
		ShellExecuteA(0, 0, "https://raidcore.gg/Legal", 0, 0, SW_SHOW);
	}

	ImGui::Text("If you do not agree to these terms or do not understand them, do not use the software.");

	ImGui::NewLine();

	ImGui::TextWrapped("By clicking \"I do NOT agree\" your game will close and Nexus will attempt to uninstall.");
	ImGui::TextWrapped("If you see this prompt again after restarting the game, you will have to manually remove");
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 0.f, 1.f));
	ImGui::TextWrapped("%s", Index::F_HOST_DLL.string().c_str());
	ImGui::PopStyleColor();
	ImGui::TextWrapped("while the game is closed.");

	ImGuiStyle& style = ImGui::GetStyle();
	ImVec2 pos = ImGui::GetCursorPos();

	float btnWidth = ImGui::CalcTextSize("I do NOT agree").x + (style.FramePadding.x * 2);
	float wndWidth = ImGui::GetWindowWidth();

	if (wndWidth > (btnWidth * 2) + style.ItemSpacing.x + (style.WindowPadding.x * 2))
	{
		btnWidth = wndWidth - style.ItemSpacing.x - (style.WindowPadding.x * 2);
		btnWidth /= 2;
	}

	if (ImGui::Button("I agree", ImVec2(btnWidth, 0)))
	{
		this->SetResult(EModalResult::OK);
	}

	ImGui::SameLine();

	if (ImGui::Button("I do NOT agree", ImVec2(btnWidth, 0)))
	{
		this->SetResult(EModalResult::Cancel);
	}
}

void CLicenseAgreementModal::OnClosing()
{
	CContext*  ctx         = CContext::GetContext();
	CSettings* settingsctx = ctx->GetSettingsCtx();

	switch (this->GetResult())
	{
		case EModalResult::OK:
		{
			settingsctx->Set(OPT_ACCEPTEULA, true);
			break;
		}
		case EModalResult::Cancel:
		{
			std::string strHost = Index::F_HOST_DLL.string();

			SHFILEOPSTRUCT fileOp{};
			fileOp.hwnd   = NULL;
			fileOp.wFunc  = FO_DELETE;
			fileOp.pFrom  = strHost.c_str();
			fileOp.pTo    = NULL;
			fileOp.fFlags = FOF_ALLOWUNDO | FOF_NOERRORUI | FOF_NOCONFIRMATION | FOF_SILENT;
			int result = SHFileOperationA(&fileOp);
			break;
		}
	}
}
