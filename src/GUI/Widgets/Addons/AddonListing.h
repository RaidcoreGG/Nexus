#ifndef GUI_ADDONSLISTING_H
#define GUI_ADDONSLISTING_H

#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_extensions.h"

#include "../../../Loader/Loader.h"
#include "../../../Loader/AddonDefinition.h"

namespace GUI
{
	static void AddonListing(AddonDefinition* aAddonDef)
	{
		std::string sig = std::to_string(aAddonDef->Signature); // helper for unique chkbxIds
		
		if (ImGui::BeginTable(("component_addon_" + sig).c_str(), 2, ImGuiTableFlags_BordersOuter, ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f)))
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

				if (ImGui::Button("Options", ImVec2(120.0f, 24.0f))) { if (aAddonDef->Options) { aAddonDef->Options(); } }
				//if (ImGui::Button("Remove", ImVec2(120.0f, 24.0f))) {}
				ImGui::TextCenteredColumnW(aAddonDef->Version);

				ImGui::EndGroup();
			}

			ImGui::EndTable();
		}
	}
}

#endif