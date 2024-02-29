#include "CAboutBox.h"

#include "Shared.h"
#include "Paths.h"
#include "State.h"
#include "Branch.h"

#include "imgui.h"
#include "imgui_extensions.h"

namespace GUI
{
	CAboutBox::CAboutBox(std::string aName)
	{
		Name = aName;
	}

	void CAboutBox::Render()
	{
		if (!Visible) { return; }

		if (ImGui::Begin(Name.c_str(), &Visible, WindowFlags_Default))
		{
			ImGui::TextDisabled("Version:");
			ImGui::Text(""); ImGui::SameLine(); ImGui::Text(Version.ToString().c_str());
#ifdef _DEBUG
			ImGui::SameLine(); ImGui::TextDisabled("debug/" BRANCH_NAME);
#else
			ImGui::SameLine(); ImGui::TextDisabled("release/" BRANCH_NAME);
#endif
			ImGui::TextDisabled("Location:");
			std::string strHost = Path::F_HOST_DLL.string();
			ImGui::Text(""); ImGui::SameLine(); ImGui::Text(strHost.c_str());
			if (ImGui::Tooltip())
			{
				for (std::string param : Parameters)
				{
					ImGui::TextDisabled(param.c_str());
				}

				ImGui::EndTooltip();
			}

			ImGui::TextDisabled("Renderer:");
			ImGui::Text(""); ImGui::SameLine(); ImGui::Text("DirectX 11"); ImGui::SameLine(); ImGui::TextDisabled("Method %d %s", State::EntryMethod, State::IsChainloading ? "Chainloading" : "");
			
			if ((bool)(State::MultiboxState & EMultiboxState::READY))
			{
				ImGui::TextDisabled("Multibox ready.");
			}
			else
			{
				if (State::MultiboxState != EMultiboxState::NONE)
				{
					ImGui::TextDisabled("Multibox State:");
					if ((bool)(State::MultiboxState & EMultiboxState::ARCHIVE_SHARED)) { ImGui::Text(""); ImGui::SameLine(); ImGui::Text("Archive shared."); }
					if ((bool)(State::MultiboxState & EMultiboxState::LOCAL_SHARED)) { ImGui::Text(""); ImGui::SameLine(); ImGui::Text("Local shared."); }
					if ((bool)(State::MultiboxState & EMultiboxState::MUTEX_CLOSED)) { ImGui::Text(""); ImGui::SameLine(); ImGui::Text("Mutex closed."); }
				}
			}

			ImGui::Separator();

			ImVec2 windowSize = ImGui::GetWindowSize();
			ImVec2 textSize = ImGui::CalcTextSize(u8"Raidcore © 2023 - 2024");
			ImVec2 position = ImGui::GetCursorPos();
			ImGui::SetCursorPos(ImVec2((position.x + (windowSize.x - textSize.x)) / 2, position.y));
			ImGui::Text(u8"Raidcore © 2023 - 2024");
		}
		ImGui::End();
	}
}