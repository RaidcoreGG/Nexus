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
			;
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
		}

		void AddMenuItem(std::string aLabel, std::string aTextureIdentifier, bool* aToggle)
		{
			Texture* icon = TextureLoader::Get(aTextureIdentifier.c_str());
			MenuItem* mItem = new MenuItem{ aLabel, aToggle, icon, false };

			{
				const std::lock_guard<std::mutex> lock(Menu::Mutex);
				MenuItems.push_back(mItem);
			}

			if (icon == nullptr)
			{
				std::thread([mItem, aLabel, aTextureIdentifier, aToggle]()
					{
						const std::lock_guard<std::mutex> lock(Menu::Mutex);
						{
							// This code is copy pasted from quick access, bit of a clownfiesta not gonna lie
							int tries = 0;

							//LogDebug(CH_QUICKACCESS, "Menu Item \"%s\" was promised 1 textures, but received 0.", aLabel.c_str());
							Sleep(100); // first retry after 100ms

							while (mItem->Icon == nullptr)
							{
								if (tries > 10)
								{
									//LogWarning(CH_QUICKACCESS, "Cancelled getting textures for menu item \"%s\" after 10 failed attempts.", aLabel.c_str());
									break;
								}

								if (mItem->Icon == nullptr) { mItem->Icon = TextureLoader::Get(aTextureIdentifier.c_str()); }

								tries++;
								Sleep(10);
							}

							/* if not all tries were used, then the texture was loaded */
							if (tries <= 10)
							{
								//LogDebug(CH_QUICKACCESS, "Menu Item \"%s\" received promised texture after %d attempt(s).", aLabel.c_str(), tries);
								return;
							}

							/* fallback icons */
							mItem->Icon = TextureLoader::Get(ICON_GENERIC);

							/* absolute sanity check */
							if (mItem->Icon == nullptr)
							{
								//LogWarning(CH_QUICKACCESS, "Neither promised textures nor fallback textures could be loaded, removing menu item \"%s\".", aLabel.c_str());
								return;
							}
						}
					}).detach();
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