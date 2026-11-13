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

#include "imgui/imgui.h"
#include "imgui/imgui_extensions.h"

#include "Host/GW2/Context.h"
#include "Host/GW2/Index/Index.h"
#include "UI/UiContext.h"

CLicenseAgreementModal::CLicenseAgreementModal()
{
	this->SetID("LicenseAgreementModal");
	this->SetDisplayName(Lang::Eula::Title);
}

void CLicenseAgreementModal::RenderContent()
{
	ImGui::TextWrapped("%s", Lang::Eula::Unofficial.Text());
	ImGui::TextWrapped("%s", Lang::Eula::NotSupported.Text());

	ImGui::NewLine();

	ImGui::TextWrapped("%s", Lang::Eula::OwnRisk.Text());

	ImGui::NewLine();

	ImGui::TextWrapped("%s", Lang::Eula::AgreeToTerms.Text());

	if (ImGui::TextURL("https://raidcore.gg/Legal", false, false))
	{
		ShellExecuteA(0, 0, "https://raidcore.gg/Legal", 0, 0, SW_SHOW);
	}

	ImGui::TextWrapped("%s", Lang::Eula::DoNotUse.Text());

	ImGui::NewLine();

	ImGui::TextWrapped("%s", Lang::Eula::DisagreeUninstall.Text());
	ImGui::TextWrapped("%s", Lang::Eula::ManualRemove.Text());
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 0.f, 1.f));
	ImGui::TextWrapped("%s", Index(EPath::NexusDLL).string().c_str());
	ImGui::PopStyleColor();
	ImGui::TextWrapped("%s", Lang::Eula::WhileClosed.Text());

	ImGuiStyle& style = ImGui::GetStyle();
	ImVec2 pos = ImGui::GetCursorPos();

	float btnWidth = max(ImGui::CalcTextSize(Lang::Eula::Agree.Text()).x, ImGui::CalcTextSize(Lang::Eula::Disagree.Text()).x) + (style.FramePadding.x * 2);
	float wndWidth = ImGui::GetWindowWidth();

	if (wndWidth > (btnWidth * 2) + style.ItemSpacing.x + (style.WindowPadding.x * 2))
	{
		btnWidth = wndWidth - style.ItemSpacing.x - (style.WindowPadding.x * 2);
		btnWidth /= 2;
	}

	if (ImGui::Button(Lang::Eula::Agree.Text(), ImVec2(btnWidth, 0)))
	{
		this->SetResult(EModalResult::OK);
	}

	ImGui::SameLine();

	if (ImGui::Button(Lang::Eula::Disagree.Text(), ImVec2(btnWidth, 0)))
	{
		this->SetResult(EModalResult::Cancel);
	}
}

void CLicenseAgreementModal::OnClosing()
{
	switch (this->GetResult())
	{
		case EModalResult::OK:
		{
			CContext::GetContext()->GetUIContext()->EulaAccepted.Set(true);
			break;
		}
		case EModalResult::Cancel:
		{
			std::string strHost = Index(EPath::NexusDLL).string();

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
