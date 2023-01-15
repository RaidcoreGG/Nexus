#include "AddonsWindow.h"

#include "../../../Shared.h"
#include "../../../Paths.h"
#include "../../../State.h"

namespace GUI
{
	void AddonsWindow::Render()
	{
		if (!Visible) { return; }

        ImGui::SetNextWindowSize(ImVec2(480.0f, 380.0f));
		if (ImGui::Begin("Addons", &Visible, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
		{
            if (ImGui::BeginTabBar("AddonTabBar", ImGuiTabBarFlags_None))
            {
                ImGui::GetItemRectSize();
                if (ImGui::BeginTabItem("Installed"))
                {
                    ImVec2 windowSize = ImGui::GetWindowSize();
                    ImVec2 textSize = ImGui::CalcTextSize("No addons installed.");
                    ImVec2 position = ImGui::GetCursorPos();
                    ImGui::SetCursorPos(ImVec2((position.x + (windowSize.x - textSize.x)) / 2, (position.y + (windowSize.y - textSize.y)) / 2));
                    ImGui::TextDisabled("No addons installed.");
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Library"))
                {
                    ImVec2 windowSize = ImGui::GetWindowSize();
                    ImVec2 textSize = ImGui::CalcTextSize("Unable to fetch addons.");
                    ImVec2 position = ImGui::GetCursorPos();
                    ImGui::SetCursorPos(ImVec2((position.x + (windowSize.x - textSize.x)) / 2, (position.y + (windowSize.y - textSize.y)) / 2));
                    ImGui::TextDisabled("Unable to fetch addons.");
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
		}
		ImGui::End();
	}

    void AddonsWindow::MenuOption(const wchar_t* aCategory)
    {
        if (wcscmp(aCategory, L"Main") == 0)
        {
            ImGui::ToggleButton("Addons", &Visible, ImVec2(ImGui::GetFontSize() * 13.75f, 0.0f));
        }
    }
}