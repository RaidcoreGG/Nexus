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
#include "imgui/imgui_extensions.h"

#include "Host/GW2/Context.h"
#include "Host/GW2/Index/Index.h"
#include "res/Resources.h"
#include "UI/Widgets/QuickAccess/QaConst.h"
#include "Util/Base64.h"
#include "Util/Inputs.h"
#include "Util/Time.h"

COptionsWindow::COptionsWindow()
{
	this->Name           = "Options";
	this->DisplayName    = Lang::Nav::Options;
	this->IconIdentifier = "ICON_OPTIONS";
	this->IconEntry      = &Resources::IconOptions;
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
	static CUiContext* uictx = ctx->GetUIContext();
	static CLocalization* langApi = uictx->GetLocalization();
	static CQuickAccess* qactx = uictx->GetQuickAccess();

	if (ImGui::BeginTabItem(Lang::Options::General.Text()))
	{
		if (ImGui::BeginChild("Content", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f), false, ImGuiWindowFlags_NoBackground))
		{
			{
				ImGui::BeginGroupPanel(Lang::Options::Language.Text(), ImVec2(-1.0f, 0.0f));

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
								langApi->Language.Set(lang);
							}
						}
					}
					ImGui::EndCombo();
				}

				ImGui::EndGroupPanel();
			}

			{
				ImGui::BeginGroupPanel(Lang::Options::UiUx.Text(), ImVec2(-1.0f, 0.0f));

				CScaling* scaling = uictx->GetScaling();
				bool dpiScaling = scaling->DpiScaling.Get();
				if (ImGui::Checkbox(Lang::Options::DpiScaling.Text(), &dpiScaling))
				{
					scaling->DpiScaling.Set(dpiScaling);
				}

				CUiInput* uiInput = uictx->GetInput();
				bool clickingRequiresMods = uiInput->FilterClicks.Get();
				if (ImGui::Checkbox(Lang::Options::ClickingRequiresModifiers.Text(), &clickingRequiresMods))
				{
					uiInput->FilterClicks.Set(clickingRequiresMods);
					CContext::GetContext()->GetUIContext()->Invalidate();
				}

				if (clickingRequiresMods)
				{
					EModifiers mods = uiInput->RequiredModifiers.Get();

					ImGui::BeginGroup();
					ImGui::Indent();
					ImGui::Text(Lang::Options::Modifiers.Text());
					bool modAlt = (mods & EModifiers::Alt) == EModifiers::Alt;
					if (ImGui::Checkbox(Lang::Options::ModifierAlt.Text(), &modAlt))
					{
						if (modAlt)
						{
							mods |= EModifiers::Alt;
						}
						else
						{
							mods &= ~EModifiers::Alt;
						}
						uiInput->RequiredModifiers.Set(mods);
						CContext::GetContext()->GetUIContext()->Invalidate();
					}
					ImGui::SameLine();
					bool modCtrl = (mods & EModifiers::Ctrl) == EModifiers::Ctrl;
					if (ImGui::Checkbox(Lang::Options::ModifierCtrl.Text(), &modCtrl))
					{
						if (modCtrl)
						{
							mods |= EModifiers::Ctrl;
						}
						else
						{
							mods &= ~EModifiers::Ctrl;
						}
						uiInput->RequiredModifiers.Set(mods);
						CContext::GetContext()->GetUIContext()->Invalidate();
					}
					ImGui::SameLine();
					bool modShift = (mods & EModifiers::Shift) == EModifiers::Shift;
					if (ImGui::Checkbox(Lang::Options::ModifierShift.Text(), &modShift))
					{
						if (modShift)
						{
							mods |= EModifiers::Shift;
						}
						else
						{
							mods &= ~EModifiers::Shift;
						}
						uiInput->RequiredModifiers.Set(mods);
						CContext::GetContext()->GetUIContext()->Invalidate();
					}
					ImGui::EndGroup();
				}

				/* fix gw2's jumping cursor */
				CMouseResetFix* mouseResetFix = CContext::GetContext()->GetMouseResetFix();

				bool lockHiddenCursor = mouseResetFix->LockCursor.Get();
				if (ImGui::Checkbox(Lang::Options::LockCursor.Text(), &lockHiddenCursor))
				{
					mouseResetFix->LockCursor.Set(lockHiddenCursor);
				}

				bool resetHiddenCursor = mouseResetFix->ResetCursor.Get();
				if (ImGui::Checkbox(Lang::Options::ResetCursor.Text(), &resetHiddenCursor))
				{
					mouseResetFix->ResetCursor.Set(resetHiddenCursor);
				}

				/* close windows on escape */
				CEscapeClosing* escapeClosing = uictx->GetEscapeClosingService();
				bool closeOnEscape = escapeClosing->Enabled.Get();
				if (ImGui::Checkbox(Lang::Options::CloseOnEscape.Text(), &closeOnEscape))
				{
					escapeClosing->Enabled.Set(closeOnEscape);
				}

				/* show addons window after after addons were disabled because of an update */
				CMainWindow* mainWindow = uictx->GetMainWindow();
				bool showAddonsWindowOnGameUpdate = mainWindow->ShowAddonsAfterGameUpdate.Get();
				if (ImGui::Checkbox(Lang::Options::ShowAddonsAfterGameUpdate.Text(), &showAddonsWindowOnGameUpdate))
				{
					mainWindow->ShowAddonsAfterGameUpdate.Set(showAddonsWindowOnGameUpdate);
				}

				bool disableFestiveFlair = qactx->DisableFestiveFlair.Get();
				if (ImGui::Checkbox(Lang::Options::DisableFestiveFlair.Text(), &disableFestiveFlair))
				{
					qactx->DisableFestiveFlair.Set(disableFestiveFlair);
				}

				ImGui::EndGroupPanel();
			}

			{
				ImGui::BeginGroupPanel(Lang::Options::QuickAccess.Text(), ImVec2(-1.0f, 0.0f));

				/* suppressed icons selector (for addition) */
				std::vector<std::string> suppressedIcons = qactx->SuppressedShortcuts.Get();
				if (ImGui::BeginCombo("##QASuppressSelector", Lang::Options::QaSelectIconToHide.Text()))
				{
					for (auto& [id, shorcut] : qactx->GetRegistry())
					{
						if (ImGui::Selectable(id.c_str()))
						{
							/* if not already added */
							if (std::find(suppressedIcons.begin(), suppressedIcons.end(), id) == suppressedIcons.end())
							{
								suppressedIcons.push_back(id);
								qactx->SuppressedShortcuts.Set(suppressedIcons);
							}
						}
					}
					ImGui::EndCombo();
				}
				/* suppressed icons list (for removal) */
				ImGui::Text(Lang::Options::QaSuppressedIcons.Text());
				std::string removeSuppress{};
				for (std::string& id : suppressedIcons)
				{
					ImGui::BeginGroup();
					ImGui::Indent();
					if (ImGui::SmallButton((std::string{ Lang::Common::Show.Text() } + "##" + id).c_str()))
					{
						removeSuppress = id;
					}
					ImGui::SameLine();
					ImGui::Text(id.c_str());
					ImGui::EndGroup();
				}

				/* remove the requested icon id */
				if (!removeSuppress.empty())
				{
					auto it = std::find(suppressedIcons.begin(), suppressedIcons.end(), removeSuppress);
					if (it != suppressedIcons.end())
					{
						suppressedIcons.erase(it);
						qactx->SuppressedShortcuts.Set(suppressedIcons);
					}
				}
				ImGui::Separator();

				/* toggle vertical layout */
				bool verticalLayout = qactx->VerticalLayout.Get();
				if (ImGui::Checkbox(Lang::Options::QaVerticalLayout.Text(), &verticalLayout))
				{
					qactx->VerticalLayout.Set(verticalLayout);
				}

				/* only show on hover */
				bool onlyShowOnHover = qactx->OnlyShowOnHover.Get();
				if (ImGui::Checkbox(Lang::Options::QaHideUnlessHovered.Text(), &onlyShowOnHover))
				{
					qactx->OnlyShowOnHover.Set(onlyShowOnHover);
				}

				/* prefetch currently selected position string */
				EQaVisibility visibility = qactx->Visibility.Get();
				ImGui::Text(Lang::Options::QaVisibility.Text());
				if (ImGui::BeginCombo("##QAVisibilitySelector", EQaVisibilityToText(visibility).Text()))
				{
					if (ImGui::Selectable(Lang::QuickAccess::AlwaysShow.Text(), visibility == EQaVisibility::AlwaysShow))
					{
						qactx->Visibility.Set(EQaVisibility::AlwaysShow);
					}
					if (ImGui::Selectable(Lang::QuickAccess::DuringGameplay.Text(), visibility == EQaVisibility::Gameplay))
					{
						qactx->Visibility.Set(EQaVisibility::Gameplay);
					}
					if (ImGui::Selectable(Lang::QuickAccess::OutOfCombat.Text(), visibility == EQaVisibility::OutOfCombat))
					{
						qactx->Visibility.Set(EQaVisibility::OutOfCombat);
					}
					if (ImGui::Selectable(Lang::QuickAccess::InCombat.Text(), visibility == EQaVisibility::InCombat))
					{
						qactx->Visibility.Set(EQaVisibility::InCombat);
					}
					if (ImGui::Selectable(Lang::QuickAccess::NeverShow.Text(), visibility == EQaVisibility::Hide))
					{
						qactx->Visibility.Set(EQaVisibility::Hide);
					}
					ImGui::EndCombo();
				}

				/* show notification icon on update */
				bool notifyChangelog = qactx->NotifyChangelog.Get();
				if (ImGui::Checkbox(Lang::Options::QaNotifyChangelog.Text(), &notifyChangelog))
				{
					qactx->NotifyChangelog.Set(notifyChangelog);
				}

				/* position/location dropdown */
				EQaPosition position = qactx->Location.Get();
				ImGui::Text(Lang::Options::QaLocation.Text());
				if (ImGui::BeginCombo("##QALocationSelector", EQaPositionToText(position).Text()))
				{
					if (ImGui::Selectable(Lang::QuickAccess::PosExtend.Text(), position == EQaPosition::Extend))
					{
						qactx->Location.Set(EQaPosition::Extend);
					}
					if (ImGui::Selectable(Lang::QuickAccess::PosUnder.Text(), position == EQaPosition::Under))
					{
						qactx->Location.Set(EQaPosition::Under);
					}
					if (ImGui::Selectable(Lang::QuickAccess::PosBottom.Text(), position == EQaPosition::Bottom))
					{
						qactx->Location.Set(EQaPosition::Bottom);
					}
					if (ImGui::Selectable(Lang::QuickAccess::PosCustom.Text(), position == EQaPosition::Custom))
					{
						qactx->Location.Set(EQaPosition::Custom);
					}
					ImGui::EndCombo();
				}

				RenderContext_t* renderer = ctx->GetRendererCtx();

				/* offset */
				ImVec2 offset{ qactx->OffsetX.Get(), qactx->OffsetY.Get() };
				ImGui::Text(Lang::Options::QaOffset.Text());
				if (ImGui::DragFloat2("##QAOffsetInput", (float*)&offset, 1.0f, (static_cast<float>(renderer->Window.Height)) * -1.f, static_cast<float>(renderer->Window.Height)))
				{
					qactx->OffsetX.Set(offset.x);
					qactx->OffsetY.Set(offset.y);
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
	CUiContext* uictx = ctx->GetUIContext();
	CFontManager* fontMgr = uictx->GetFontManager();

	if (ImGui::BeginTabItem(Lang::Options::Style.Text()))
	{
		if (ImGui::BeginChild("Content", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f), false, ImGuiWindowFlags_NoBackground))
		{
			ImGui::BeginGroupPanel(Lang::Options::Font.Text(), ImVec2(-1.0f, 0.0f));
			std::string fontFile = fontMgr->UserFont.Get();
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

							fontMgr->UserFont.Set(fontFile);
							uictx->LoadFonts();
						}
					}
				}
				ImGui::EndCombo();
			}

			ImGui::Text(Lang::Options::FontSize.Text());
			float fontSize = fontMgr->FontSize.Get();
			if (ImGui::InputFloat("##FontSizeInput", &fontSize, 0.0f, 0.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue))
			{
				fontSize = min(max(fontSize, 1.0f), 50.0f);

				fontMgr->FontSize.Set(fontSize);

				ImGuiContext* imguictx = ImGui::GetCurrentContext();
				imguictx->FontSize = fontSize;

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
					ImGui::BeginGroupPanel(Lang::Options::StyleEditor::Presets.Text(), ImVec2(-1.0f, 0.0f));
					if (ImGui::BeginCombo("##PresetSelector", Lang::Common::Select.Text()))
					{
						if (ImGui::Selectable("Nexus"))
						{
							uictx->ApplyStyle(EUIStyle::Nexus);
						}
						if (ImGui::Selectable(Lang::Options::StyleEditor::ArcDpsDefault.Text()))
						{
							uictx->ApplyStyle(EUIStyle::ArcDPS_Default);
							this->HasUnsavedStyle = true;
						}

						static std::filesystem::path arcIniPath = Index(EPath::DIR_ADDONS) / "arcdps/arcdps.ini";
						if (std::filesystem::exists(arcIniPath))
						{
							if (ImGui::Selectable(Lang::Options::StyleEditor::ArcDpsCurrent.Text()))
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
								static Texture_t* s_TexCross = nullptr;

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
									ImGui::TooltipGeneric(Lang::Options::StyleEditor::DeletePreset.Text());

									ImGui::SameLine();
								}
								else
								{
									s_TexCross = CContext::GetContext()->GetTextureService()->GetOrCreate("ICON_CLOSE", Resources::IconClose);
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
						if (ImGui::BeginTabItem(Lang::Options::StyleEditor::Sizes.Text()))
						{
							ImGui::TextUnformatted(Lang::Options::StyleEditor::Main.Text());
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
							ImGui::TextUnformatted(Lang::Options::StyleEditor::Borders.Text());
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
							ImGui::TextUnformatted(Lang::Options::StyleEditor::Rounding.Text());
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
							ImGui::TextUnformatted(Lang::Options::StyleEditor::Alignment.Text());
							if (ImGui::SliderFloat2("WindowTitleAlign", (float*)&style.WindowTitleAlign, 0.0f, 1.0f, "%.2f"))
							{
								style.WindowTitleAlign.x = min(max(style.WindowTitleAlign.x, 0.0f), 1.0f);
								style.WindowTitleAlign.y = min(max(style.WindowTitleAlign.y, 0.0f), 1.0f);
								this->HasUnsavedStyle = true;
							}
							int window_menu_button_position = style.WindowMenuButtonPosition + 1;
							const char* menuButtonPositions[] = { Lang::Options::StyleEditor::None.Text(), Lang::Options::StyleEditor::Left.Text(), Lang::Options::StyleEditor::Right.Text() };
							if (ImGui::Combo("WindowMenuButtonPosition", (int*)&window_menu_button_position, menuButtonPositions, IM_ARRAYSIZE(menuButtonPositions)))
							{
								style.WindowMenuButtonPosition = window_menu_button_position - 1;
								this->HasUnsavedStyle = true;
							}
							const char* colorButtonPositions[] = { Lang::Options::StyleEditor::Left.Text(), Lang::Options::StyleEditor::Right.Text() };
							ImGui::Combo("ColorButtonPosition", (int*)&style.ColorButtonPosition, colorButtonPositions, IM_ARRAYSIZE(colorButtonPositions));
							if (ImGui::SliderFloat2("ButtonTextAlign", (float*)&style.ButtonTextAlign, 0.0f, 1.0f, "%.2f"))
							{
								style.ButtonTextAlign.x = min(max(style.ButtonTextAlign.x, 0.0f), 1.0f);
								style.ButtonTextAlign.y = min(max(style.ButtonTextAlign.y, 0.0f), 1.0f);
								this->HasUnsavedStyle = true;
							}
							ImGui::SameLine(); ImGui::HelpMarker(Lang::Options::StyleEditor::ButtonAlignHint.Text());
							if (ImGui::SliderFloat2("SelectableTextAlign", (float*)&style.SelectableTextAlign, 0.0f, 1.0f, "%.2f"))
							{
								style.SelectableTextAlign.x = min(max(style.SelectableTextAlign.x, 0.0f), 1.0f);
								style.SelectableTextAlign.y = min(max(style.SelectableTextAlign.y, 0.0f), 1.0f);
								this->HasUnsavedStyle = true;
							}
							ImGui::SameLine(); ImGui::HelpMarker(Lang::Options::StyleEditor::SelectableAlignHint.Text());
							ImGui::TextUnformatted(Lang::Options::StyleEditor::SafeAreaPadding.Text());
							ImGui::SameLine(); ImGui::HelpMarker(Lang::Options::StyleEditor::SafeAreaHint.Text());
							if (ImGui::SliderFloat2("DisplaySafeAreaPadding", (float*)&style.DisplaySafeAreaPadding, 0.0f, 30.0f, "%.0f"))
							{
								style.DisplaySafeAreaPadding.x = min(max(style.DisplaySafeAreaPadding.x, 0.0f), 30.0f);
								style.DisplaySafeAreaPadding.y = min(max(style.DisplaySafeAreaPadding.y, 0.0f), 30.0f);
								this->HasUnsavedStyle = true;
							}
							ImGui::EndTabItem();
						}

						if (ImGui::BeginTabItem(Lang::Options::StyleEditor::Colors.Text()))
						{
							static ImGuiTextFilter filter;
							filter.Draw(Lang::Options::StyleEditor::FilterColors.Text(), ImGui::GetFontSize() * 16);

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

						if (ImGui::BeginTabItem(Lang::Options::StyleEditor::Rendering.Text()))
						{
							ImGui::Checkbox(Lang::Options::StyleEditor::AaLines.Text(), &style.AntiAliasedLines);
							ImGui::SameLine();
							ImGui::HelpMarker(Lang::Options::StyleEditor::AaLinesHint.Text());

							ImGui::Checkbox(Lang::Options::StyleEditor::AaLinesTex.Text(), &style.AntiAliasedLinesUseTex);
							ImGui::SameLine();
							ImGui::HelpMarker(Lang::Options::StyleEditor::AaLinesTexHint.Text());

							ImGui::Checkbox(Lang::Options::StyleEditor::AaFill.Text(), &style.AntiAliasedFill);
							ImGui::PushItemWidth(100);
							ImGui::DragFloat(Lang::Options::StyleEditor::CurveTolerance.Text(), &style.CurveTessellationTol, 0.02f, 0.10f, 10.0f, "%.2f");
							if (style.CurveTessellationTol < 0.10f) style.CurveTessellationTol = 0.10f;

							// When editing the "Circle Segment Max Error" value, draw a preview of its effect on auto-tessellated circles.
							ImGui::DragFloat(Lang::Options::StyleEditor::CircleError.Text(), &style.CircleSegmentMaxError, 0.01f, 0.10f, 10.0f, "%.2f");
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
							ImGui::HelpMarker(Lang::Options::StyleEditor::CircleErrorHint.Text());

							ImGui::DragFloat(Lang::Options::StyleEditor::GlobalAlpha.Text(), &style.Alpha, 0.005f, 0.20f, 1.0f, "%.2f"); // Not exposing zero here so user doesn't "lose" the UI (zero alpha clips all widgets). But application code could have a toggle to switch between zero and non-zero.
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
					if (ImGui::Button(Lang::Common::Save.Text()))
					{
						uictx->UserStyle.Set(Base64::Encode((unsigned char*)&style, sizeof(ImGuiStyle)));
						this->HasUnsavedStyle = false;
					}

					ImGui::SameLine();

					/* Revert */
					if (ImGui::Button(Lang::Common::Revert.Text()))
					{
						uictx->ApplyStyle();
						this->HasUnsavedStyle = false;
					}

					ImGui::SameLine();

					/* Save As Preset */
					if (ImGui::Button(Lang::Options::StyleSavePreset.Text()))
					{
						this->ExportModal.SetData(Base64::Encode((unsigned char*)&style, sizeof(ImGuiStyle)));
					}

					ImGui::SameLine();

					/* Import Preset */
					if (ImGui::Button(Lang::Options::StyleImportPreset.Text()))
					{
						this->ImportModal.OpenModal();
					}

					ImGui::SameLine();

					/* Open Presets Folder */
					if (ImGui::Button(Lang::Options::StyleOpenPresetsFolder.Text()))
					{
						std::string pathStyles = Index(EPath::DIR_STYLES).string();
						ShellExecuteA(NULL, "explore", pathStyles.c_str(), NULL, NULL, SW_SHOW);
					}

					if (this->HasUnsavedStyle)
					{
						ImGui::SameLine();
						ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "%s", Lang::Options::StyleEditor::Unsaved.Text());
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

	for (const std::filesystem::directory_entry entry : std::filesystem::directory_iterator(Index(EPath::DIR_FONTS)))
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

	for (const std::filesystem::directory_entry entry : std::filesystem::directory_iterator(Index(EPath::DIR_STYLES)))
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
