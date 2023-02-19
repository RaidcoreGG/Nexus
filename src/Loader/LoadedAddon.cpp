#include "LoadedAddon.h"

void LoadedAddon::RenderItem()
{
	std::string sig = std::to_string(Definitions->Signature); // helper for unique chkbxIds

	if (ImGui::BeginTable(("#AddonItem" + sig).c_str(), 2, ImGuiTableFlags_BordersOuter, ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f)))
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		{
			ImGui::BeginGroup();

			ImGui::Text(Definitions->Name); ImGui::SameLine(); ImGui::Text("by %s", Definitions->Author);
			ImGui::TextWrapped(Definitions->Description);

			ImGui::EndGroup();
		}

		ImGui::TableSetColumnIndex(1);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - 120.0f);
		{
			ImGui::BeginGroup();
			if (Definitions->Options) { if (ImGui::Button("Options", ImVec2(120.0f, 24.0f))) { Definitions->Options(); } }
			//if (ImGui::Button("Remove", ImVec2(120.0f, 24.0f))) {}
			ImGui::TextCenteredColumn(Definitions->Version);

			ImGui::EndGroup();
		}

		ImGui::EndTable();
	}
}