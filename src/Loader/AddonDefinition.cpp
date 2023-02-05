#include "AddonDefinition.h"

bool AddonDefinition::HasMinimumRequirements()
{
	if (Signature != 0 &&
		Name &&
		Version &&
		Author &&
		Description &&
		Load &&
		Unload)
	{
		return true;
	}

	return false;
}

void AddonDefinition::RenderItem()
{
	std::string sig = std::to_string(Signature); // helper for unique chkbxIds

	if (ImGui::BeginTable(("#AddonItem" + sig).c_str(), 2, ImGuiTableFlags_BordersOuter, ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f)))
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		{
			ImGui::BeginGroup();

			ImGui::Text(Name); ImGui::SameLine(); ImGui::Text("by"); ImGui::SameLine(); ImGui::Text(Author);
			ImGui::TextWrapped(Description);

			ImGui::EndGroup();
		}

		ImGui::TableSetColumnIndex(1);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - 120.0f);
		{
			ImGui::BeginGroup();
			if (Options) { if (ImGui::Button("Options", ImVec2(120.0f, 24.0f))) { Options(); } }
			//if (ImGui::Button("Remove", ImVec2(120.0f, 24.0f))) {}
			ImGui::TextCenteredColumn(Version);

			ImGui::EndGroup();
		}

		ImGui::EndTable();
	}
}