#include "Menu.h"

#include "../../GUI.h"

namespace GUI
{
	namespace Menu
	{
		bool Visible						= false;

		std::mutex Mutex;
		std::vector<MenuItem*> MenuItems;

		Texture* MenuBG						= nullptr;
		Texture* MenuButton					= nullptr;
		Texture* MenuButtonHover			= nullptr;

		void Render()
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
					for (MenuItem* mItem : MenuItems)
					{
						if (mItem->Label != "Debug" || State::IsDeveloperMode)
						{
							if (mItem->Render())
							{
								if (GUI::CloseMenuAfterSelecting)
								{
									/* if they return true, they were pressed -> hide the menu */
									Visible = false;
								}
							}
						}
					}
				}
				Mutex.unlock();
				ImGui::PopFont();
			}
			ImGui::End();
		}

		void AddMenuItem(std::string aLabel, bool* aToggle)
		{
			Mutex.lock();
			{
				MenuItems.push_back(new MenuItem{ aLabel, aToggle, false });
			}
			Mutex.unlock();
		}

		void ReceiveTextures(const char* aIdentifier, Texture* aTexture)
		{
			std::string str = aIdentifier;

			if (str == TEX_MENU_BACKGROUND)
			{
				MenuBG = aTexture;
			}
			else if (str == TEX_MENU_BUTTON)
			{
				MenuButton = aTexture;
			}
			else if (str == TEX_MENU_BUTTON_HOVER)
			{
				MenuButtonHover = aTexture;
			}
		}
	}
}