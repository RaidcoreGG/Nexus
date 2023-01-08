#include "Menu.h"
#include "../Shared.h"
#include "../Paths.h"
#include "../State.h"

namespace GUI
{
	namespace Menu
	{
		static bool IsShown = true;
		static bool IsAbout = false;

		void Show()
		{
			if (ImGui::Begin("Raidcore", &IsShown, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
			{

				ImGui::Button("Addons", ImVec2(ImGui::GetFontSize() * 13.75f, ImGui::GetFontSize() * 1.25f));
				ImGui::Button("Keybinds", ImVec2(ImGui::GetFontSize() * 13.75f, ImGui::GetFontSize() * 1.25f));

				ImGui::Separator();

				ImGui::Button("Log", ImVec2(ImGui::GetFontSize() * 13.75f, ImGui::GetFontSize() * 1.25f));

				ImGui::Separator();

				if (ImGui::Button("About", ImVec2(ImGui::GetFontSize() * 13.75f, ImGui::GetFontSize() * 1.25f))) { IsAbout = !IsAbout; }
			}
			ImGui::End();
			About();
		}

		void About()
		{
			if (!IsAbout) { return; }

			if (ImGui::Begin("About", &IsAbout, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
			{
				ImGui::TextW(Version);
				ImGui::Text("Location:"); ImGui::SameLine(); ImGui::TextW(Path::F_HOST_DLL);
				ImGui::Text("Parameters:"); ImGui::SameLine(); ImGui::TextW(Parameters);

				/*ImGui::Separator();

				ImGui::Text("CRC32: 0");*/

				ImGui::Separator();

				ImGui::Text("RAIDCORE.gg (Copyright) 2018 - 2023");
			}
			ImGui::End();
		}
	}
}