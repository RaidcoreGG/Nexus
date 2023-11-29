#include "AddonsWindow.h"

#include <Windows.h>
#include <shellapi.h>

#include "Shared.h"
#include "Paths.h"
#include "State.h"

#include "Loader/Loader.h"
#include "AddonItem.h"

#include "imgui.h"
#include "imgui_extensions.h"

namespace GUI
{
	float awWidth = 30.0f;
	float awHeight = 24.0f;

	AddonsWindow::AddonsWindow(std::string aName)
	{
		Name = aName;
	}

	void AddonsWindow::Render()
	{
		if (!Visible) { return; }

		ImGui::SetNextWindowSize(ImVec2(awWidth * ImGui::GetFontSize(), awHeight * ImGui::GetFontSize()), ImGuiCond_FirstUseEver);
		if (ImGui::Begin(Name.c_str(), &Visible, ImGuiWindowFlags_NoCollapse))
		{
			float width = 7.5f * ImGui::GetFontSize();
			float height = 1.5f * ImGui::GetFontSize();

			if (ImGui::BeginTabBar("AddonTabBar", ImGuiTabBarFlags_None))
			{
				if (ImGui::BeginTabItem("Installed"))
				{
					{
						ImGui::BeginChild("##AddonTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), (height * 1.5f) * -1));

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

					if (ImGui::Button("Open Addons Folder", ImVec2(width, height)))
					{
						ShellExecuteA(NULL, "explore", Path::D_GW2_ADDONS, NULL, NULL, SW_SHOW);
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