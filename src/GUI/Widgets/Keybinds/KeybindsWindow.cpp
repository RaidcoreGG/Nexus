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
            KeybindHandler::KeybindRegistryMutex.lock();
            for (auto& [identifier, keybind] : KeybindHandler::KeybindRegistry)
            {
                ImGui::Text(identifier.c_str());
                ImGui::SameLine();
                ImGui::Text(keybind.ToString().c_str());
            }
            KeybindHandler::KeybindRegistryMutex.unlock();
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