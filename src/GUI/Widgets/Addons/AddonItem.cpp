#include "AddonItem.h"

namespace GUI
{
	void AddonItem(AddonDefinition* aAddonDef)
	{
		std::string sig = std::to_string(aAddonDef->Signature); // helper for unique chkbxIds

		if (ImGui::BeginTable(("#AddonItem" + sig).c_str(), 2, ImGuiTableFlags_BordersOuter, ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f)))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			{
				ImGui::BeginGroup();

				ImGui::TextW(aAddonDef->Name); ImGui::SameLine(); ImGui::Text("by"); ImGui::SameLine(); ImGui::TextW(aAddonDef->Author);
				ImGui::TextWrappedW(aAddonDef->Description);

				ImGui::EndGroup();
			}

			ImGui::TableSetColumnIndex(1);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - 120.0f);
			{
				ImGui::BeginGroup();
				if (aAddonDef->Options) { if (ImGui::Button("Options", ImVec2(120.0f, 24.0f))) { aAddonDef->Options(); } }
				//if (ImGui::Button("Remove", ImVec2(120.0f, 24.0f))) {}
				ImGui::TextCenteredColumnW(aAddonDef->Version);

				ImGui::EndGroup();
			}

			ImGui::EndTable();
		}
	}
}