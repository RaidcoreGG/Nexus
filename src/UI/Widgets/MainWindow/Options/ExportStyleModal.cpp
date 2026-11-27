///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ExportStyleModal.cpp
/// Description  :  Modal for style exports.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "ExportStyleModal.h"

#include <fstream>

#include "imgui/imgui.h"

#include "Host/GW2/Context.h"
#include "Host/GW2/Index/Index.h"

namespace Raidcore::UI
{
	CExportStyleModal::CExportStyleModal()
	{
		this->SetID("ExportStyleModal");
		this->SetDisplayName(Lang::Options::StyleSavePresetTitle);
		this->Data = {};
		memset(this->PathBuffer, 0, sizeof(this->PathBuffer));
	}

	void CExportStyleModal::RenderContent()
	{
		if (ImGui::InputTextWithHint("##InputFilename", Lang::Options::StyleEditor::Name.Text(), this->PathBuffer, sizeof(this->PathBuffer)))
		{
			this->Path = GW2::Index(GW2::EPath::DIR_STYLES) / this->PathBuffer;
			this->Path += ".imstyle180";
		}

		if (std::filesystem::exists(this->Path))
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.f, 0.f, 1.f));
			ImGui::TextWrapped(Lang::Options::StyleEditor::Overwrite.Text(), this->PathBuffer);
			ImGui::PopStyleColor();
		}

		if (ImGui::Button(Lang::Common::Save.Text()))
		{
			this->SetResult(UI::EModalResult::OK);
		}

		ImGui::SameLine();

		if (ImGui::Button(Lang::Common::Cancel.Text()))
		{
			this->SetResult(UI::EModalResult::Cancel);
		}
	}

	void CExportStyleModal::OnClosing()
	{
		switch (this->GetResult())
		{
			case UI::EModalResult::OK:
			{
				try
				{
					std::ofstream file(this->Path);

					if (!file)
					{
						/* Signal failure, to keep the modal open. */
						this->SetResult(UI::EModalResult::None);
						break;
					}

					file << this->Data;
					file.close();

					this->ClearData();
				}
				catch (...)
				{
					GW2::CContext* ctx = GW2::CContext::GetContext();
					Core::LogWarning("UI Context", "Error saving stylesheet.");

					/* Signal failure, to keep the modal open. */
					this->SetResult(UI::EModalResult::None);
				}
				break;
			}
			case UI::EModalResult::Cancel:
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
}
