#include "Menu.h"

#include "Shared.h"
#include "Paths.h"
#include "State.h"
#include "Renderer.h"
#include "resource.h"
#include "Consts.h"

#include "GUI/GUI.h"
#include "GUI/IWindow.h"

#include "Textures/TextureLoader.h"

#include "imgui.h"
#include "imgui_extensions.h"

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
			
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));

			if (ImGui::Begin("Menu", &Visible, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar))
			{
				if (MenuBG)
				{
					ImGui::SetCursorPos(ImVec2(0, 0));
					ImGui::Image(MenuBG->Resource, ImVec2(bgSz, bgSz));
				}

				ImGui::PushFont(FontUI);
				ImGui::SetCursorPos(ImVec2(8.0f, 8.0f));
				Menu::Mutex.lock();
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
				Menu::Mutex.unlock();
				ImGui::PopFont();
			}
			ImGui::End();

			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
		}

		void AddMenuItem(std::string aLabel, std::string aTextureIdentifier, bool* aToggle)
		{
			Texture* icon = TextureLoader::Get(aTextureIdentifier.c_str());
			MenuItem* mItem = new MenuItem{ aLabel, aTextureIdentifier, aToggle, icon, false };

			{
				const std::lock_guard<std::mutex> lock(Menu::Mutex);
				MenuItems.push_back(mItem);
			}
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