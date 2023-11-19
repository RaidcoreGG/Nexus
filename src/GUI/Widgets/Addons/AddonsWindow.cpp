#include "AddonsWindow.h"

namespace GUI
{
	AddonsWindow::AddonsWindow(std::string aName)
	{
		Name = aName;
	}

	void AddonsWindow::Render()
	{
		if (!Visible) { return; }

		ImGui::SetNextWindowSize(ImVec2(480.0f, 380.0f));
		if (ImGui::Begin(Name.c_str(), &Visible, WindowFlags_Default))
		{
			if (ImGui::BeginTabBar("AddonTabBar", ImGuiTabBarFlags_None))
			{
				if (ImGui::BeginTabItem("Installed"))
				{
					{
						ImGui::BeginChild("##AddonTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

						if (Loader::Addons.size() == 0)
						{
							ImVec2 windowSize = ImGui::GetWindowSize();
							ImVec2 textSize = ImGui::CalcTextSize("No addons installed.");
							ImVec2 position = ImGui::GetCursorPos();
							ImGui::SetCursorPos(ImVec2((position.x + (windowSize.x - textSize.x)) / 2, (position.y + (windowSize.y - textSize.y)) / 2));
							ImGui::TextDisabled("No addons installed.");
						}
						else
						{
							Loader::Mutex.lock();
							{
								for (auto& [path, addon] : Loader::Addons)
								{
									AddonItem(addon);
								}
							}
							Loader::Mutex.unlock();
						}

						ImGui::EndChild();
					}
					
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
}