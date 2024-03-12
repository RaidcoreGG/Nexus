#include "CChangelogWindow.h"

#include <Windows.h>
#include <shellapi.h>

#include "Consts.h"
#include "Shared.h"
#include "Paths.h"
#include "State.h"

#include "GUI/Widgets/QuickAccess/QuickAccess.h"

#include "imgui.h"
#include "imgui_extensions.h"

namespace GUI
{
	float chlWidth = 30.0f;
	float chlHeight = 24.0f;

	CChangelogWindow::CChangelogWindow(std::string aName)
	{
		Name = aName;
	}

	void CChangelogWindow::Render()
	{
		if (!Visible) { return; }

		QuickAccess::SetNotificationShortcut(QA_MENU, false);

		ImGui::SetNextWindowSize(ImVec2(chlWidth * ImGui::GetFontSize(), chlHeight * ImGui::GetFontSize()), ImGuiCond_FirstUseEver);
		if (ImGui::Begin(Name.c_str(), &Visible, ImGuiWindowFlags_NoCollapse))
		{
			//float width = 7.5f * ImGui::GetFontSize();
			//float height = 1.5f * ImGui::GetFontSize();

			if (IsUpdateAvailable)
			{
				ImGui::TextColored(ImVec4(0, 0.580f, 1, 1), Language.Translate("((000039))"));
			}
			else
			{
				ImGui::Text(Language.Translate("((000040))"));
			}

			if (!ChangelogText.empty())
			{
				ImGui::TextWrapped(ChangelogText.c_str());
			}
			else
			{
				ImGui::Text(Language.Translate("((000037))"));
			}
		}
		ImGui::End();
	}
}