#include "OptionsWindow.h"

#include "../../GUI.h"

namespace GUI
{
	std::string CurrentlyEditing;
	char CurrentAPIKey[73]{};

	void OptionsWindow::Render()
	{
		if (!Visible) { return; }

		ImGui::SetNextWindowSize(ImVec2(480.0f, 380.0f));
		if (ImGui::Begin("Options", &Visible, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
		{
			if (ImGui::BeginTabBar("OptionsTabBar", ImGuiTabBarFlags_None))
			{
				if (ImGui::BeginTabItem("General"))
				{
					{
						ImGui::BeginChild("##GeneralTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

						// new item

						ImGui::EndChild();
					}

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Style"))
				{
					{
						ImGui::BeginChild("##StyleTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

						ImGui::Text("Font Size: ");
						ImGui::SameLine();
						ImGui::InputFloat("##fontsize", &GUI::FontSize);
						ImGui::TooltipGeneric("Changing font size requires a restart. You can preview different fonts and sizes with the \"Preview\" button.");

						ImGui::EndChild();
					}

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Keybinds"))
				{
					{
						ImGui::BeginChild("##KeybindsTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

						float kbButtonWidth = ImGui::CalcTextSize("XXXXXXXXXXXXXXXX").x;

						if (ImGui::BeginTable("table_keybinds", 2, ImGuiTableFlags_BordersInnerH))
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

						ImGui::InputText("##currentapikey", CurrentAPIKey, 73);
						ImGui::SameLine();
						if (ImGui::Button("Add"))
						{
							std::string str = CurrentAPIKey;

							if (std::regex_match(str, std::regex("[A-F\\d]{8}-([A-F\\d]{4}-){3}[A-F\\d]{20}(-[A-F\\d]{4}){3}-[A-F\\d]{12}")))
							{
								API::AddKey(str);
							}
						}

						bool shouldSave = false;

						if (ImGui::BeginTable("table_apikeys", 2, ImGuiTableFlags_BordersInnerH))
						{
							API::Mutex.lock();
							{
								for (std::string key : API::APIKeys)
								{
									ImGui::TableNextRow();
									ImGui::TableSetColumnIndex(0);
									ImGui::Text("%.18s", key.c_str());

									ImGui::TableSetColumnIndex(1);
									if (ImGui::Button(("Remove##"+key).c_str())) // ##+key for unique id
									{
										// copy paste of the API::Save() code because of mutex shenanigans
										API::APIKeys.erase(std::find(API::APIKeys.begin(), API::APIKeys.end(), key));
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