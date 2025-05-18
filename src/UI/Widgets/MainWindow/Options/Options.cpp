///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Options.cpp
/// Description  :  Contains the content of the options window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Options.h"

#include <shellapi.h>

#include "imgui/imgui.h"
#include "imgui_extensions.h"

#include "Consts.h"
#include "Context.h"
#include "Index.h"
#include "Renderer.h"
#include "resource.h"
#include "Util/Base64.h"
#include "Util/Time.h"

COptionsWindow::COptionsWindow()
{
	this->Name           = "Options";
	this->DisplayName    = "((000004))";
	this->IconIdentifier = "ICON_OPTIONS";
	this->IconID         = RES_ICON_OPTIONS;
	this->IsHost         = true;

	this->ExportModal    = {};
	this->ImportModal    = {};

	this->Invalidate();
}

void COptionsWindow::RenderContent()
{
	if (this->IsInvalid)
	{
		static CContext* ctx = CContext::GetContext();
		static CUiContext* uictx = ctx->GetUIContext();
		static CEscapeClosing* escclose = uictx->GetEscapeClosingService();

		escclose->Deregister(this->GetVisibleStatePtr());
		escclose->Register(this->GetNameID().c_str(), this->GetVisibleStatePtr());

		this->PopulateFonts();
		this->PopulateStyles();
		this->IsInvalid = false;
	}

	if (ImGui::BeginTabBar("OptionsTabBar", ImGuiTabBarFlags_None))
	{
		this->TabGeneral();
		this->TabStyle();

		ImGui::EndTabBar();
	}
}

void COptionsWindow::RenderSubWindows()
{
	this->ExportModal.Render();

	if (this->ImportModal.Render() && this->ImportModal.GetResult() == EModalResult::OK)
	{
		this->HasUnsavedStyle = true;
	}
}

void COptionsWindow::TabGeneral()
{
	static CContext* ctx = CContext::GetContext();
	static CLocalization* langApi = ctx->GetLocalization();
	static CSettings* settingsctx = ctx->GetSettingsCtx();
	static CUiContext* uictx = ctx->GetUIContext();
	static CQuickAccess* qactx = uictx->GetQuickAccess();

	if (ImGui::BeginTabItem(langApi->Translate("((000052))")))
	{
		if (ImGui::BeginChild("Content", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f), false, ImGuiWindowFlags_NoBackground))
		{
			{
				ImGui::BeginGroupPanel(langApi->Translate("((000041))"), ImVec2(-1.0f, 0.0f));

				/* prefetch active lang */
				std::string activeLang = langApi->GetActiveLanguage();

				/* selector dropdown */
				if (ImGui::BeginCombo("##LanguageSelector", activeLang.c_str()))
				{
					for (std::string lang : langApi->GetLanguages())
					{
						if (ImGui::Selectable(lang.c_str(), lang == activeLang))
						{
							if (lang != activeLang)
							{
								langApi->SetLanguage(lang);
								settingsctx->Set(OPT_LANGUAGE, lang);
							}
						}
					}
					ImGui::EndCombo();
				}

				ImGui::EndGroupPanel();
			}

			{
				ImGui::BeginGroupPanel(langApi->Translate("((000042))"), ImVec2(-1.0f, 0.0f));

				static bool dpiScaling = settingsctx->Get<bool>(OPT_DPISCALING, true);
				if (ImGui::Checkbox(langApi->Translate("((000113))"), &dpiScaling))
				{
					settingsctx->Set(OPT_DPISCALING, dpiScaling);
					uictx->UpdateScaling();
				}

				/* fix gw2's jumping cursor */
				static bool lockHiddenCursor = settingsctx->Get<bool>(OPT_CAMCTRL_LOCKCURSOR, false);
				if (ImGui::Checkbox(langApi->Translate("((000112))"), &lockHiddenCursor))
				{
					settingsctx->Set(OPT_CAMCTRL_LOCKCURSOR, lockHiddenCursor);
				}

				static bool resetHiddenCursor = settingsctx->Get<bool>(OPT_CAMCTRL_RESETCURSOR, false);
				if (ImGui::Checkbox(langApi->Translate("((000114))"), &resetHiddenCursor))
				{
					settingsctx->Set(OPT_CAMCTRL_RESETCURSOR, resetHiddenCursor);
				}

				/* close windows on escape */
				static bool closeOnEscape = settingsctx->Get<bool>(OPT_CLOSEESCAPE, true);
				if (ImGui::Checkbox(langApi->Translate("((000044))"), &closeOnEscape))
				{
					settingsctx->Set(OPT_CLOSEESCAPE, closeOnEscape);
				}

				/* show addons window after after addons were disabled because of an update */
				static bool showAddonsWindowOnGameUpdate = settingsctx->Get<bool>(OPT_SHOWADDONSWINDOWAFTERDUU, false);
				if (ImGui::Checkbox(langApi->Translate("((000086))"), &showAddonsWindowOnGameUpdate))
				{
					settingsctx->Set(OPT_SHOWADDONSWINDOWAFTERDUU, showAddonsWindowOnGameUpdate);
				}

				static bool disableFestiveFlair = settingsctx->Get<bool>(OPT_DISABLEFESTIVEFLAIR, false);
				if (ImGui::Checkbox(langApi->Translate("((000101))"), &disableFestiveFlair))
				{
					settingsctx->Set(OPT_DISABLEFESTIVEFLAIR, disableFestiveFlair);
				}

				ImGui::EndGroupPanel();
			}

			{
				ImGui::BeginGroupPanel(langApi->Translate("((000045))"), ImVec2(-1.0f, 0.0f));

				/* toggle vertical layout */
				if (ImGui::Checkbox(langApi->Translate("((000046))"), &qactx->VerticalLayout))
				{
					settingsctx->Set(OPT_QAVERTICAL, qactx->VerticalLayout);
				}

				/* prefetch currently selected position string */
				std::string qaVisStr = qactx->EQAVisibilityToString(qactx->Visibility);
				EQAVisibility newQaVis = qactx->Visibility;

				ImGui::Text(langApi->Translate("((000097))"));
				if (ImGui::BeginCombo("##QAVisibilitySelector", langApi->Translate(qaVisStr.c_str())))
				{
					if (ImGui::Selectable(langApi->Translate("((000047))"), qaVisStr == "((000047))"))
					{
						newQaVis = EQAVisibility::AlwaysShow;
					}
					if (ImGui::Selectable(langApi->Translate("((000093))"), qaVisStr == "((000093))"))
					{
						newQaVis = EQAVisibility::Gameplay;
					}
					if (ImGui::Selectable(langApi->Translate("((000094))"), qaVisStr == "((000094))"))
					{
						newQaVis = EQAVisibility::OutOfCombat;
					}
					if (ImGui::Selectable(langApi->Translate("((000095))"), qaVisStr == "((000095))"))
					{
						newQaVis = EQAVisibility::InCombat;
					}
					if (ImGui::Selectable(langApi->Translate("((000096))"), qaVisStr == "((000096))"))
					{
						newQaVis = EQAVisibility::Hide;
					}

					ImGui::EndCombo();
				}

				/* save if qaVis was changed */
				if (qactx->Visibility != newQaVis)
				{
					qactx->Visibility = newQaVis;
					settingsctx->Set(OPT_QAVISIBILITY, qactx->Visibility);
				}

				/* show notification icon on update */
				static bool notifyChangelog = settingsctx->Get<bool>(OPT_NOTIFYCHANGELOG, false);
				if (ImGui::Checkbox(langApi->Translate("((000049))"), &notifyChangelog))
				{
					settingsctx->Set(OPT_NOTIFYCHANGELOG, notifyChangelog);
				}

				/* prefetch currently selected position string */
				std::string qaPosStr = qactx->EQAPositionToString(qactx->Location);
				EQAPosition newQaPos = qactx->Location;

				/* position/location dropdown */
				ImGui::Text(langApi->Translate("((000050))"));
				if (ImGui::BeginCombo("##QALocationSelector", langApi->Translate(qaPosStr.c_str())))
				{
					if (ImGui::Selectable(langApi->Translate("((000067))"), qaPosStr == "((000067))"))
					{
						newQaPos = EQAPosition::Extend;
					}
					if (ImGui::Selectable(langApi->Translate("((000068))"), qaPosStr == "((000068))"))
					{
						newQaPos = EQAPosition::Under;
					}
					if (ImGui::Selectable(langApi->Translate("((000069))"), qaPosStr == "((000069))"))
					{
						newQaPos = EQAPosition::Bottom;
					}
					if (ImGui::Selectable(langApi->Translate("((000070))"), qaPosStr == "((000070))"))
					{
						newQaPos = EQAPosition::Custom;
					}

					ImGui::EndCombo();
				}

				/* save if qaPos was changed */
				if (qactx->Location != newQaPos)
				{
					qactx->Location = newQaPos;
					settingsctx->Set(OPT_QALOCATION, qactx->Location);
				}

				/* offset */
				ImGui::Text(langApi->Translate("((000051))"));
				if (ImGui::DragFloat2("##QAOffsetInput", (float*)&qactx->Offset, 1.0f, (static_cast<int>(Renderer::Height)) * -1, static_cast<int>(Renderer::Height)))
				{
					settingsctx->Set(OPT_QAOFFSETX, qactx->Offset.x);
					settingsctx->Set(OPT_QAOFFSETY, qactx->Offset.y);
				}
				ImGui::EndGroupPanel();
			}
		}

		ImGui::EndChild();

		ImGui::EndTabItem();
	}
}

void COptionsWindow::TabStyle()
{
	CContext* ctx = CContext::GetContext();
	CLocalization* langApi = ctx->GetLocalization();
	CSettings* settingsctx = ctx->GetSettingsCtx();
	CUiContext* uictx = ctx->GetUIContext();

	if (ImGui::BeginTabItem(langApi->Translate("((000053))")))
	{
		if (ImGui::BeginChild("Content", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f), false, ImGuiWindowFlags_NoBackground))
		{
			ImGui::BeginGroupPanel(langApi->Translate("((000092))"), ImVec2(-1.0f, 0.0f));
			static std::string fontFile = settingsctx->Get<std::string>(OPT_USERFONT, "");
			if (fontFile.empty()) { fontFile = "Default"; }
			if (ImGui::BeginCombo("##FontSelector", fontFile.c_str()))
			{
				static long long s_LastPolled = 0;
				long long now = Time::GetTimestamp();

				if (now - s_LastPolled > 10)
				{
					this->PopulateFonts();
					s_LastPolled = now;
				}

				for (std::filesystem::path& font : this->Fonts)
				{
					if (ImGui::Selectable(font.stem().string().c_str(), font.filename().string() == fontFile))
					{
						if (font != fontFile)
						{
							fontFile = font.filename().string();

							settingsctx->Set(OPT_USERFONT, fontFile);
							uictx->LoadFonts();
						}
					}
				}
				ImGui::EndCombo();
			}

			ImGui::Text(langApi->Translate("((000054))"));
			static float fontSize = settingsctx->Get<float>(OPT_FONTSIZE, 13.0f);
			if (ImGui::InputFloat("##FontSizeInput", &fontSize, 0.0f, 0.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue))
			{
				fontSize = min(max(fontSize, 1.0f), 50.0f);

				settingsctx->Set(OPT_FONTSIZE, fontSize);

				ImGuiContext* imguictx = ImGui::GetCurrentContext();
				imguictx->FontSize = fontSize;

				CFontManager* fontMgr = uictx->GetFontManager();
				fontMgr->ResizeFont("USER_FONT", imguictx->FontSize);
				fontMgr->ResizeFont("FONT_DEFAULT", imguictx->FontSize);
			}
			ImGui::EndGroupPanel();

			ImVec2 region = ImGui::GetContentRegionAvail();

			static float presetsAreaEndY = region.y;
			static float actionsAreaEndY = region.y;
			float spacing = ImGui::GetStyle().ItemSpacing.y * 2;

			if (ImGui::BeginChild("##StyleEditor", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_NoBackground))
			{
				ImGuiStyle& style = ImGui::GetStyle();

				if (ImGui::BeginChild("##Presets", ImVec2(0.0f, presetsAreaEndY), false, ImGuiWindowFlags_NoBackground))
				{
					ImGui::BeginGroupPanel("Presets", ImVec2(-1.0f, 0.0f));
					if (ImGui::BeginCombo("##PresetSelector", "Select"))
					{
						/*if (ImGui::Selectable("Nexus"))
						{
							uictx->ApplyStyle(EUIStyle::Nexus);
						}*/
						if (ImGui::Selectable("ArcDPS Default"))
						{
							uictx->ApplyStyle(EUIStyle::ArcDPS_Default);
							this->HasUnsavedStyle = true;
						}

						static std::filesystem::path arcIniPath = Index::D_GW2_ADDONS / "arcdps/arcdps.ini";
						if (std::filesystem::exists(arcIniPath))
						{
							if (ImGui::Selectable("ArcDPS Current (Import from arcdps.ini)"))
							{
								uictx->ApplyStyle(EUIStyle::ArcDPS_Current);
								this->HasUnsavedStyle = true;
							}
						}

						ImGui::Separator();

						if (ImGui::Selectable("ImGui Classic"))
						{
							uictx->ApplyStyle(EUIStyle::ImGui_Classic);
							this->HasUnsavedStyle = true;
						}
						if (ImGui::Selectable("ImGui Light"))
						{
							uictx->ApplyStyle(EUIStyle::ImGui_Light);
							this->HasUnsavedStyle = true;
						}
						if (ImGui::Selectable("ImGui Dark"))
						{
							uictx->ApplyStyle(EUIStyle::ImGui_Dark);
							this->HasUnsavedStyle = true;
						}

						static long long s_LastPolled = 0;
						long long now = Time::GetTimestamp();

						if (now - s_LastPolled > 10)
						{
							this->PopulateStyles();
							s_LastPolled = now;
						}

						if (this->Styles.size() > 0)
						{
							ImGui::Separator();

							for (std::filesystem::path& style : this->Styles)
							{
								static Texture* s_TexCross = nullptr;

								if (s_TexCross)
								{
									ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
									ImGui::PushID(style.stem().string().c_str());
									if (ImGui::ImageButton(s_TexCross->Resource, ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight())))
									{
										try
										{
											std::filesystem::remove(style);
										}
										catch (...) {}
										s_LastPolled = 0; /* Reset to force a refresh. */
									}
									ImGui::PopID();
									ImGui::PopStyleVar();
									ImGui::TooltipGeneric("Delete preset");

									ImGui::SameLine();
								}
								else
								{
									s_TexCross = CContext::GetContext()->GetTextureService()->GetOrCreate("ICON_CLOSE", RES_ICON_CLOSE, ctx->GetModule());
								}

								/* Display the filename without extension. */
								if (ImGui::Selectable(style.stem().string().c_str()))
								{
									/* Pass the filename with extension. */
									uictx->ApplyStyle(EUIStyle::File, style.filename().string());
									this->HasUnsavedStyle = true;
								}
							}
						}
						ImGui::EndCombo();
					}
					ImGui::EndGroupPanel();

					presetsAreaEndY = ImGui::GetCursorPos().y;
				}
				ImGui::EndChild();

				if (ImGui::BeginChild("##Editor", ImVec2(0.0f, region.y - presetsAreaEndY - actionsAreaEndY - spacing), false, ImGuiWindowFlags_NoBackground))
				{
					if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None))
					{
						if (ImGui::BeginTabItem("Sizes"))
						{
							ImGui::Text("Main");
							if (ImGui::SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f"))
							{
								style.WindowPadding.x = min(max(style.WindowPadding.x, 0.0f), 20.0f);
								style.WindowPadding.y = min(max(style.WindowPadding.y, 0.0f), 20.0f);
								this->HasUnsavedStyle = true;
							}
							if (ImGui::SliderFloat2("FramePadding", (float*)&style.FramePadding, 0.0f, 20.0f, "%.0f"))
							{
								style.FramePadding.x = min(max(style.FramePadding.x, 0.0f), 20.0f);
								style.FramePadding.y = min(max(style.FramePadding.y, 0.0f), 20.0f);
								this->HasUnsavedStyle = true;
							}
							if (ImGui::SliderFloat2("CellPadding", (float*)&style.CellPadding, 0.0f, 20.0f, "%.0f"))
							{
								style.CellPadding.x = min(max(style.CellPadding.x, 0.0f), 20.0f);
								style.CellPadding.y = min(max(style.CellPadding.y, 0.0f), 20.0f);
								this->HasUnsavedStyle = true;
							}
							if (ImGui::SliderFloat2("ItemSpacing", (float*)&style.ItemSpacing, 0.0f, 20.0f, "%.0f"))
							{
								style.ItemSpacing.x = min(max(style.ItemSpacing.x, 0.0f), 20.0f);
								style.ItemSpacing.y = min(max(style.ItemSpacing.y, 0.0f), 20.0f);
								this->HasUnsavedStyle = true;
							}
							if (ImGui::SliderFloat2("ItemInnerSpacing", (float*)&style.ItemInnerSpacing, 0.0f, 20.0f, "%.0f"))
							{
								style.ItemInnerSpacing.x = min(max(style.ItemInnerSpacing.x, 0.0f), 20.0f);
								style.ItemInnerSpacing.y = min(max(style.ItemInnerSpacing.y, 0.0f), 20.0f);
								this->HasUnsavedStyle = true;
							}
							if (ImGui::SliderFloat2("TouchExtraPadding", (float*)&style.TouchExtraPadding, 0.0f, 10.0f, "%.0f"))
							{
								style.TouchExtraPadding.x = min(max(style.TouchExtraPadding.x, 0.0f), 10.0f);
								style.TouchExtraPadding.y = min(max(style.TouchExtraPadding.y, 0.0f), 10.0f);
								this->HasUnsavedStyle = true;
							}
							if (ImGui::SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f, "%.0f"))
							{
								style.IndentSpacing = min(max(style.IndentSpacing, 0.0f), 30.0f);
								this->HasUnsavedStyle = true;
							}
							if (ImGui::SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f, "%.0f"))
							{
								style.ScrollbarSize = min(max(style.ScrollbarSize, 1.0f), 20.0f);
								this->HasUnsavedStyle = true;
							}
							if (ImGui::SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.0f"))
							{
								style.GrabMinSize = min(max(style.GrabMinSize, 1.0f), 20.0f);
								this->HasUnsavedStyle = true;
							}
							ImGui::Text("Borders");
							if (ImGui::SliderFloat("WindowBorderSize", &style.WindowBorderSize, 0.0f, 1.0f, "%.0f"))
							{
								style.WindowBorderSize = min(max(style.WindowBorderSize, 0.0f), 1.0f);
								this->HasUnsavedStyle = true;
							}
							if (ImGui::SliderFloat("ChildBorderSize", &style.ChildBorderSize, 0.0f, 1.0f, "%.0f"))
							{
								style.ChildBorderSize = min(max(style.ChildBorderSize, 0.0f), 1.0f);
								this->HasUnsavedStyle = true;
							}
							if (ImGui::SliderFloat("PopupBorderSize", &style.PopupBorderSize, 0.0f, 1.0f, "%.0f"))
							{
								style.PopupBorderSize = min(max(style.PopupBorderSize, 0.0f), 1.0f);
								this->HasUnsavedStyle = true;
							}
							if (ImGui::SliderFloat("FrameBorderSize", &style.FrameBorderSize, 0.0f, 1.0f, "%.0f"))
							{
								style.FrameBorderSize = min(max(style.FrameBorderSize, 0.0f), 1.0f);
								this->HasUnsavedStyle = true;
							}
							if (ImGui::SliderFloat("TabBorderSize", &style.TabBorderSize, 0.0f, 1.0f, "%.0f"))
							{
								style.TabBorderSize = min(max(style.TabBorderSize, 0.0f), 1.0f);
								this->HasUnsavedStyle = true;
							}
							ImGui::Text("Rounding");
							if (ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f, "%.0f"))
							{
								style.WindowRounding = min(max(style.WindowRounding, 0.0f), 12.0f);
								this->HasUnsavedStyle = true;
							}
							if (ImGui::SliderFloat("ChildRounding", &style.ChildRounding, 0.0f, 12.0f, "%.0f"))
							{
								style.ChildRounding = min(max(style.ChildRounding, 0.0f), 12.0f);
								this->HasUnsavedStyle = true;
							}
							if (ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f"))
							{
								style.FrameRounding = min(max(style.FrameRounding, 0.0f), 12.0f);
								this->HasUnsavedStyle = true;
							}
							if (ImGui::SliderFloat("PopupRounding", &style.PopupRounding, 0.0f, 12.0f, "%.0f"))
							{
								style.PopupRounding = min(max(style.PopupRounding, 0.0f), 12.0f);
								this->HasUnsavedStyle = true;
							}
							if (ImGui::SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f, 12.0f, "%.0f"))
							{
								style.ScrollbarRounding = min(max(style.ScrollbarRounding, 0.0f), 12.0f);
								this->HasUnsavedStyle = true;
							}
							if (ImGui::SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 12.0f, "%.0f"))
							{
								style.GrabRounding = min(max(style.GrabRounding, 0.0f), 12.0f);
								this->HasUnsavedStyle = true;
							}
							if (ImGui::SliderFloat("LogSliderDeadzone", &style.LogSliderDeadzone, 0.0f, 12.0f, "%.0f"))
							{
								style.LogSliderDeadzone = min(max(style.LogSliderDeadzone, 0.0f), 12.0f);
								this->HasUnsavedStyle = true;
							}
							if (ImGui::SliderFloat("TabRounding", &style.TabRounding, 0.0f, 12.0f, "%.0f"))
							{
								style.TabRounding = min(max(style.TabRounding, 0.0f), 12.0f);
								this->HasUnsavedStyle = true;
							}
							ImGui::Text("Alignment");
							if (ImGui::SliderFloat2("WindowTitleAlign", (float*)&style.WindowTitleAlign, 0.0f, 1.0f, "%.2f"))
							{
								style.WindowTitleAlign.x = min(max(style.WindowTitleAlign.x, 0.0f), 1.0f);
								style.WindowTitleAlign.y = min(max(style.WindowTitleAlign.y, 0.0f), 1.0f);
								this->HasUnsavedStyle = true;
							}
							int window_menu_button_position = style.WindowMenuButtonPosition + 1;
							if (ImGui::Combo("WindowMenuButtonPosition", (int*)&window_menu_button_position, "None\0Left\0Right\0"))
							{
								style.WindowMenuButtonPosition = window_menu_button_position - 1;
								this->HasUnsavedStyle = true;
							}
							ImGui::Combo("ColorButtonPosition", (int*)&style.ColorButtonPosition, "Left\0Right\0");
							if (ImGui::SliderFloat2("ButtonTextAlign", (float*)&style.ButtonTextAlign, 0.0f, 1.0f, "%.2f"))
							{
								style.ButtonTextAlign.x = min(max(style.ButtonTextAlign.x, 0.0f), 1.0f);
								style.ButtonTextAlign.y = min(max(style.ButtonTextAlign.y, 0.0f), 1.0f);
								this->HasUnsavedStyle = true;
							}
							ImGui::SameLine(); ImGui::HelpMarker("Alignment applies when a button is larger than its text content.");
							if (ImGui::SliderFloat2("SelectableTextAlign", (float*)&style.SelectableTextAlign, 0.0f, 1.0f, "%.2f"))
							{
								style.SelectableTextAlign.x = min(max(style.SelectableTextAlign.x, 0.0f), 1.0f);
								style.SelectableTextAlign.y = min(max(style.SelectableTextAlign.y, 0.0f), 1.0f);
								this->HasUnsavedStyle = true;
							}
							ImGui::SameLine(); ImGui::HelpMarker("Alignment applies when a selectable is larger than its text content.");
							ImGui::Text("Safe Area Padding");
							ImGui::SameLine(); ImGui::HelpMarker("Adjust if you cannot see the edges of your screen (e.g. on a TV where scaling has not been configured).");
							if (ImGui::SliderFloat2("DisplaySafeAreaPadding", (float*)&style.DisplaySafeAreaPadding, 0.0f, 30.0f, "%.0f"))
							{
								style.DisplaySafeAreaPadding.x = min(max(style.DisplaySafeAreaPadding.x, 0.0f), 30.0f);
								style.DisplaySafeAreaPadding.y = min(max(style.DisplaySafeAreaPadding.y, 0.0f), 30.0f);
								this->HasUnsavedStyle = true;
							}
							ImGui::EndTabItem();
						}

						if (ImGui::BeginTabItem("Colors"))
						{
							static ImGuiTextFilter filter;
							filter.Draw("Filter colors", ImGui::GetFontSize() * 16);

							ImGui::PushItemWidth(-160);
							for (int i = 0; i < ImGuiCol_COUNT; i++)
							{
								const char* name = ImGui::GetStyleColorName(i);
								if (!filter.PassFilter(name))
									continue;
								ImGui::PushID(i);
								if (ImGui::ColorEdit4("##color", (float*)&style.Colors[i], ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf))
								{
									this->HasUnsavedStyle = true;
								}
								ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
								ImGui::TextUnformatted(name);
								ImGui::PopID();
							}
							ImGui::PopItemWidth();

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

				if (ImGui::BeginChild("##Actions", ImVec2(0.0f, actionsAreaEndY), false, ImGuiWindowFlags_NoBackground))
				{
					ImGui::Separator();

					/* Save */
					if (ImGui::Button(langApi->Translate("((000058))")))
					{
						settingsctx->Set(OPT_IMGUISTYLE, Base64::Encode((unsigned char*)&style, sizeof(ImGuiStyle)));
						this->HasUnsavedStyle = false;
					}

					ImGui::SameLine();

					/* Revert */
					if (ImGui::Button(langApi->Translate("((000059))")))
					{
						uictx->ApplyStyle();
						this->HasUnsavedStyle = false;
					}

					ImGui::SameLine();

					/* Save As Preset */
					if (ImGui::Button(langApi->Translate("Save As Preset")))
					{
						this->ExportModal.SetData(Base64::Encode((unsigned char*)&style, sizeof(ImGuiStyle)));
					}

					ImGui::SameLine();

					/* Import Preset */
					if (ImGui::Button(langApi->Translate("Import Preset")))
					{
						this->ImportModal.OpenModal();
					}

					ImGui::SameLine();

					/* Open Presets Folder */
					if (ImGui::Button(langApi->Translate("Open Presets Folder")))
					{
						std::string pathStyles = Index::D_GW2_ADDONS_NEXUS_STYLES.string();
						ShellExecuteA(NULL, "explore", pathStyles.c_str(), NULL, NULL, SW_SHOW);
					}

					if (this->HasUnsavedStyle)
					{
						ImGui::SameLine();
						ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Unsaved changes.");
					}

					actionsAreaEndY = ImGui::GetCursorPos().y;
				}
				ImGui::EndChild();
			}
			ImGui::EndChild();
		}
		ImGui::EndChild();

		ImGui::EndTabItem();
	}
}

void COptionsWindow::PopulateFonts()
{
	this->Fonts.clear();

	for (const std::filesystem::directory_entry entry : std::filesystem::directory_iterator(Index::D_GW2_ADDONS_NEXUS_FONTS))
	{
		std::filesystem::path path = entry.path();

		/* sanity checks */
		if (std::filesystem::is_directory(path)) { continue; }
		if (std::filesystem::file_size(path) == 0) { continue; }
		if (path.extension() == ".ttf")
		{
			this->Fonts.push_back(path);
		}
	}
}

void COptionsWindow::PopulateStyles()
{
	this->Styles.clear();

	for (const std::filesystem::directory_entry entry : std::filesystem::directory_iterator(Index::D_GW2_ADDONS_NEXUS_STYLES))
	{
		std::filesystem::path path = entry.path();

		/* sanity checks */
		if (std::filesystem::is_directory(path)) { continue; }
		if (std::filesystem::file_size(path) == 0) { continue; }
		if (path.extension() == ".imstyle180")
		{
			this->Styles.push_back(path);
		}
	}
}
