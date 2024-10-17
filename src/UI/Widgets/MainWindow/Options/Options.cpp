///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Options.cpp
/// Description  :  Contains the content of the options window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Options.h"

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

	this->Invalidate();
}

void COptionsWindow::RenderContent()
{
	if (this->IsInvalid)
	{
		this->PopulateFonts();
		this->IsInvalid = false;
	}

	if (ImGui::BeginTabBar("OptionsTabBar", ImGuiTabBarFlags_None))
	{
		this->TabGeneral();
		this->TabStyle();

		ImGui::EndTabBar();
	}
}

void COptionsWindow::TabGeneral()
{
	CContext* ctx = CContext::GetContext();
	CLocalization* langApi = ctx->GetLocalization();
	CSettings* settingsctx = ctx->GetSettingsCtx();
	CUiContext* uictx = ctx->GetUIContext();
	CQuickAccess* qactx = uictx->GetQuickAccess();

	if (ImGui::BeginTabItem(langApi->Translate("((000052))")))
	{
		if (ImGui::BeginChild("##GeneralTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f)))
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

				/* close windows on escape */
				static bool closeOnEscape = settingsctx->Get<bool>(OPT_CLOSEESCAPE);
				if (ImGui::Checkbox(langApi->Translate("((000044))"), &closeOnEscape))
				{
					settingsctx->Set(OPT_CLOSEESCAPE, closeOnEscape);
				}

				/* show addons window after after addons were disabled because of an update */
				static bool showAddonsWindowOnGameUpdate = settingsctx->Get<bool>(OPT_SHOWADDONSWINDOWAFTERDUU);
				if (ImGui::Checkbox(langApi->Translate("((000086))"), &showAddonsWindowOnGameUpdate))
				{
					settingsctx->Set(OPT_SHOWADDONSWINDOWAFTERDUU, showAddonsWindowOnGameUpdate);
				}

				static bool disableFestiveFlair = settingsctx->Get<bool>(OPT_DISABLEFESTIVEFLAIR);
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
				static bool notifyChangelog = settingsctx->Get<bool>(OPT_NOTIFYCHANGELOG);
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
		if (ImGui::BeginChild("##StyleTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f)))
		{
			static long long lastCheckedFonts = Time::GetTimestamp();

			ImGui::BeginGroupPanel(langApi->Translate("((000092))"), ImVec2(-1.0f, 0.0f));
			static std::string fontFile = settingsctx->Get<std::string>(OPT_USERFONT);
			if (fontFile.empty()) { fontFile = "Default"; }
			if (ImGui::BeginCombo("##FontSelector", fontFile.c_str()))
			{
				long long now = Time::GetTimestamp();

				if (now - lastCheckedFonts > 10)
				{
					this->PopulateFonts();
					lastCheckedFonts = now;
				}

				for (std::string font : this->Fonts)
				{
					if (ImGui::Selectable(font.c_str(), font == fontFile))
					{
						if (font != fontFile)
						{
							fontFile = font;

							settingsctx->Set(OPT_USERFONT, fontFile);
							uictx->LoadFonts();
						}
					}
				}
				ImGui::EndCombo();
			}
			ImGui::EndGroupPanel();

			ImGuiContext* imguictx = ImGui::GetCurrentContext();

			ImGui::BeginGroupPanel(langApi->Translate("((000054))"), ImVec2(-1.0f, 0.0f));
			if (ImGui::InputFloat("##FontSizeInput", &imguictx->FontSize, 0.0f, 0.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue))
			{
				imguictx->FontSize = min(max(imguictx->FontSize, 1.0f), 50.0f);

				settingsctx->Set(OPT_FONTSIZE, imguictx->FontSize);

				CFontManager* fontMgr = uictx->GetFontManager();
				fontMgr->ResizeFont("USER_FONT", imguictx->FontSize);
				fontMgr->ResizeFont("FONT_DEFAULT", imguictx->FontSize);
			}
			ImGui::EndGroupPanel();

			ImGuiIO& io = ImGui::GetIO();
			ImGui::BeginGroupPanel(langApi->Translate("((000072))"), ImVec2(-1.0f, 0.0f));
			if (ImGui::DragFloat("##GlobalScaleInput", &io.FontGlobalScale, 0.005f, 0.75f, 3.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
			{
				settingsctx->Set(OPT_GLOBALSCALE, io.FontGlobalScale);
				float uiScale = settingsctx->Get<float>(OPT_LASTUISCALE);

				if (uiScale <= 0)
				{
					uiScale = SC_NORMAL;
				}

				Renderer::Scaling = uiScale * io.FontGlobalScale;
			}
			ImGui::EndGroupPanel();

			ImGui::BeginGroupPanel(langApi->Translate("((000053))"), ImVec2(-1.0f, 0.0f));
			static bool linkArcDpsStyle = settingsctx->Get<bool>(OPT_LINKARCSTYLE);
			if (ImGui::Checkbox(langApi->Translate("((000056))"), &linkArcDpsStyle))
			{
				settingsctx->Set(OPT_LINKARCSTYLE, linkArcDpsStyle);
				uictx->ApplyStyle();
			}
			ImGui::TooltipGeneric(langApi->Translate("((000057))"));

			if (!linkArcDpsStyle)
			{
				ImGuiStyle& style = ImGui::GetStyle();

				ImGui::Text("Select from preset:");
				if (ImGui::BeginCombo("##StylePresetSelector", nullptr))
				{
					/*if (ImGui::Selectable("Nexus"))
					{
						uictx->ApplyStyle(EUIStyle::Nexus);
					}*/
					if (ImGui::Selectable("ImGui Classic"))
					{
						uictx->ApplyStyle(EUIStyle::ImGui_Classic);
					}
					if (ImGui::Selectable("ImGui Light"))
					{
						uictx->ApplyStyle(EUIStyle::ImGui_Light);
					}
					if (ImGui::Selectable("ImGui Dark"))
					{
						uictx->ApplyStyle(EUIStyle::ImGui_Dark);
					}
					ImGui::EndCombo();
				}

				if (ImGui::SmallButton(langApi->Translate("((000058))")))
				{
					settingsctx->Set(OPT_IMGUISTYLE, Base64::Encode((unsigned char*)&style, sizeof(ImGuiStyle)));
					settingsctx->Set(OPT_IMGUICOLORS, Base64::Encode((unsigned char*)&style.Colors[0], sizeof(ImVec4) * ImGuiCol_COUNT));
				}
				ImGui::SameLine();
				if (ImGui::SmallButton(langApi->Translate("((000059))")))
				{
					uictx->ApplyStyle();
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

						ImGui::BeginChild("##colors", ImVec2(0, 0), true);
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
			ImGui::EndGroupPanel();
		}
		ImGui::EndChild();

		ImGui::EndTabItem();
	}
}

void COptionsWindow::PopulateFonts()
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
