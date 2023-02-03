#include "AboutBox.h"

#include "../../../Shared.h"
#include "../../../Paths.h"

namespace GUI
{
	void AboutBox::Render()
	{
		if (!Visible) { return; }

		if (ImGui::Begin("About", &Visible, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
		{
			ImGui::TextDisabled("Version:"); ImGui::SameLine(); ImGui::TextW(Version);
#ifdef _DEBUG
			ImGui::SameLine(); ImGui::TextDisabledW(L"[DEBUG BUILD]");
#endif
			ImGui::TextDisabled("Location:");
			if (ImGui::TreeNodeW(Path::F_HOST_DLL))
			{
				for (std::wstring param : Parameters)
				{
					ImGui::Text(""); ImGui::SameLine(); ImGui::TextDisabledW(param.c_str());
				}
				ImGui::TreePop();
			}
			
			ImGui::Separator();

			ImVec2 windowSize = ImGui::GetWindowSize();
			ImVec2 textSize = ImGui::CalcTextSize(u8"RAIDCORE.gg © 2023");
			ImVec2 position = ImGui::GetCursorPos();
			ImGui::SetCursorPos(ImVec2((position.x + (windowSize.x - textSize.x)) / 2, position.y));
			ImGui::Text(u8"RAIDCORE.gg © 2023");
		}
		ImGui::End();
	}

	void AboutBox::MenuOption(int aCategory)
	{
		if (aCategory == 2)
		{
			ImGui::ToggleButton("About", &Visible, ImVec2(ImGui::GetFontSize() * 13.75f, 0.0f));
		}
	}
}