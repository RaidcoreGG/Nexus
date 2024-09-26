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

	bool IsStylePanelOpen = false;
	std::vector<std::string> Fonts;

	float kbButtonWidth = 0.0f;

	struct InputBindPacked
	{
		std::string KeysText;
		ManagedInputBind Bind;
	};

	struct KBCat
	{
		std::string								Name;
		std::map<std::string, InputBindPacked>	InputBinds;
	};

	bool ShouldOpenInputBindModal = false;
	bool IsEditingGameBind = false;
	std::string InputBindModalTitle;

	bool IsInputBindsPanelOpen = false; // used to refetch binds
	std::vector<KBCat> InputBindCategoryMap;
	std::string CurrentlyEditing; // the identifier
	std::string CurrentlyEditingBind; // the current bind

	struct GameInputBindPacked
	{
		std::string Name;
		std::string KeysText;
		InputBind Bind;
	};

	struct GKBCat
	{
		std::string Name;
		std::map<EGameBinds, GameInputBindPacked> GameInputBinds;
	};

	bool IsGameInputBindsPanelOpen = false; // used to refetch binds
	EGameBinds CurrentlyEditingGame; // the identifier
	std::vector<GKBCat> GameInputBindCategoryMap;

	char CurrentAPIKey[73]{};

	/* proto tabs */
	void GeneralTab();
	bool AddonsTab();
	void StyleTab();
	bool InputBindsTab();
	void GameInputBindsTab();
	void APITab();
	void ChangelogTab();

	COptionsWindow::COptionsWindow(std::string aName)
	{
		Name = aName;
	}

	void DeleteStaleBind(std::string aIdentifier)
	{
		if (aIdentifier.empty()) { return; }

		std::thread([aIdentifier]() {
			InputBindApi->Delete(aIdentifier);
			IsInputBindsPanelOpen = false; // we set this so the display gets refreshed
		}).detach();
	}
	void DrawInputBindsTable(const std::map<std::string, InputBindPacked>& aInputBinds)
	{
		if (ImGui::BeginTable("table_inputbinds", 3, ImGuiTableFlags_BordersInnerH))
		{
			for (auto& [identifier, inputBind] : aInputBinds)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(Language->Translate(identifier.c_str()));

				ImGui::TableSetColumnIndex(1);
				ImGui::PushID(identifier.c_str());
				if (ImGui::Button(inputBind.KeysText.c_str(), ImVec2(kbButtonWidth, 0.0f)))
				{
					CurrentlyEditing = identifier;
					CurrentlyEditingBind = inputBind.KeysText;

					ShouldOpenInputBindModal = true;
					IsEditingGameBind = false;

					InputBindModalTitle = Language->Translate("((000062))");
					InputBindModalTitle.append(Language->Translate(CurrentlyEditing.c_str()));
					InputBindModalTitle.append("###inputbind_setter_modal");
				}
				ImGui::PopID();

				ImGui::TableSetColumnIndex(2);
				if (inputBind.Bind.Handler == nullptr)
				{
					if (ImGui::SmallButton(("X##" + identifier).c_str()))
					{
						DeleteStaleBind(identifier);
					}
					ImGui::SameLine();
					ImGui::TextDisabled(Language->Translate("((000061))"));
				}
			}

			ImGui::EndTable();
		}
	}
	void DrawGameBindsTable(const std::map<EGameBinds, GameInputBindPacked>& aInputBinds)
	{
		if (ImGui::BeginTable("table_gameinputbinds", 2, ImGuiTableFlags_BordersInnerH))
		{
			for (auto& [identifier, inputBind] : aInputBinds)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(Language->Translate(CGameBindsApi::ToString(identifier).c_str())); // get translation here

				ImGui::TableSetColumnIndex(1);
				ImGui::PushID(inputBind.Name.c_str());
				if (ImGui::Button(inputBind.KeysText.c_str(), ImVec2(kbButtonWidth, 0.0f)))
				{
					CurrentlyEditingGame = identifier;
					CurrentlyEditingBind = inputBind.KeysText;

					ShouldOpenInputBindModal = true;
					IsEditingGameBind = true;

					InputBindModalTitle = Language->Translate("((000062))");
					InputBindModalTitle.append(Language->Translate(CGameBindsApi::ToString(CurrentlyEditingGame).c_str()));
					InputBindModalTitle.append("###inputbind_setter_modal");
				}
				ImGui::PopID();
			}

			ImGui::EndTable();
		}
	}
	void DrawBindSetterModal()
	{
		if (ShouldOpenInputBindModal == true)
		{
			ImGui::OpenPopup(InputBindModalTitle.c_str());
			ShouldOpenInputBindModal = false;
		}

		ImVec2 center(Renderer::Width * 0.5f, Renderer::Height * 0.5f);
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal(InputBindModalTitle.c_str(), NULL, WindowFlags_Default))
		{
			InputBindApi->StartCapturing();

			InputBind currInputBind = InputBindApi->GetCapturedInputBind();
			std::string usedBy = InputBindApi->IsInUse(currInputBind);

			if (currInputBind == InputBind{})
			{
				ImGui::Text(CurrentlyEditingBind.c_str());
			}
			else
			{
				ImGui::Text(CInputBindApi::IBToString(currInputBind, true).c_str());
			}

			bool overwriting = false;

			if (usedBy != CurrentlyEditing && !usedBy.empty())
			{
				ImGui::TextColored(ImVec4(255, 0, 0, 255), (Language->Translate("((000063))") + usedBy + ".").c_str());
				overwriting = true;
			}

			bool close = false;

			if (ImGui::Button(Language->Translate("((000064))")))
			{
				if (IsEditingGameBind)
				{
					GameBindsApi->Set(CurrentlyEditingGame, InputBind{});
					IsGameInputBindsPanelOpen = false; // we set this so the display gets refreshed
				}
				else
				{
					InputBindApi->Set(CurrentlyEditing, InputBind{});
					IsInputBindsPanelOpen = false; // we set this so the display gets refreshed
				}
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

			if (ImGui::Button(Language->Translate("((000065))")))
			{
				if (IsEditingGameBind)
				{
					GameBindsApi->Set(CurrentlyEditingGame, currInputBind);
					IsGameInputBindsPanelOpen = false; // we set this so the display gets refreshed
				}
				else
				{
					if (overwriting)
					{
						InputBindApi->Set(usedBy, InputBind{});
					}

					InputBindApi->Set(CurrentlyEditing, currInputBind);
					IsInputBindsPanelOpen = false; // we set this so the display gets refreshed
				}
				
				close = true;
			}
			ImGui::SameLine();
			if (ImGui::Button(Language->Translate("((000066))")))
			{
				close = true;
			}

			if (close)
			{
				if (IsEditingGameBind)
				{
					CurrentlyEditingBind = "";
					CurrentlyEditingGame = (EGameBinds)-1;
				}
				else
				{
					CurrentlyEditing = "";
					CurrentlyEditingBind = "";
				}

				InputBindApi->EndCapturing();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void PopulateFonts()
	{
		if (!IsStylePanelOpen)
		{
			Fonts.clear();

			for (const std::filesystem::directory_entry entry : std::filesystem::directory_iterator(Index::D_GW2_ADDONS_NEXUS_FONTS))
			{
				std::filesystem::path path = entry.path();

				/* sanity checks */
				if (std::filesystem::is_directory(path)) { continue; }
				if (std::filesystem::file_size(path) == 0) { continue; }
				if (path.extension() == ".ttf")
				{
					Fonts.push_back(path.filename().string());
				}
			}
		}
	}
	void PopulateInputBinds()
	{
		if (!IsInputBindsPanelOpen)
		{
			InputBindCategoryMap.clear();

			/* copy of all InputBinds */
			std::map<std::string, ManagedInputBind> InputBindRegistry = InputBindApi->GetRegistry();

			/* acquire categories */
			for (auto& [identifier, inputBind] : InputBindRegistry)
			{
				std::string owner = Loader::GetOwner(inputBind.Handler);

				auto it = std::find_if(InputBindCategoryMap.begin(), InputBindCategoryMap.end(), [owner](KBCat category) { return category.Name == owner; });

				if (it == InputBindCategoryMap.end())
				{
					KBCat cat{};
					cat.Name = owner;
					cat.InputBinds[identifier] = {
						CInputBindApi::IBToString(inputBind.Bind, true),
						inputBind
					};
					InputBindCategoryMap.push_back(cat);
				}
				else
				{
					it->InputBinds[identifier] = {
						CInputBindApi::IBToString(inputBind.Bind, true),
						inputBind
					};
				}
			}
		}
	}
	void PopulateGameBinds()
	{
		if (!IsGameInputBindsPanelOpen)
		{
			GameInputBindCategoryMap.clear();

			/* copy of all InputBinds */
			std::map<EGameBinds, InputBind> InputBindRegistry = GameBindsApi->GetRegistry();

			/* acquire categories */
			for (auto& [identifier, inputBind] : InputBindRegistry)
			{
				std::string catName = CGameBindsApi::GetCategory(identifier);

				auto it = std::find_if(GameInputBindCategoryMap.begin(), GameInputBindCategoryMap.end(), [catName](GKBCat category) { return category.Name == catName; });

				if (it == GameInputBindCategoryMap.end())
				{
					GKBCat cat{};
					cat.Name = catName;
					cat.GameInputBinds[identifier] = {
						CGameBindsApi::ToString(identifier),
						CInputBindApi::IBToString(inputBind, true),
						inputBind
					};
					GameInputBindCategoryMap.push_back(cat);
				}
				else
				{
					it->GameInputBinds[identifier] = {
						CGameBindsApi::ToString(identifier),
						CInputBindApi::IBToString(inputBind, true),
						inputBind
					};
				}
			}
		}
	}

	void COptionsWindow::Render()
	{
		if (!Visible) { return; }

		ImGui::SetNextWindowSize(ImVec2(owWidth * ImGui::GetFontSize(), owHeight * ImGui::GetFontSize()), ImGuiCond_FirstUseEver);
		if (ImGui::Begin(Name.c_str(), &Visible, ImGuiWindowFlags_NoCollapse))
		{
			if (ImGui::BeginTabBar("OptionsTabBar", ImGuiTabBarFlags_None))
			{
				kbButtonWidth = ImGui::CalcTextSize("XXXXXXXXXXXXXXXXXXXXXXXX").x;

				GeneralTab();
				bool addonsActive = AddonsTab();
				StyleTab();
				bool kbActive = InputBindsTab();
				GameInputBindsTab();
				//APITab();
				ChangelogTab();

				if (!addonsActive && !kbActive)
				{
					IsInputBindsPanelOpen = false;
				}

				ImGui::EndTabBar();
			}
		}
		ImGui::End();

		DrawBindSetterModal();
	}

	void LanguageSettings()
	{
		/* header */
		ImGui::TextDisabled(Language->Translate("((000041))"));

		/* prefetch active lang */
		std::string activeLang = Language->GetActiveLanguage();

		/* selector dropdown */
		if (ImGui::BeginCombo("##languageselector", activeLang.c_str()))
		{
			for (std::string lang : Language->GetLanguages())
			{
				if (ImGui::Selectable(lang.c_str(), lang == activeLang))
				{
					if (lang != activeLang)
					{
						Language->SetLanguage(lang);
						Settings::Settings[OPT_LANGUAGE] = lang;
						Settings::Save();
					}
				}
			}
			ImGui::EndCombo();
		}
	}
	void UIUXSettings()
	{
		/* header */
		ImGui::TextDisabled(Language->Translate("((000042))"));
		
		/* close menu after selecting item */
		if (ImGui::Checkbox(Language->Translate("((000043))"), &GUI::CloseMenuAfterSelecting))
		{
			Settings::Settings[OPT_CLOSEMENU] = GUI::CloseMenuAfterSelecting;
			Settings::Save();
		}

		/* close windows on escape */
		if (ImGui::Checkbox(Language->Translate("((000044))"), &GUI::CloseOnEscape))
		{
			Settings::Settings[OPT_CLOSEESCAPE] = GUI::CloseOnEscape;
			Settings::Save();
		}

		/* show addons window after after addons were disabled because of an update */
		if (ImGui::Checkbox(Language->Translate("((000086))"), &GUI::ShowAddonsWindowAfterDUU))
		{
			Settings::Settings[OPT_SHOWADDONSWINDOWAFTERDUU] = GUI::ShowAddonsWindowAfterDUU;
			Settings::Save();
		}
	}
	void QuickAccessSettings()
	{
		/* header */
		ImGui::TextDisabled(Language->Translate("((000045))"));
		
		/* toggle vertical layout */
		if (ImGui::Checkbox(Language->Translate("((000046))"), &QuickAccess::VerticalLayout))
		{
			Settings::Settings[OPT_QAVERTICAL] = QuickAccess::VerticalLayout;
			Settings::Save();
		}

		/* prefetch currently selected position string */
		std::string qaVisStr = QuickAccess::EQAVisibilityToString(QuickAccess::Visibility);
		EQAVisibility newQaVis = QuickAccess::Visibility;

		ImGui::Text(Language->Translate("((000097))"));
		if (ImGui::BeginCombo("##qavisibility", Language->Translate(qaVisStr.c_str())))
		{
			if (ImGui::Selectable(Language->Translate("((000047))"), qaVisStr == "((000047))"))
			{
				newQaVis = EQAVisibility::AlwaysShow;
			}
			if (ImGui::Selectable(Language->Translate("((000093))"), qaVisStr == "((000093))"))
			{
				newQaVis = EQAVisibility::Gameplay;
			}
			if (ImGui::Selectable(Language->Translate("((000094))"), qaVisStr == "((000094))"))
			{
				newQaVis = EQAVisibility::OutOfCombat;
			}
			if (ImGui::Selectable(Language->Translate("((000095))"), qaVisStr == "((000095))"))
			{
				newQaVis = EQAVisibility::InCombat;
			}
			if (ImGui::Selectable(Language->Translate("((000096))"), qaVisStr == "((000096))"))
			{
				newQaVis = EQAVisibility::Hide;
			}

			ImGui::EndCombo();
		}

		/* save if qaVis was changed */
		if (QuickAccess::Visibility != newQaVis)
		{
			QuickAccess::Visibility = newQaVis;
			Settings::Settings[OPT_QAVISIBILITY] = QuickAccess::Visibility;
			Settings::Save();
		}

		/* show notification icon on update */
		if (ImGui::Checkbox(Language->Translate("((000049))"), &GUI::NotifyChangelog))
		{
			Settings::Settings[OPT_NOTIFYCHANGELOG] = GUI::NotifyChangelog;
			Settings::Save();
		}

		/* prefetch currently selected position string */
		std::string qaPosStr = QuickAccess::EQAPositionToString(QuickAccess::Location);
		EQAPosition newQaPos = QuickAccess::Location;

		/* position/location dropdown */
		ImGui::Text(Language->Translate("((000050))"));
		if (ImGui::BeginCombo("##qalocation", Language->Translate(qaPosStr.c_str())))
		{
			if (ImGui::Selectable(Language->Translate("((000067))"), qaPosStr == "((000067))"))
			{
				newQaPos = EQAPosition::Extend;
			}
			if (ImGui::Selectable(Language->Translate("((000068))"), qaPosStr == "((000068))"))
			{
				newQaPos = EQAPosition::Under;
			}
			if (ImGui::Selectable(Language->Translate("((000069))"), qaPosStr == "((000069))"))
			{
				newQaPos = EQAPosition::Bottom;
			}
			if (ImGui::Selectable(Language->Translate("((000070))"), qaPosStr == "((000070))"))
			{
				newQaPos = EQAPosition::Custom;
			}

			ImGui::EndCombo();
		}

		/* save if qaPos was changed */
		if (QuickAccess::Location != newQaPos)
		{
			QuickAccess::Location = newQaPos;
			Settings::Settings[OPT_QALOCATION] = QuickAccess::Location;
			Settings::Save();
		}

		/* offset */
		ImGui::Text(Language->Translate("((000051))"));
		if (ImGui::DragFloat2("##qaoffset", (float*)&QuickAccess::Offset, 1.0f, static_cast<float>(Renderer::Height) * -1.0f, static_cast<float>(Renderer::Height)))
		{
			Settings::Settings[OPT_QAOFFSETX] = QuickAccess::Offset.x;
			Settings::Settings[OPT_QAOFFSETY] = QuickAccess::Offset.y;
			Settings::Save();
		}
	}

	void GeneralTab()
	{
		if (ImGui::BeginTabItem(Language->Translate("((000052))")))
		{
			ImGui::BeginChild("##GeneralTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

			LanguageSettings();
			UIUXSettings();
			QuickAccessSettings();

			ImGui::EndChild();

			ImGui::EndTabItem();
		}
	}

	bool AddonsTab()
	{
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

	void StyleTab()
	{
		if (ImGui::BeginTabItem(Language->Translate("((000053))")))
		{
			PopulateFonts();

			IsStylePanelOpen = true;

			ImGui::BeginChild("##StyleTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

			ImGui::TextDisabled(Language->Translate("((000092))"));
			if (ImGui::BeginCombo("##fontselector", GUI::FontFile.c_str()))
			{
				for (std::string font : Fonts)
				{
					if (ImGui::Selectable(font.c_str(), font == GUI::FontFile))
					{
						if (font != FontFile)
						{
							FontFile = font;
							
							GUI::LoadFonts();

							Settings::Settings[OPT_USERFONT] = FontFile;
							Settings::Save();
						}
					}
				}
				ImGui::EndCombo();
			}

			ImGui::TextDisabled(Language->Translate("((000054))"));
			if (ImGui::InputFloat("##fontsize", &FontSize, 0.0f, 0.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue))
			{
				FontSize = min(max(FontSize, 1.0f), 50.0f);

				Settings::Settings[OPT_FONTSIZE] = FontSize;
				Settings::Save();

				FontManager.ResizeFont("USER_FONT", FontSize);
			}

			ImGuiIO& io = ImGui::GetIO();
			ImGui::TextDisabled(Language->Translate("((000072))"));
			if (ImGui::DragFloat("##globalscale", &io.FontGlobalScale, 0.005f, 0.75f, 3.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
			{
				Settings::Settings[OPT_GLOBALSCALE] = io.FontGlobalScale;
				Settings::Save();

				GUI::Rescale();
			}

			ImGui::Separator();

			if (ImGui::Checkbox(Language->Translate("((000056))"), &GUI::LinkArcDPSStyle))
			{
				Settings::Settings[OPT_LINKARCSTYLE] = GUI::LinkArcDPSStyle;
				ImportArcDPSStyle();
				Settings::Save();
			}
			ImGui::TooltipGeneric(Language->Translate("((000057))"));

			if (!GUI::LinkArcDPSStyle)
			{
				ImGuiStyle& style = ImGui::GetStyle();

				if (ImGui::SmallButton(Language->Translate("((000058))")))
				{
					std::string encode = Base64::Encode((unsigned char*)&style, sizeof(ImGuiStyle));
					Settings::Settings[OPT_IMGUISTYLE] = encode;
					Settings::Save();

					encode = Base64::Encode((unsigned char*)&style.Colors[0], sizeof(ImVec4) * ImGuiCol_COUNT);
					Settings::Settings[OPT_IMGUICOLORS] = encode;
					Settings::Save();
				}
				ImGui::SameLine();
				if (ImGui::SmallButton(Language->Translate("((000059))")))
				{
					if (!Settings::Settings[OPT_IMGUISTYLE].is_null())
					{
						std::string style64 = Settings::Settings[OPT_IMGUISTYLE].get<std::string>();
						std::string decode = Base64::Decode(&style64[0], style64.length());
						memcpy(&style, &decode[0], decode.length());
					}

					if (!Settings::Settings[OPT_IMGUICOLORS].is_null())
					{
						std::string colors64 = Settings::Settings[OPT_IMGUICOLORS].get<std::string>();
						std::string decode = Base64::Decode(&colors64[0], colors64.length());
						memcpy(&style.Colors[0], &decode[0], decode.length());
					}
				}

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

			ImGui::EndTabItem();
		}
		else
		{
			IsStylePanelOpen = false;
		}
	}

	bool InputBindsTab()
	{
		bool isActive = false;

		if (ImGui::BeginTabItem(Language->Translate("((000060))")))
		{
			isActive = true;

			PopulateInputBinds();

			IsInputBindsPanelOpen = true;

			ImGui::BeginChild("##InputBindsTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

			for (KBCat cat : InputBindCategoryMap)
			{
				if (ImGui::CollapsingHeader(cat.Name != NULLSTR ? cat.Name.c_str() : Language->Translate("((000088))"), ImGuiTreeNodeFlags_DefaultOpen))
				{
					DrawInputBindsTable(cat.InputBinds);
				}
			}

			ImGui::EndChild();

			ImGui::EndTabItem();
		}
		
		return isActive;
	}

	void GameInputBindsTab()
	{
		if (ImGui::BeginTabItem(Language->Translate("((000091))")))
		{
			PopulateGameBinds();
			
			IsGameInputBindsPanelOpen = true;

			ImGui::BeginChild("##GameBindsTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f));

			for (GKBCat cat : GameInputBindCategoryMap)
			{
				if (ImGui::CollapsingHeader(Language->Translate(cat.Name.c_str()), ImGuiTreeNodeFlags_DefaultOpen))
				{
					DrawGameBindsTable(cat.GameInputBinds);
				}
			}
			
			ImGui::EndChild();

			ImGui::EndTabItem();
		}
		else
		{
			IsGameInputBindsPanelOpen = false;
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