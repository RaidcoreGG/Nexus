///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ExportStyleModal.cpp
/// Description  :  Modal for style exports.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "ExportStyleModal.h"

#include <fstream>

#include "imgui.h"

#include "Renderer.h"
#include "Index.h"
#include "Context.h"

CExportStyleModal::CExportStyleModal()
{
	this->SetID("ExportStyleModal");
	this->SetDisplayName("Save style preset");
	this->Data = {};
	memset(this->PathBuffer, 0, sizeof(this->PathBuffer));
}

void CExportStyleModal::RenderContent()
{
	if (ImGui::InputTextWithHint("##InputFilename", "Style Name", this->PathBuffer, sizeof(this->PathBuffer)))
	{
		this->Path = Index::D_GW2_ADDONS_NEXUS_STYLES / this->PathBuffer;
		this->Path += ".imstyle180";
	}

	if (std::filesystem::exists(this->Path))
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.f, 0.f, 1.f));
		ImGui::TextWrapped("%s already exists and will be overwritten.", this->PathBuffer);
		ImGui::PopStyleColor();
	}

	if (ImGui::Button("Save"))
	{
		this->SetResult(EModalResult::OK);
	}

	ImGui::SameLine();

	if (ImGui::Button("Cancel"))
	{
		this->SetResult(EModalResult::Cancel);
	}
}

void CExportStyleModal::OnClosing()
{
	switch (this->GetResult())
	{
		case EModalResult::OK:
		{
			try
			{
				std::ofstream file(this->Path);

				if (!file)
				{
					/* Signal failure, to keep the modal open. */
					this->SetResult(EModalResult::None);
					break;
				}

				file << this->Data;
				file.close();

				this->ClearData();
			}
			catch (...)
			{
				CContext* ctx = CContext::GetContext();
				CLogHandler* logger = ctx->GetLogger();
				logger->Warning(CH_UICONTEXT, "Error saving stylesheet.");

				/* Signal failure, to keep the modal open. */
				this->SetResult(EModalResult::None);
			}
			break;
		}
		case EModalResult::Cancel:
		{
			this->ClearData();
			break;
		}
	}
}

void CExportStyleModal::SetData(const std::string& aBase64Style)
{
	this->Data = aBase64Style;

	this->OpenModal();
}

void CExportStyleModal::ClearData()
{
	/* Reset data. */
	this->Data.clear();
	memset(this->PathBuffer, 0, sizeof(this->PathBuffer));
	this->Path.clear();
}
