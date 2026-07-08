///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ImportStyleModal.cpp
/// Description  :  Modal for style importing.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "ImportStyleModal.h"

#include "Runtime/Runtime.h"
using namespace Raidcore::Nexus;

CImportStyleModal::CImportStyleModal()
{
	this->SetID("ImportStyleModal");
	this->SetDisplayName("Import style preset");
	memset(this->DataBuffer, 0, sizeof(this->DataBuffer));
}

void CImportStyleModal::RenderContent()
{
	ImGui::InputTextWithHint("##InputStyleCode", "Style Code", this->DataBuffer, sizeof(this->DataBuffer));

	if (ImGui::Button("Apply"))
	{
		this->SetResult(EModalResult::OK);
	}

	ImGui::SameLine();

	if (ImGui::Button("Cancel"))
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
			Runtime& ctx = Runtime::Get();
			CUiContext* uictx = ctx.GetUIContext();

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
