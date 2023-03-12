#include "AddonItem.h"

namespace GUI
{
	void AddonItem(AddonDefinition* aDefinitions)
	{
		std::string sig = std::to_string(aDefinitions->Signature); // helper for unique chkbxIds

		if (ImGui::BeginTable(("#AddonItem" + sig).c_str(), 2, ImGuiTableFlags_BordersOuter, ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f)))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			{
				ImGui::BeginGroup();

				ImGui::Text(aDefinitions->Name); ImGui::SameLine(); ImGui::Text("by %s", aDefinitions->Author);
				ImGui::TextWrapped(aDefinitions->Description);

				ImGui::EndGroup();
			}

			ImGui::TableSetColumnIndex(1);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - 120.0f);
			{
				ImGui::BeginGroup();
				if (aDefinitions->HasFlag(EAddonFlags::HasOptions))
				{
					if (ImGui::Button("Options", ImVec2(120.0f, 24.0f)))
					{
						Events::Raise(EV_OPTIONS_CALLED, (void*)aDefinitions->Signature);
					}
				}
				if (ImGui::Button("Uninstall", ImVec2(120.0f, 24.0f)))
				{
					LogDebug("Uninstall called: %s", aDefinitions->Name);
				}
				ImGui::TextCenteredColumn(aDefinitions->Version);

				ImGui::EndGroup();
			}

			ImGui::EndTable();
		}
	}
}