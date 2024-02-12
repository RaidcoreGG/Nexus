#include "OptionsWindow.h"

#include <regex>

#include "Shared.h"
#include "Paths.h"
#include "State.h"
#include "Renderer.h"

#include "Keybinds/KeybindHandler.h"
#include "API/APIController.h"
#include "Settings/Settings.h"

#include "GUI/GUI.h"
#include "GUI/IWindow.h"
#include "GUI/Widgets/QuickAccess/QuickAccess.h"

#include "imgui.h"
#include "imgui_extensions.h"

namespace GUI
{
	float owWidth = 30.0f;
	float owHeight = 24.0f;

	std::string CurrentlyEditing;
	char CurrentAPIKey[73]{};

	const char* qaLocations[] = { "Extend", "Under", "Bottom", "Custom" };

	/* proto tabs */
	void GeneralTab();
	void AddonsTab();
	void StyleTab();
	void KeybindsTab();
	void APITab();

	OptionsWindow::OptionsWindow(std::string aName)
	{
		Name = aName;
	}

	void OptionsWindow::Render()
	{
		if (!Visible) { return; }

		ImGui::SetNextWindowSize(ImVec2(owWidth * ImGui::GetFontSize(), owHeight * ImGui::GetFontSize()), ImGuiCond_FirstUseEver);
		if (ImGui::Begin(Name.c_str(), &Visible, ImGuiWindowFlags_NoCollapse))
		{
			if (ImGui::BeginTabBar("OptionsTabBar", ImGuiTabBarFlags_None))
			{
				GeneralTab();
				AddonsTab();
				StyleTab();
				KeybindsTab();
				//APITab();

				ImGui::EndTabBar();
			}
		}
		ImGui::End();
	}

	void GeneralTab()
	{
		if (ImGui::BeginTabItem("General"))
		{
			{
				ImGui::BeginChild("##GeneralTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

				ImGui::TextDisabled("UI/UX");
				{
					if (ImGui::Checkbox("Close Menu after selecting item", &GUI::CloseMenuAfterSelecting))
					{
						Settings::Settings[OPT_CLOSEMENU] = GUI::CloseMenuAfterSelecting;
						Settings::Save();
					}
					if (ImGui::Checkbox("Closing Windows with Escape", &GUI::CloseOnEscape))
					{
						Settings::Settings[OPT_CLOSEESCAPE] = GUI::CloseOnEscape;
						Settings::Save();
					}
				}

				ImGui::Separator();

				ImGui::TextDisabled("Quick Access");
				{
					if (ImGui::Checkbox("Vertical Layout", &QuickAccess::VerticalLayout))
					{
						Settings::Settings[OPT_QAVERTICAL] = QuickAccess::VerticalLayout;
						Settings::Save();
					}
					if (ImGui::Checkbox("Show notification icon when Nexus updates", &GUI::NotifyChangelog))
					{
						Settings::Settings[OPT_NOTIFYCHANGELOG] = GUI::NotifyChangelog;
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
				}

				ImGui::EndChild();
			}

			ImGui::EndTabItem();
		}
	}

	void AddonsTab()
	{
		if (ImGui::BeginTabItem("Addons"))
		{
			{
				ImGui::BeginChild("##AddonsTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

				for (GUI_RENDER renderCb : RegistryOptionsRender)
				{
					if (renderCb)
					{
						renderCb();
						ImGui::Separator();
					}
				}

				ImGui::EndChild();
			}

			ImGui::EndTabItem();
		}
	}

	void StyleTab()
	{
		if (ImGui::BeginTabItem("Style"))
		{
			{
				ImGui::BeginChild("##StyleTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

				ImGui::TextDisabled("Font Size");
				{
					if (ImGui::InputFloat("##fontsize", &FontSize, 0.0f, 0.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue))
					{
						if (FontSize < 1.0f)
						{
							FontSize = 1.0f;
						}
						Settings::Settings[OPT_FONTSIZE] = FontSize;
						Settings::Save();
					}
					ImGui::TooltipGeneric("Changing font size requires a restart.");
				}

				ImGui::Separator();

				if (ImGui::Checkbox("Link to ArcDPS style", &GUI::LinkArcDPSStyle))
				{
					Settings::Settings[OPT_LINKARCSTYLE] = GUI::LinkArcDPSStyle;
					ImportArcDPSStyle();
					Settings::Save();
				}

				if (!GUI::LinkArcDPSStyle)
				{
					ImGuiStyle& style = ImGui::GetStyle();
					ImGuiStyle ref_saved_style = style;
					ImGuiStyle* ref = &ref_saved_style;

					if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None))
					{
						if (ImGui::BeginTabItem("Sizes"))
						{
							ImGui::Text("Main");
							ImGui::SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
							ImGui::SliderFloat2("FramePadding", (float*)&style.FramePadding, 0.0f, 20.0f, "%.0f");
							ImGui::SliderFloat2("CellPadding", (float*)&style.CellPadding, 0.0f, 20.0f, "%.0f");
							ImGui::SliderFloat2("ItemSpacing", (float*)&style.ItemSpacing, 0.0f, 20.0f, "%.0f");
							ImGui::SliderFloat2("ItemInnerSpacing", (float*)&style.ItemInnerSpacing, 0.0f, 20.0f, "%.0f");
							ImGui::SliderFloat2("TouchExtraPadding", (float*)&style.TouchExtraPadding, 0.0f, 10.0f, "%.0f");
							ImGui::SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f, "%.0f");
							ImGui::SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f, "%.0f");
							ImGui::SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.0f");
							ImGui::Text("Borders");
							ImGui::SliderFloat("WindowBorderSize", &style.WindowBorderSize, 0.0f, 1.0f, "%.0f");
							ImGui::SliderFloat("ChildBorderSize", &style.ChildBorderSize, 0.0f, 1.0f, "%.0f");
							ImGui::SliderFloat("PopupBorderSize", &style.PopupBorderSize, 0.0f, 1.0f, "%.0f");
							ImGui::SliderFloat("FrameBorderSize", &style.FrameBorderSize, 0.0f, 1.0f, "%.0f");
							ImGui::SliderFloat("TabBorderSize", &style.TabBorderSize, 0.0f, 1.0f, "%.0f");
							ImGui::Text("Rounding");
							ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f, "%.0f");
							ImGui::SliderFloat("ChildRounding", &style.ChildRounding, 0.0f, 12.0f, "%.0f");
							ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f");
							ImGui::SliderFloat("PopupRounding", &style.PopupRounding, 0.0f, 12.0f, "%.0f");
							ImGui::SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f, 12.0f, "%.0f");
							ImGui::SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 12.0f, "%.0f");
							ImGui::SliderFloat("LogSliderDeadzone", &style.LogSliderDeadzone, 0.0f, 12.0f, "%.0f");
							ImGui::SliderFloat("TabRounding", &style.TabRounding, 0.0f, 12.0f, "%.0f");
							ImGui::Text("Alignment");
							ImGui::SliderFloat2("WindowTitleAlign", (float*)&style.WindowTitleAlign, 0.0f, 1.0f, "%.2f");
							int window_menu_button_position = style.WindowMenuButtonPosition + 1;
							if (ImGui::Combo("WindowMenuButtonPosition", (int*)&window_menu_button_position, "None\0Left\0Right\0"))
								style.WindowMenuButtonPosition = window_menu_button_position - 1;
							ImGui::Combo("ColorButtonPosition", (int*)&style.ColorButtonPosition, "Left\0Right\0");
							ImGui::SliderFloat2("ButtonTextAlign", (float*)&style.ButtonTextAlign, 0.0f, 1.0f, "%.2f");
							ImGui::SameLine(); ImGui::HelpMarker("Alignment applies when a button is larger than its text content.");
							ImGui::SliderFloat2("SelectableTextAlign", (float*)&style.SelectableTextAlign, 0.0f, 1.0f, "%.2f");
							ImGui::SameLine(); ImGui::HelpMarker("Alignment applies when a selectable is larger than its text content.");
							ImGui::Text("Safe Area Padding");
							ImGui::SameLine(); ImGui::HelpMarker("Adjust if you cannot see the edges of your screen (e.g. on a TV where scaling has not been configured).");
							ImGui::SliderFloat2("DisplaySafeAreaPadding", (float*)&style.DisplaySafeAreaPadding, 0.0f, 30.0f, "%.0f");
							ImGui::EndTabItem();
						}

						if (ImGui::BeginTabItem("Colors"))
						{
							static ImGuiTextFilter filter;
							filter.Draw("Filter colors", ImGui::GetFontSize() * 16);

							ImGui::BeginChild("##colors", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NavFlattened);
							ImGui::PushItemWidth(-160);
							for (int i = 0; i < ImGuiCol_COUNT; i++)
							{
								const char* name = ImGui::GetStyleColorName(i);
								if (!filter.PassFilter(name))
									continue;
								ImGui::PushID(i);
								ImGui::ColorEdit4("##color", (float*)&style.Colors[i], ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
								if (memcmp(&style.Colors[i], &ref->Colors[i], sizeof(ImVec4)) != 0)
								{
									// Tips: in a real user application, you may want to merge and use an icon font into the main font,
									// so instead of "Save"/"Revert" you'd use icons!
									// Read the FAQ and docs/FONTS.md about using icon fonts. It's really easy and super convenient!
									ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Save")) { ref->Colors[i] = style.Colors[i]; }
									ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Revert")) { style.Colors[i] = ref->Colors[i]; }
								}
								ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
								ImGui::TextUnformatted(name);
								ImGui::PopID();
							}
							ImGui::PopItemWidth();
							ImGui::EndChild();

							ImGui::EndTabItem();
						}

						if (ImGui::BeginTabItem("Rendering"))
						{
							ImGui::Checkbox("Anti-aliased lines", &style.AntiAliasedLines);
							ImGui::SameLine();
							ImGui::HelpMarker("When disabling anti-aliasing lines, you'll probably want to disable borders in your style as well.");

							ImGui::Checkbox("Anti-aliased lines use texture", &style.AntiAliasedLinesUseTex);
							ImGui::SameLine();
							ImGui::HelpMarker("Faster lines using texture data. Require backend to render with bilinear filtering (not point/nearest filtering).");

							ImGui::Checkbox("Anti-aliased fill", &style.AntiAliasedFill);
							ImGui::PushItemWidth(100);
							ImGui::DragFloat("Curve Tessellation Tolerance", &style.CurveTessellationTol, 0.02f, 0.10f, 10.0f, "%.2f");
							if (style.CurveTessellationTol < 0.10f) style.CurveTessellationTol = 0.10f;

							// When editing the "Circle Segment Max Error" value, draw a preview of its effect on auto-tessellated circles.
							ImGui::DragFloat("Circle Segment Max Error", &style.CircleSegmentMaxError, 0.01f, 0.10f, 10.0f, "%.2f");
							if (ImGui::IsItemActive())
							{
								ImGui::SetNextWindowPos(ImGui::GetCursorScreenPos());
								ImGui::BeginTooltip();
								ImVec2 p = ImGui::GetCursorScreenPos();
								ImDrawList* draw_list = ImGui::GetWindowDrawList();
								float RAD_MIN = 10.0f, RAD_MAX = 80.0f;
								float off_x = 10.0f;
								for (int n = 0; n < 7; n++)
								{
									const float rad = RAD_MIN + (RAD_MAX - RAD_MIN) * (float)n / (7.0f - 1.0f);
									draw_list->AddCircle(ImVec2(p.x + off_x + rad, p.y + RAD_MAX), rad, ImGui::GetColorU32(ImGuiCol_Text), 0);
									off_x += 10.0f + rad * 2.0f;
								}
								ImGui::Dummy(ImVec2(off_x, RAD_MAX * 2.0f));
								ImGui::EndTooltip();
							}
							ImGui::SameLine();
							ImGui::HelpMarker("When drawing circle primitives with \"num_segments == 0\" tesselation will be calculated automatically.");

							ImGui::DragFloat("Global Alpha", &style.Alpha, 0.005f, 0.20f, 1.0f, "%.2f"); // Not exposing zero here so user doesn't "lose" the UI (zero alpha clips all widgets). But application code could have a toggle to switch between zero and non-zero.
							ImGui::PopItemWidth();

							ImGui::EndTabItem();
						}

						ImGui::EndTabBar();
					}
				}

				ImGui::EndChild();
			}

			ImGui::EndTabItem();
		}
	}

	void KeybindsTab()
	{
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
							if (ImGui::Button((keybind.Bind.ToString(true) + "##" + identifier).c_str(), ImVec2(kbButtonWidth, 0.0f)))
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
					if (ImGui::BeginPopupModal(("Set Keybind: " + CurrentlyEditing).c_str(), NULL, WindowFlags_Default))
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
}