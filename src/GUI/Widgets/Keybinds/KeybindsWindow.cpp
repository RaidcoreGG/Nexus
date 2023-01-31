#include "KeybindsWindow.h"

#include "../../../Shared.h"
#include "../../../Paths.h"
#include "../../../State.h"

#include "../../../Loader/Loader.h"

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
                ImGui::TextW(identifier.c_str());
                ImGui::SameLine();
                ImGui::TextW(keybind.ToString().c_str());
                ImGui::Text("%p", &identifier);
            }
            KeybindHandler::KeybindRegistryMutex.unlock();

            ImGui::Separator();

            KeybindHandler::KeybindRegistryMutex.lock();
            for (auto& [identifier, handler] : KeybindHandler::KeybindHandlerRegistry)
            {
                ImGui::TextW(identifier.c_str());
                ImGui::SameLine();
                ImGui::Text("Handler: %p", handler);
                ImGui::Text("%p", &identifier);
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