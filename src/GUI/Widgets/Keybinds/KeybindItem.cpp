#include "KeybindItem.h"

namespace GUI
{
	void KeybindItem(std::wstring aIdentifier, Keybind aKeybind)
	{
		if (ImGui::BeginTable("#KeybindItem", 3, ImGuiTableFlags_BordersOuter, ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f)))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			{
				ImGui::BeginGroup();

				ImGui::TextW(aIdentifier.c_str());

				ImGui::EndGroup();
			}

			ImGui::TableSetColumnIndex(1);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - 120.0f);
			{
				ImGui::BeginGroup();
				
				//ImGui::SmallButton("")

				ImGui::EndGroup();
			}

			ImGui::EndTable();
		}
	}
}