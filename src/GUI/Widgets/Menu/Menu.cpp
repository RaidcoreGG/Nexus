#include "Menu.h"

#include "../../GUI.h"

namespace GUI
{
	void MenuWindow::Render()
	{
		if (!Visible) { return; }

		float bgSz = 220 * Renderer::Scaling;
		;
		if (ImGui::Begin("Menu", &Visible, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar))
		{
			if (MenuBG)
			{
				ImGui::SetCursorPos(ImVec2(0, 0));
				ImGui::Image(MenuBG->Resource, ImVec2(bgSz, bgSz));
			}

			ImGui::PushFont(FontUI);
			ImGui::SetCursorPos(ImVec2(0, 8.0f));
			Mutex.lock();
			{
				for (MenuItem* mItem : MenuItems) { if (mItem->Render()) { Visible = false; } }
			}
			Mutex.unlock();
			ImGui::PopFont();
		}
		ImGui::End();
	}

	void MenuWindow::AddMenuItem(const char* aLabel, bool* aToggle)
	{
		Mutex.lock();
		{
			MenuItems.push_back(new MenuItem{ aLabel, aToggle, false });
		}
		Mutex.unlock();
	}
}