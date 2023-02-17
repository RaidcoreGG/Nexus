#include "KeybindsWindow.h"

#include "../../../Shared.h"
#include "../../../Paths.h"
#include "../../../State.h"

namespace GUI
{
    void KeybindsWindow::Render()
    {
        if (!Visible) { return; }

        ImGui::SetNextWindowSize(ImVec2(480.0f, 380.0f));
        if (ImGui::Begin("Keybinds", &Visible, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
        {
            Keybinds::Mutex.lock();
            for (auto& [identifier, keybind] : Keybinds::Registry)
            {
                ImGui::Text(identifier.c_str());
                ImGui::SameLine();
                if (ImGui::SmallButton(keybind.ToString().c_str()))
                {
                    ImGui::OpenPopup("Set Keybind");
                }

                ImVec2 center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                if (ImGui::BeginPopupModal("Set Keybind", NULL, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    /* somehow take input, idk */

                    if (ImGui::Button("Accept")) {}
                    if (ImGui::Button("Cancel")) { ImGui::CloseCurrentPopup(); }

                    ImGui::EndPopup();
                }
            }
            Keybinds::Mutex.unlock();
        }
        ImGui::End();
    }

    void KeybindsWindow::MenuOption(int aCategory)
    {
        if (aCategory == 0)
        {
            ImGui::ToggleButton("Keybinds", &Visible, ImVec2(ImGui::GetFontSize() * 13.75f, 0.0f));
        }
    }
}