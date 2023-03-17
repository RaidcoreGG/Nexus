#include "KeybindsWindow.h"

namespace GUI
{
	std::string CurrentlyEditing;

	void KeybindsWindow::Render()
	{
		if (!Visible) { return; }

		ImGui::SetNextWindowSize(ImVec2(480.0f, 380.0f));
		if (ImGui::Begin("Keybinds", &Visible, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
		{
			if (ImGui::BeginTable("table_keybinds", 2))
			{
				Keybinds::Mutex.lock();
				for (auto& [identifier, keybind] : Keybinds::Registry)
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text(identifier.c_str());

					ImGui::TableSetColumnIndex(1);
					if (ImGui::SmallButton(keybind.Bind.ToString(true).c_str()))
					{
						CurrentlyEditing = identifier;
						ImGui::OpenPopup(("Set Keybind: " + CurrentlyEditing).c_str(), ImGuiPopupFlags_AnyPopupLevel);
					}
				}
				Keybinds::Mutex.unlock();

				ImVec2 center(Renderer::Width * 0.5f, Renderer::Height * 0.5f);
				ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
				if (ImGui::BeginPopupModal(("Set Keybind: " + CurrentlyEditing).c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
				{
					Keybinds::IsSettingKeybind = true;
					if (Keybinds::CurrentKeybind == Keybind{})
					{
						ImGui::Text(Keybinds::Registry[CurrentlyEditing].Bind.ToString(true).c_str());
					}
					else
					{
						ImGui::Text(Keybinds::CurrentKeybind.ToString(true).c_str());
					}

					bool overwriting = false;

					if (Keybinds::CurrentKeybindUsedBy != CurrentlyEditing && Keybinds::CurrentKeybindUsedBy != "")
					{
						ImGui::TextColored(ImVec4(255, 0, 0, 255), "You will overwrite \"%s\".", Keybinds::CurrentKeybindUsedBy.c_str());
						overwriting = true;
					}

					bool close = false;

					if (ImGui::Button("Unbind"))
					{
						Keybinds::Set(CurrentlyEditing, Keybind{});
						close = true;
					}

					/* i love imgui */
					ImGui::SameLine();
					ImGui::Spacing();
					ImGui::SameLine();
					ImGui::Spacing();
					ImGui::SameLine();
					ImGui::Spacing();
					ImGui::SameLine();
					/* i love imgui end*/

					if (ImGui::Button("Accept"))
					{
						if (overwriting)
						{
							Keybinds::Set(Keybinds::CurrentKeybindUsedBy, Keybind{});
						}
						Keybinds::Set(CurrentlyEditing, Keybinds::CurrentKeybind);
						close = true;
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel"))
					{
						close = true;
					}

					if (close)
					{
						CurrentlyEditing = "";
						Keybinds::CurrentKeybind = Keybind{};
						Keybinds::CurrentKeybindUsedBy = "";
						Keybinds::IsSettingKeybind = false;
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}

				ImGui::EndTable();
			}
		}
		ImGui::End();
	}
}