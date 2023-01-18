#ifndef GUI_ADDONSLISTING_H
#define GUI_ADDONSLISTING_H

#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_extensions.h"

#include "../../../Loader/AddonDef.h"

namespace GUI
{
	static void AddonListing(const AddonDef& aAddonDef)
	{
		std::string sig = std::to_string(aAddonDef.Signature); // helper for unique chkbxIds
		
		int width = ImGui::CalcTextSize("Options").x * 1.5f;

		{
			ImGui::BeginGroup();
			
			{
				ImGui::BeginGroup();
				
				ImGui::TextW(aAddonDef.Name); ImGui::SameLine(); ImGui::Text("by"); ImGui::SameLine(); ImGui::TextW(aAddonDef.Author);
				ImGui::TextW(aAddonDef.Description);
				ImGui::TextW(aAddonDef.Version);

				ImGui::EndGroup();
			}

			ImGui::SameLine(ImGui::GetWindowContentRegionWidth() - (width + 4.0f));

			{
				ImGui::BeginGroup();
				
				ImVec2 pos = ImGui::GetCursorPos();
				pos.y += 4.0f;
				ImGui::SetCursorPos(pos);
				if (ImGui::Button("Options", ImVec2(width, 0.0f))) { if (aAddonDef.Options) { aAddonDef.Options(); } }
				pos = ImGui::GetCursorPos();
				pos.y += 4.0f;
				ImGui::SetCursorPos(pos);
				if (ImGui::Button("Unload", ImVec2(width, 0.0f))) { if (aAddonDef.Unload) { aAddonDef.Unload(); } }

				ImGui::EndGroup();
			}

			ImGui::Separator();

			ImGui::EndGroup();
		}
	}
}

#endif