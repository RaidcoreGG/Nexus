#include "About.h"

#include "../Shared.h"
#include "../Paths.h"
#include "../State.h"

namespace GUI
{
	bool About::Visible = false;

	void About::Show()
	{
		if (!Visible) { return; }

		if (ImGui::Begin("About", &Visible, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
		{
			ImGui::Text("Version:");	ImGui::SameLine(); ImGui::TextW(Version);
#ifdef _DEBUG
			ImGui::SameLine(); ImGui::Text("[DEBUG BUILD]");
#endif
			ImGui::Text("Location:");	ImGui::SameLine(); ImGui::TextW(Path::F_HOST_DLL);
			ImGui::Text("Parameters:"); ImGui::SameLine(); ImGui::TextW(Parameters);

			/*ImGui::Separator();

			ImGui::Text("CRC32: 0");*/

			ImGui::Separator();

			ImGui::Text(u8"RAIDCORE.gg © 2018 - 2023");
		}
		ImGui::End();
	}
}