#include "ChangelogWindow.h"

#include <Windows.h>
#include <shellapi.h>

#include "Consts.h"
#include "Shared.h"
#include "Paths.h"
#include "State.h"

#include "imgui.h"
#include "imgui_extensions.h"

namespace GUI
{
	float chlWidth = 30.0f;
	float chlHeight = 24.0f;

	ChangelogWindow::ChangelogWindow(std::string aName)
	{
		Name = aName;
	}

	void ChangelogWindow::Render()
	{
		if (!Visible) { return; }

		ImGui::SetNextWindowSize(ImVec2(chlWidth * ImGui::GetFontSize(), chlHeight * ImGui::GetFontSize()), ImGuiCond_FirstUseEver);
		if (ImGui::Begin(Name.c_str(), &Visible, ImGuiWindowFlags_NoCollapse))
		{
			//float width = 7.5f * ImGui::GetFontSize();
			//float height = 1.5f * ImGui::GetFontSize();

			if (IsUpdateAvailable)
			{
				ImGui::TextDisabled("These changes take effect the next time you start the game:");
			}
			else
			{
				ImGui::TextDisabled("Last changes:");
			}

			if (!ChangelogText.empty())
			{
				ImGui::TextWrapped(ChangelogText.c_str());
			}
			else
			{
				ImGui::Text("There's nothing here.");
			}
		}
		ImGui::End();
	}
}