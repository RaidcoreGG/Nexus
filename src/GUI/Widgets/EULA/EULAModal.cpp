#include "EULAModal.h"

#include <Windows.h>
#include <shellapi.h>

#include "Renderer.h"
#include "Shared.h"
#include "Paths.h"
#include "Consts.h"

#include "GUI/GUI.h"
#include "Settings/Settings.h"
#include "Events/EventHandler.h"

#include "imgui.h"
#include "imgui_extensions.h"

namespace GUI
{
	/* proto tabs */
	void GeneralTab();
	void AddonsTab();
	void StyleTab();
	void KeybindsTab();
	void APITab();

	EULAModal::EULAModal()
	{
		Name = "EULAModal";
	}

	void EULAModal::Render()
	{
		ImGui::OpenPopup("Legal Agreement", ImGuiPopupFlags_AnyPopupLevel);
		ImVec2 center(Renderer::Width * 0.5f, Renderer::Height * 0.5f);
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("Legal Agreement", NULL, WindowFlags_Default))
		{
			bool close = false;

			ImGui::TextWrapped("This is an unofficial library and Raidcore is in no way associated with ArenaNet nor with any of its partners. Modifying Guild Wars 2 through any third party software is not supported by ArenaNet nor by any of its partners.");

			ImGui::Text("By using this software you are agreeing to the terms and conditions as laid out on:");

			if (ImGui::TextURL("https://raidcore.gg/Legal", true, false))
			{
				ShellExecuteA(0, 0, "https://raidcore.gg/Legal", 0, 0, SW_SHOW);
			}

			ImGui::Text("If you do not agree to these terms, do not use the software.");

			ImGui::TextDisabled("By clicking \"I do NOT agree\" your game will close and Nexus will attempt to uninstall.");
			ImGui::TextDisabled("If you see this prompt again after restarting, you will have to manually remove \"%s\" while the game is closed.", Path::F_HOST_DLL.string().c_str());

			if (ImGui::Button("I agree"))
			{
				HasAcceptedEULA = true;
				Settings::Settings[OPT_ACCEPTEULA] = true;
				Settings::Save();
				close = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("I do NOT agree"))
			{
				std::string strHost = Path::F_HOST_DLL.string();

				SHFILEOPSTRUCT fileOp;
				fileOp.hwnd = NULL;
				fileOp.wFunc = FO_DELETE;
				fileOp.pFrom = strHost.c_str();
				fileOp.pTo = NULL;
				fileOp.fFlags = FOF_ALLOWUNDO | FOF_NOERRORUI | FOF_NOCONFIRMATION | FOF_SILENT;
				int result = SHFileOperationA(&fileOp);

				close = true;

				exit(0);
			}

			if (close)
			{
				ImGui::CloseCurrentPopup();
				Events::RaiseNotification(EV_EULA_ACCEPTED);
			}

			ImGui::EndPopup();
		}
	}
}