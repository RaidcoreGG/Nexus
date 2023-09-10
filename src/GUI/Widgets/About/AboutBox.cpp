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
			ImGui::Text(""); ImGui::SameLine(); ImGui::Text(Path::F_HOST_DLL);
			if (ImGui::Tooltip())
			{
				for (std::string param : Parameters)
				{
					ImGui::TextDisabled(("-" + param).c_str());
				}

				ImGui::EndTooltip();
			}

			ImGui::TextDisabled("Renderer:");
			ImGui::Text(""); ImGui::SameLine(); ImGui::Text("DirectX 11"); ImGui::SameLine(); ImGui::TextDisabled("Method %d %s", State::EntryMethod, State::IsChainloading ? "Chainloading" : "");
			
			if (State::MultiboxState == EMultiboxState::READY) { ImGui::TextDisabled("Multibox ready."); }

			ImGui::Separator();

			ImVec2 windowSize = ImGui::GetWindowSize();
			ImVec2 textSize = ImGui::CalcTextSize(u8"Raidcore © 2023");
			ImVec2 position = ImGui::GetCursorPos();
			ImGui::SetCursorPos(ImVec2((position.x + (windowSize.x - textSize.x)) / 2, position.y));
			ImGui::Text(u8"Raidcore © 2023");
		}
		ImGui::End();
	}
}