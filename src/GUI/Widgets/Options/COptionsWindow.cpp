#ifdef MEME
#include "COptionsWindow.h"

#include <algorithm>
#include <regex>
#include <vector>
#include <thread>
#include <filesystem>

#include "Consts.h"
#include "Index.h"
#include "Renderer.h"
#include "Shared.h"
#include "State.h"

#include "Inputs/InputBinds/InputBindHandler.h"
#include "Loader/Loader.h"
#include "Services/API/APIController.h"
#include "Services/Localization/Localization.h"
#include "Services/Settings/Settings.h"

#include "Util/Base64.h"

#include "GUI/GUI.h"
#include "GUI/IWindow.h"
#include "GUI/Widgets/QuickAccess/QuickAccess.h"

#include "imgui/imgui.h"
#include "imgui/imgui_extensions.h"

namespace GUI
{
	float owWidth = 30.0f;
	float owHeight = 24.0f;

	char CurrentAPIKey[73]{};

	/* proto tabs */
	bool AddonsTab();
	void APITab();
	void ChangelogTab();

	COptionsWindow::COptionsWindow(std::string aName)
	{
		Name = aName;
	}

	bool AddonsTab()
	{
		if (RegistryOptionsRender.size() == 0) { return false; }

		bool isActive = false;

		if (ImGui::BeginTabItem(Language->Translate("((000003))")))
		{
			isActive = true;

			PopulateInputBinds();

			IsInputBindsPanelOpen = true;

			ImGui::BeginChild("##AddonsTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

			if (ImGui::BeginTabBar("AddonOptionsTabBar", ImGuiTabBarFlags_None))
			{
				for (GUI_RENDER renderCb : RegistryOptionsRender)
				{
					std::string parent = Loader::GetOwner(renderCb);
					if (ImGui::BeginTabItem((parent + "##AddonOptions").c_str()))
					{
						for (KBCat cat : InputBindCategoryMap)
						{
							if (cat.Name != parent) { continue; }
							if (cat.InputBinds.size() == 0) { continue; }

							if (ImGui::CollapsingHeader(Language->Translate("((000060))"), ImGuiTreeNodeFlags_DefaultOpen))
							{
								DrawInputBindsTable(cat.InputBinds);
							}
						}

						if (ImGui::CollapsingHeader(Language->Translate("((000004))"), ImGuiTreeNodeFlags_DefaultOpen))
						{
							renderCb();
						}

						ImGui::EndTabItem();
					}
				}

				ImGui::EndTabBar();
			}

			ImGui::EndChild();

			ImGui::EndTabItem();
		}

		return isActive;
	}

	void APITab()
	{
		if (ImGui::BeginTabItem("API"))
		{
			{
				ImGui::BeginChild("##APITabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

				ImGui::InputText("##currentapikey", CurrentAPIKey, 73, ImGuiInputTextFlags_EnterReturnsTrue);
				ImGui::SameLine();
				if (ImGui::Button("Add"))
				{
					std::string str = CurrentAPIKey;

					if (std::regex_match(str, std::regex("[A-F\\d]{8}-([A-F\\d]{4}-){3}[A-F\\d]{20}(-[A-F\\d]{4}){3}-[A-F\\d]{12}")))
					{
						API::AddKey(str);
						memset(CurrentAPIKey, 0, 73);
					}
				}

				bool shouldSave = false;

				if (ImGui::BeginTable("table_apikeys", 2, ImGuiTableFlags_BordersInnerH))
				{
					API::Mutex.lock();
					{
						for (ActiveToken key : API::Keys)
						{
							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("%.18s", key.Key.c_str());

							ImGui::TableSetColumnIndex(1);
							if (ImGui::Button(("Remove##" + key.Key).c_str())) // ##+key for unique id
							{
								// copy paste of the API::Save() code because of mutex shenanigans
								API::Keys.erase(std::find(API::Keys.begin(), API::Keys.end(), key));
								shouldSave = true;
							}
						}
					}
					API::Mutex.unlock();

					ImGui::EndTable();
				}

				ImGui::EndChild();
			}

			ImGui::EndTabItem();
		}
	}

	void ChangelogTab()
	{
		static bool didNotify = false;

		if (!didNotify)
		{
			QuickAccess::SetNotificationShortcut(QA_MENU, false);
			didNotify = true;
		}

		if (ImGui::BeginTabItem(Language->Translate("((000005))")))
		{
			if (IsUpdateAvailable)
			{
				ImGui::TextColored(ImVec4(0, 0.580f, 1, 1), Language->Translate("((000039))"));
			}
			else
			{
				ImGui::Text(Language->Translate("((000040))"));
			}

			if (!ChangelogText.empty())
			{
				ImGui::TextWrapped(ChangelogText.c_str());
			}
			else
			{
				ImGui::Text(Language->Translate("((000037))"));
			}

			ImGui::EndTabItem();
		}
	}
}
#endif