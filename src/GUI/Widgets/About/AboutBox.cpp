#include "AboutBox.h"

namespace GUI
{
	void AboutBox::Render()
	{
		if (!Visible) { return; }

		if (ImGui::Begin("About", &Visible, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
		{
			ImGui::TextDisabled("Version:");
			ImGui::Text(""); ImGui::SameLine(); ImGui::Text(Version);
#ifdef _DEBUG
			ImGui::SameLine(); ImGui::TextDisabled("[DEBUG BUILD]");
#endif
			ImGui::TextDisabled("Location:");
			if (ImGui::TreeNode(Path::F_HOST_DLL))
			{
				for (std::wstring param : Parameters)
				{
					ImGui::Text(""); ImGui::SameLine(); ImGui::TextDisabledW(param.c_str());
				}
				ImGui::TreePop();
			}

			ImGui::TextDisabled("Renderer:");
			ImGui::Text(""); ImGui::SameLine(); ImGui::Text("DirectX 11"); ImGui::SameLine(); ImGui::TextDisabled("Method %d %s", State::EntryMethod, State::IsChainloading ? "Chainloading" : "");
			
			ImGui::TextDisabled("Extras:");
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 0.f }); // smol checkbox
			ImGui::Checkbox("Developer Mode", &State::IsDeveloperMode);
			ImGui::PopStyleVar();
			if (State::MultiboxState == EMultiboxState::READY) { ImGui::TextDisabled("Multibox ready."); }

			ImGui::Separator();

			ImVec2 windowSize = ImGui::GetWindowSize();
			ImVec2 textSize = ImGui::CalcTextSize(u8"RAIDCORE.gg © 2023");
			ImVec2 position = ImGui::GetCursorPos();
			ImGui::SetCursorPos(ImVec2((position.x + (windowSize.x - textSize.x)) / 2, position.y));
			ImGui::Text(u8"RAIDCORE.gg © 2023");
		}
		ImGui::End();
	}
}