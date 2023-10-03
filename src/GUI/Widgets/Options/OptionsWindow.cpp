#include "OptionsWindow.h"

namespace GUI
{
	std::string CurrentlyEditing;
	char CurrentAPIKey[73]{};

	const char* qaLocations[] = { "Extend", "Under", "Bottom", "Custom" };

	void OptionsWindow::Render()
	{
		if (!Visible) { return; }

		ImGui::ShowStyleEditor();

		ImGui::SetNextWindowSize(ImVec2(480.0f, 380.0f));
		if (ImGui::Begin("Options", &Visible, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
		{
			if (ImGui::BeginTabBar("OptionsTabBar", ImGuiTabBarFlags_None))
			{
				if (ImGui::BeginTabItem("General"))
				{
					{
						ImGui::BeginChild("##GeneralTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

						ImGui::TextDisabled("General");
						if (ImGui::Checkbox("Developer Mode", &State::IsDeveloperMode))
						{
							Settings::Settings[OPT_DEVMODE] = State::IsDeveloperMode;
							Settings::Save();
						}
						ImGui::TooltipGeneric("Enables the Debug menu and some other features.");

						ImGui::TextDisabled("UI/UX");
						if (ImGui::Checkbox("Close Menu after selecting item", &GUI::CloseMenuAfterSelecting))
						{
							Settings::Settings[OPT_CLOSEMENU] = GUI::CloseMenuAfterSelecting;
							Settings::Save();
						}

						ImGui::EndChild();
					}

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Style"))
				{
					{
						ImGui::BeginChild("##StyleTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

						ImGui::TextDisabled("Font");
						ImGui::Text("Size: ");
						ImGui::SameLine();
						if (ImGui::InputFloat("##fontsize", &GUI::FontSize, 0.0f, 0.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue))
						{
							Settings::Settings[OPT_FONTSIZE] = FontSize;
							Settings::Save();
						}
						ImGui::TooltipGeneric("Changing font size requires a restart. You can preview different fonts and sizes with the \"Preview\" button.");

						ImGui::Separator();

						ImGui::TextDisabled("Quick Access");
						if (ImGui::Checkbox("Vertical Layout", &QuickAccess::VerticalLayout))
						{
							Settings::Settings[OPT_QAVERTICAL] = QuickAccess::VerticalLayout;
							Settings::Save();
						}
						ImGui::Text("Location:");
						if (ImGui::Combo("##qalocation", (int*)&QuickAccess::Location, qaLocations, IM_ARRAYSIZE(qaLocations)))
						{
							Settings::Settings[OPT_QALOCATION] = QuickAccess::Location;
							Settings::Save();
						}
						ImGui::Text("Offset:");
						if (ImGui::InputFloat2("##qaoffset", (float*)&QuickAccess::Offset))
						{
							Settings::Settings[OPT_QAOFFSETX] = QuickAccess::Offset.x;
							Settings::Settings[OPT_QAOFFSETY] = QuickAccess::Offset.y;
							Settings::Save();
						}

						ImGui::EndChild();
					}

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Keybinds"))
				{
					{
						ImGui::BeginChild("##KeybindsTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

						float kbButtonWidth = ImGui::CalcTextSize("XXXXXXXXXXXXXXXX").x;

						if (ImGui::BeginTable("table_keybinds", 3, ImGuiTableFlags_BordersInnerH))
						{
							Keybinds::Mutex.lock();
							{
								for (auto& [identifier, keybind] : Keybinds::Registry)
								{
									ImGui::TableNextRow();
									ImGui::TableSetColumnIndex(0);
									ImGui::Text(identifier.c_str());

									ImGui::TableSetColumnIndex(1);
									if (ImGui::Button(keybind.Bind.ToString(true).c_str(), ImVec2(kbButtonWidth, 0.0f)))
									{
										CurrentlyEditing = identifier;
										ImGui::OpenPopup(("Set Keybind: " + CurrentlyEditing).c_str(), ImGuiPopupFlags_AnyPopupLevel);
									}

									ImGui::TableSetColumnIndex(2);
									if (keybind.Handler == nullptr)
									{
										ImGui::TextDisabled("Keybind not in use.");
									}
								}
							}
							Keybinds::Mutex.unlock();

							ImVec2 center(Renderer::Width * 0.5f, Renderer::Height * 0.5f);
							ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
							if (ImGui::BeginPopupModal(("Set Keybind: " + CurrentlyEditing).c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
							{
								Keybinds::IsSettingKeybind = true;
								if (Keybinds::CurrentKeybind == Keybind{})
								{
									ImGui::Text(Keybinds::Registry[CurrentlyEditing].Bind.ToString(true).c_str());
								}
								else
								{
									ImGui::Text(Keybinds::CurrentKeybind.ToString(true).c_str());
								}

								bool overwriting = false;

								if (Keybinds::CurrentKeybindUsedBy != CurrentlyEditing && Keybinds::CurrentKeybindUsedBy != "")
								{
									ImGui::TextColored(ImVec4(255, 0, 0, 255), "You will overwrite \"%s\".", Keybinds::CurrentKeybindUsedBy.c_str());
									overwriting = true;
								}

								bool close = false;

								if (ImGui::Button("Unbind"))
								{
									Keybinds::Set(CurrentlyEditing, Keybind{});
									close = true;
								}

								/* i love imgui */
								ImGui::SameLine();
								ImGui::Spacing();
								ImGui::SameLine();
								ImGui::Spacing();
								ImGui::SameLine();
								ImGui::Spacing();
								ImGui::SameLine();
								/* i love imgui end*/

								if (ImGui::Button("Accept"))
								{
									if (overwriting)
									{
										Keybinds::Set(Keybinds::CurrentKeybindUsedBy, Keybind{});
									}
									Keybinds::Set(CurrentlyEditing, Keybinds::CurrentKeybind);
									close = true;
								}
								ImGui::SameLine();
								if (ImGui::Button("Cancel"))
								{
									close = true;
								}

								if (close)
								{
									CurrentlyEditing = "";
									Keybinds::CurrentKeybind = Keybind{};
									Keybinds::CurrentKeybindUsedBy = "";
									Keybinds::IsSettingKeybind = false;
									ImGui::CloseCurrentPopup();
								}

								ImGui::EndPopup();
							}

							ImGui::EndTable();
						}

						ImGui::EndChild();
					}

					ImGui::EndTabItem();
				}
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
									if (ImGui::Button(("Remove##"+key.Key).c_str())) // ##+key for unique id
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
				ImGui::EndTabBar();
			}
		}
		ImGui::End();
	}
}