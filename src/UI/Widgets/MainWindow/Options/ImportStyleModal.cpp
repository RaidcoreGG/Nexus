///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ImportStyleModal.cpp
/// Description  :  Modal for style importing.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "ImportStyleModal.h"

#include "Host/GW2/Context.h"

CImportStyleModal::CImportStyleModal()
{
	this->SetID("ImportStyleModal");
	this->SetDisplayName(Lang::Options::StyleImportPresetTitle);
	memset(this->DataBuffer, 0, sizeof(this->DataBuffer));
}

void CImportStyleModal::RenderContent()
{
	ImGui::InputTextWithHint("##InputStyleCode", Lang::Options::StyleEditor::Code.Text(), this->DataBuffer, sizeof(this->DataBuffer));

	if (ImGui::Button(Lang::Common::Apply.Text()))
	{
		this->SetResult(EModalResult::OK);
	}

	ImGui::SameLine();

	if (ImGui::Button(Lang::Common::Cancel.Text()))
	{
		this->SetResult(EModalResult::Cancel);
	}
}

void CImportStyleModal::OnClosing()
{
	switch (this->GetResult())
	{
		case EModalResult::OK:
		{
			CContext* ctx = CContext::GetContext();
			CUiContext* uictx = ctx->GetUIContext();

			uictx->ApplyStyle(EUIStyle::Code, this->DataBuffer);

			/* Reset data. */
			memset(this->DataBuffer, 0, sizeof(this->DataBuffer));
			break;
		}
		case EModalResult::Cancel:
		{
			/* Reset data. */
			memset(this->DataBuffer, 0, sizeof(this->DataBuffer));
			break;
		}
	}
}
