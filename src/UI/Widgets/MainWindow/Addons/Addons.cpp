///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Addons.cpp
/// Description  :  Contains the content of the addons window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Addons.h"

#include <shellapi.h>

#include "imgui/imgui.h"
#include "imgui_extensions.h"

#include "Core/Addons/Library/LibAddon.h"
#include "Core/Context.h"
#include "Core/Index/Index.h"
#include "LoadUnloadButton.h"
#include "Resources/ResConst.h"
#include "Util/Strings.h"

constexpr EAddonsFilterFlags FILTER_INSTALLED
	= EAddonsFilterFlags::ShowEnabled
	| EAddonsFilterFlags::ShowDisabled;

constexpr EAddonsFilterFlags FILTER_LIBRARY = EAddonsFilterFlags::ShowDownloadable;

static CAddonsWindow* AddonsWindow = nullptr;

void OnAddonChanged(void* aEventData)
{
	if (!AddonsWindow) { return; }

	if (aEventData)
	{
		AddonsWindow->Invalidate(*(signed int*)aEventData);
	}
	else
	{
		AddonsWindow->Invalidate();
	}
}

CAddonsWindow::CAddonsWindow()
{
	this->Name           = "Addons";
	this->DisplayName    = "((000003))";
	this->IconIdentifier = "ICON_ADDONS";
	this->IconID         = RES_ICON_ADDONS;
	this->IsHost         = true;
	this->IsAnchored     = true;

	this->Invalidate();

	CContext*  ctx         = CContext::GetContext();
	CSettings* settingsctx = ctx->GetSettingsCtx();
	CEventApi* evtapi      = ctx->GetEventApi();
	
	this->Filter     = settingsctx->Get(OPT_ADDONFILTERS, FILTER_INSTALLED);
	this->IsListMode = settingsctx->Get(OPT_ISLISTMODE,   true            );

	evtapi->Subscribe("EV_ADDON_LOADED", OnAddonChanged);
	evtapi->Subscribe("EV_ADDON_UNLOADED", OnAddonChanged);
	evtapi->Subscribe("EV_ADDON_CREATED", OnAddonChanged);
	evtapi->Subscribe("EV_ADDON_DESTROYED", OnAddonChanged);
	AddonsWindow = this;
}

void CAddonsWindow::Invalidate()
{
	this->IsInvalid = true;
}

void CAddonsWindow::Invalidate(signed int aAddonID)
{
	this->IsInvalid = true;

	if (this->AddonData.GetSig() == aAddonID)
	{
		this->AddonData.OptionsRender = nullptr;
	}
}

void CAddonsWindow::SetContent(AddonListing_t& aAddonData)
{
	CContext* ctx = CContext::GetContext();
	CUiContext* uictx = ctx->GetUIContext();

	this->AddonData = aAddonData;
	this->AddonData.InputBinds = uictx->GetInputBinds(this->AddonData.GetName());
	this->HasContent = true;
}

void CAddonsWindow::ClearContent()
{
	this->AddonData.InputBinds.clear();
	this->HasContent = false;
}

void CAddonsWindow::AddonItem(AddonListing_t& aAddonData, float aWidth)
{
	/* Unique id. */
	std::string id;

	if (aAddonData.GetSig() != 0)
	{
		/* Use addon signature. */
		id = String::Format("0x%08X", aAddonData.GetSig());
	}
	else
	{
		if (aAddonData.Addon)
		{
			/* Use addon file location. */
			id = aAddonData.Addon->GetLocation().string();
		}
		else
		{
			/* This should not be possible. */
			throw "Unreachable code.";
		}
	}

	ImGuiStyle& style = ImGui::GetStyle();

	ImVec2 itemSz = ImVec2(aWidth, ImGui::GetFrameHeightWithSpacing() * 4);
	float innerHeight = itemSz.y - (style.WindowPadding.y * 2);

	ImVec2 btnTextSz = ImGui::CalcTextSize("############");

	float btnWidth = btnTextSz.x + (style.FramePadding.x * 2);
	float actionsAreaWidth = btnWidth;

	ImVec2 curPos = ImGui::GetCursorPos();

	/* above visible space                        || under visible space */
	if (curPos.y < ImGui::GetScrollY() - itemSz.y || curPos.y > ImGui::GetScrollY() + ImGui::GetWindowHeight())
	{
		ImGui::Dummy(itemSz);
		return;
	}

	static CContext*       ctx      = CContext::GetContext();
	static CUiContext*     uictx    = ctx->GetUIContext();
	static CConfigMgr*     cfgmgr   = ctx->GetCfgMgr();
	static CTextureLoader* texapi   = ctx->GetTextureService();
	static CLocalization*  langApi  = uictx->GetLocalization();
	static CAlerts*        alertctx = uictx->GetAlerts();

	ImDrawList* dl = ImGui::GetWindowDrawList();

	bool pushedBgCol = false;

	if (aAddonData.IsHovered)
	{
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
		pushedBgCol = true;
	}
	if (ImGui::BeginChild(("Addon_" + id).c_str(), itemSz, true))
	{
		if (pushedBgCol)
		{
			ImGui::PopStyleColor();
			pushedBgCol = false;
		}

		ImGuiWindow* wnd = ImGui::GetCurrentWindow();
		ImColor col = ImColor(85, 85, 85, 255);

		if (aAddonData.Addon && aAddonData.Addon->IsLoaded())
		{
			col = ImColor(89, 172, 98, 255);
		}
		else if (aAddonData.Addon && aAddonData.Addon->IsVersionDisabled())
		{
			col = ImColor(172, 89, 89, 255);
			/* TODO: other error states. */
		}

		ImVec2 pad = ImGui::GetStyle().WindowPadding;
		float round = ImGui::GetStyle().ChildRounding;
		float spc = ImGui::GetStyle().ItemSpacing.x;

		dl->AddRectFilled(ImVec2(wnd->Pos.x, wnd->Pos.y), ImVec2(wnd->Pos.x + (ImGui::GetFrameHeight() / 2.f), wnd->Pos.y + wnd->Size.y), col, round);

		ImGui::Dummy(ImVec2(pad.x + (ImGui::GetFrameHeight() / 2.f) - spc, 0));
		ImGui::SameLine();

		ImGui::BeginGroup();

		/* Name */
		ImGui::Text(aAddonData.GetName().c_str());

		/* Version */
		if (!aAddonData.GetVersion().empty())
		{
			ImGui::SameLine();
			ImGui::TextDisabled(aAddonData.GetVersion().c_str());
		}

		/* Author */
		if (!aAddonData.GetAuthor().empty())
		{
			ImGui::TextDisabled("%s", aAddonData.GetAuthor().c_str());
		}

		/* Description */
		if (!aAddonData.GetDesc().empty())
		{
			ImGui::TextWrapped(aAddonData.GetDesc().c_str());
		}

		ImGui::EndGroup();
	}
	ImGui::EndChild();

	if (pushedBgCol)
	{
		ImGui::PopStyleColor();
		pushedBgCol = false;
	}

	aAddonData.IsHovered = ImGui::IsItemHovered();
	if (ImGui::IsItemClicked())
	{
		if (aAddonData.Addon)
		{
			this->SetContent(aAddonData);
		}
		else if (aAddonData.HasLibDef)
		{
			/* TODO */
		}
		aAddonData.IsHovered = false;
	}
}

void CAddonsWindow::RenderContent()
{
	if (this->IsInvalid)
	{
		static CContext* ctx = CContext::GetContext();
		static CUiContext* uictx = ctx->GetUIContext();
		static CEscapeClosing* escclose = uictx->GetEscapeClosingService();

		escclose->Deregister(this->GetVisibleStatePtr());
		escclose->Register(this->GetNameID().c_str(), this->GetVisibleStatePtr());

		this->PopulateAddons();

		this->IsInvalid = false;
	}

	ImVec2 region = ImGui::GetContentRegionAvail();

	static ImVec2 s_FilterBarSize  = ImVec2(region.x, region.y);
	static ImVec2 s_ActionsBarSize = ImVec2(region.x, region.y);

	ImGuiStyle& style = ImGui::GetStyle();

	if (!this->IsPoppedOut)
	{
		float btnSz = ImGui::GetFontSize() * 1.5f;
		s_FilterBarSize.x = region.x - style.ItemSpacing.x - btnSz - style.ItemSpacing.x;
	}

	this->RenderFilterBar(s_FilterBarSize);
	
	this->RenderBody(
		ImVec2(
			region.x,
			region.y - s_FilterBarSize.y - s_ActionsBarSize.y - style.ItemSpacing.y - style.ItemSpacing.y
		)
	);

	this->RenderActionsBar(s_ActionsBarSize);
}

void CAddonsWindow::RenderFilterBar(ImVec2& aSize)
{
	CContext*       ctx         = CContext::GetContext();
	CSettings*      settingsctx = ctx->GetSettingsCtx();
	CTextureLoader* texapi      = ctx->GetTextureService();
	CUiContext*     uictx       = ctx->GetUIContext();
	CLocalization*  lang        = uictx->GetLocalization();

	ImGuiStyle& style = ImGui::GetStyle();

	if (ImGui::BeginChild("Filters", aSize, false, ImGuiWindowFlags_NoBackground))
	{
		/* Static assets */
		static char s_SearchTerm[400] = {};
		static Texture_t* s_ClearIcon          = texapi->GetOrCreate("ICON_CLOSE",  RES_ICON_CLOSE,  ctx->GetModule());
		static Texture_t* s_ViewModeIcon_List  = texapi->GetOrCreate("ICON_LIST",   RES_ICON_LIST,   ctx->GetModule());
		static Texture_t* s_ViewModeIcon_Tiles = texapi->GetOrCreate("ICON_TILES",  RES_ICON_TILES,  ctx->GetModule());
		static Texture_t* s_FilterIcon         = texapi->GetOrCreate("ICON_FILTER", RES_ICON_FILTER, ctx->GetModule());

		if (!s_ClearIcon)          { s_ClearIcon          = texapi->GetOrCreate("ICON_CLOSE",  RES_ICON_CLOSE,  ctx->GetModule()); }
		if (!s_ViewModeIcon_List)  { s_ViewModeIcon_List  = texapi->GetOrCreate("ICON_LIST",   RES_ICON_LIST,   ctx->GetModule()); }
		if (!s_ViewModeIcon_Tiles) { s_ViewModeIcon_Tiles = texapi->GetOrCreate("ICON_TILES",  RES_ICON_TILES,  ctx->GetModule()); }
		if (!s_FilterIcon)         { s_FilterIcon         = texapi->GetOrCreate("ICON_FILTER", RES_ICON_FILTER, ctx->GetModule()); }

		/* Search Term */
		if (ImGui::InputTextWithHint("##SearchTerm", lang->Translate("((000104))"), &s_SearchTerm[0], 400))
		{
			this->SearchTerm = String::ToLower(s_SearchTerm);
			this->Invalidate();
			this->ClearContent();
		}

		/* Clear Search Term */
		if (s_ClearIcon)
		{
			ImGui::SameLine();
			if (ImGui::ImageButton(s_ClearIcon->Resource, ImVec2(ImGui::GetFontSize(), ImGui::GetFontSize())))
			{
				memset(s_SearchTerm, 0, 400);
				this->SearchTerm = String::ToLower(s_SearchTerm);
				this->Invalidate();
				this->ClearContent();
			}
		}

		/* View Mode */
		Texture_t* viewModeIcon = this->IsListMode ? s_ViewModeIcon_List : s_ViewModeIcon_Tiles;
		if (viewModeIcon)
		{
			ImGui::SameLine();
			if (ImGui::ImageButton(viewModeIcon->Resource, ImVec2(ImGui::GetFontSize(), ImGui::GetFontSize())))
			{
				viewModeIcon = nullptr;
				this->IsListMode = !this->IsListMode;
				settingsctx->Set(OPT_ISLISTMODE, this->IsListMode);
			}
		}

		/* Advanced Filters */
		bool doPopHighlight = false;

		if (s_FilterIcon)
		{
			/* If filter does not match quick filter, highlight manual filter icon. */
			if (this->Filter != FILTER_INSTALLED && this->Filter != FILTER_LIBRARY)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_ButtonActive]);
				doPopHighlight = true;
			}

			if (ImGui::ImageButton(s_FilterIcon->Resource, ImVec2(ImGui::GetFontSize(), ImGui::GetFontSize())))
			{
				ImGui::OpenPopup("Filters");
			}

			if (doPopHighlight)
			{
				ImGui::PopStyleColor();
				doPopHighlight = false;
			}

			if (ImGui::BeginPopupContextItem("Filters"))
			{
				bool showEnabled = (this->Filter & EAddonsFilterFlags::ShowEnabled) == EAddonsFilterFlags::ShowEnabled;
				if (ImGui::Checkbox(lang->Translate("((000106))"), &showEnabled))
				{
					if (showEnabled)
					{
						this->Filter |= EAddonsFilterFlags::ShowEnabled;
					}
					else
					{
						this->Filter &= ~EAddonsFilterFlags::ShowEnabled;
					}
					settingsctx->Set(OPT_ADDONFILTERS, this->Filter);
					this->Invalidate();
					this->ClearContent();
				}

				bool showDisabled = (this->Filter & EAddonsFilterFlags::ShowDisabled) == EAddonsFilterFlags::ShowDisabled;
				if (ImGui::Checkbox(lang->Translate("((000107))"), &showDisabled))
				{
					if (showDisabled)
					{
						this->Filter |= EAddonsFilterFlags::ShowDisabled;
					}
					else
					{
						this->Filter &= ~EAddonsFilterFlags::ShowDisabled;
					}
					settingsctx->Set(OPT_ADDONFILTERS, this->Filter);
					this->Invalidate();
					this->ClearContent();
				}

				bool showDownloadable = (this->Filter & EAddonsFilterFlags::ShowDownloadable) == EAddonsFilterFlags::ShowDownloadable;
				if (ImGui::Checkbox(lang->Translate("((000108))"), &showDownloadable))
				{
					if (showDownloadable)
					{
						this->Filter |= EAddonsFilterFlags::ShowDownloadable;
					}
					else
					{
						this->Filter &= ~EAddonsFilterFlags::ShowDownloadable;
					}
					settingsctx->Set(OPT_ADDONFILTERS, this->Filter);
					this->Invalidate();
					this->ClearContent();
				}

				ImGui::TextDisabled("%u / %u", this->Addons.size(), this->AddonsAmtUnfiltered);

				ImGui::EndPopup();
			}
		}

		ImGui::SameLine();

		/* Quick Filter: Installed */
		if ((this->Filter & FILTER_INSTALLED) == FILTER_INSTALLED &&
			(this->Filter & ~FILTER_INSTALLED) == EAddonsFilterFlags::None)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_ButtonActive]);
			doPopHighlight = true;
		}

		if (ImGui::Button(lang->Translate("((000031))")))
		{
			this->Filter = FILTER_INSTALLED;
			settingsctx->Set(OPT_ADDONFILTERS, this->Filter);
			this->Invalidate();
			this->ClearContent();
		}

		if (doPopHighlight)
		{
			ImGui::PopStyleColor();
			doPopHighlight = false;
		}

		ImGui::SameLine();

		/* Quick Filter: Library */
		if ((this->Filter & FILTER_LIBRARY) == FILTER_LIBRARY &&
			(this->Filter & ~FILTER_LIBRARY) == EAddonsFilterFlags::None)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_ButtonActive]);
			doPopHighlight = true;
		}

		if (ImGui::Button(lang->Translate("((000032))")))
		{
			this->Filter = FILTER_LIBRARY;
			settingsctx->Set(OPT_ADDONFILTERS, this->Filter);
			this->Invalidate();
			this->ClearContent();
		}

		if (doPopHighlight)
		{
			ImGui::PopStyleColor();
			doPopHighlight = false;
		}

		aSize.y = ImGui::GetCursorPos().y;
	}
	ImGui::EndChild();
}

void CAddonsWindow::RenderBody(ImVec2 aSize)
{
	CContext*       ctx   = CContext::GetContext();
	CUiContext*     uictx = ctx->GetUIContext();
	CLocalization*  lang  = uictx->GetLocalization();

	ImGuiStyle& style = ImGui::GetStyle();

	if (ImGui::BeginChild("Body", aSize, false, ImGuiWindowFlags_NoBackground))
	{
		if (this->HasContent)
		{
			/* Details view */
			this->RenderDetails();
		}
		else if (this->Addons.size() == 0)
		{
			/* Nothing matching filter. */
			ImVec2 windowSize = ImGui::GetWindowSize();
			ImVec2 textSize = ImGui::CalcTextSize(lang->Translate("((000098))"));
			ImVec2 position = ImGui::GetCursorPos();
			ImGui::SetCursorPos(ImVec2((position.x + (windowSize.x - textSize.x)) / 2, (position.y + (windowSize.y - textSize.y)) / 2));
			ImGui::TextDisabled(lang->Translate("((000098))"));
		}
		else
		{
			/* Addons list */
			float addonItemWidth = this->IsListMode
				? (aSize.x - style.ScrollbarSize)
				: ((aSize.x - style.ItemSpacing.x - style.ScrollbarSize) / 2);

			int i = 0;

			for (AddonListing_t& addon : this->Addons)
			{
				if (!this->IsListMode && i % 2 == 1)
				{
					ImGui::SameLine();
				}

				AddonItem(addon, addonItemWidth);
				i++;
			}
		}
	}
	ImGui::EndChild();
}

void CAddonsWindow::RenderDetails()
{
	assert(this->AddonData.Addon);

	ImGuiStyle& style = ImGui::GetStyle();

	float btnSz = ImGui::GetFontSize() * 1.5f;
	ImVec2 btnTextSz = ImGui::CalcTextSize("############");
	float btnWidth = btnTextSz.x + (style.FramePadding.x * 2);

	static Texture_t* chevronRt = nullptr;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	if (ImGui::BeginChild("Details_Collapse", ImVec2(btnSz, 0), false, ImGuiWindowFlags_NoBackground))
	{
		ImVec2 initial = ImGui::GetCursorPos();

		if (ImGui::Button("##ClearDetails", ImGui::GetWindowSize()))
		{
			this->ClearContent();
		}

		if (chevronRt)
		{
			ImGui::SetCursorPos(ImVec2(initial.x, (ImGui::GetWindowHeight() - btnSz) / 2));
			ImGui::Image(chevronRt->Resource, ImVec2(btnSz, btnSz));
		}
		else
		{
			CContext* ctx = CContext::GetContext();
			CTextureLoader* textureApi = ctx->GetTextureService();
			chevronRt = textureApi->GetOrCreate("ICON_CHEVRON_RT", RES_ICON_CHEVRON_RT, ctx->GetModule());
		}
	}
	ImGui::EndChild();
	ImGui::PopStyleVar();

	ImGui::SameLine();

	if (ImGui::BeginChild("Details_Content", ImVec2(0, 0), false, ImGuiWindowFlags_NoBackground))
	{
		CContext*      ctx = CContext::GetContext();
		CUiContext*    uictx = ctx->GetUIContext();
		CLocalization* langApi = uictx->GetLocalization();

		std::string id;

		if (this->AddonData.GetSig() != 0)
		{
			/* Use addon signature. */
			id = String::Format("0x%08X", this->AddonData.GetSig());
		}
		else
		{
			if (this->AddonData.Addon)
			{
				/* Use addon file location. */
				id = this->AddonData.Addon->GetLocation().string();
			}
			else
			{
				/* This should not be possible. */
				throw "Unreachable code.";
			}
		}

		std::string hashid = "##" + id;

		ImGuiStyle& style = ImGui::GetStyle();

		float btnSz = ImGui::GetFontSize() * 1.5f;

		// helper variable in case we unload or do anything else that might modify the callback before ->Invalidate() is invoked
		bool skipOptions = false;

		std::string headerStr = langApi->Translate("((000099))");
		headerStr.append(" ");
		headerStr.append(this->AddonData.GetName());

		if (ImGui::CollapsingHeader(headerStr.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			Config_t*   config = this->AddonData.Addon->GetConfig();
			CConfigMgr* cfgmgr = ctx->GetCfgMgr();

			/* TODO: Check Update Button */
			/*if (addonData.NexusAddon->Definitions->Provider != EUpdateProvider::Self)
			{
				if (ImGui::Button((!addonData.NexusAddon->IsCheckingForUpdates
					? langApi->Translate("((000035))")
					: langApi->Translate("((000071))") + sigid).c_str()))
				{
					if (!addonData.NexusAddon->IsCheckingForUpdates)
					{
						for (auto addon : loader->Addons)
						{
							if (addon->Definitions == addonData.NexusAddon->Definitions)
							{
								addonData.NexusAddon->IsCheckingForUpdates = true;

								Addon_t* addon = addonData.NexusAddon;

								std::filesystem::path tmpPath = addon->Path.string();
								std::thread([addon, tmpPath]()
								{
									AddonInfo_t addonInfo
									{
										addon->Definitions->Signature,
										addon->Definitions->Name,
										addon->Definitions->Version,
										addon->Definitions->Provider,
										addon->Definitions->UpdateLink != nullptr
											? addon->Definitions->UpdateLink
											: "",
										addon->MD5,
										addon->AllowPrereleases
									};

									CContext* ctx = CContext::GetContext();
									CUpdater* updater = ctx->GetUpdater();
									CUiContext* uictx = ctx->GetUIContext();
									CAlerts* alertctx = uictx->GetAlerts();
									CLocalization* langApi = uictx->GetLocalization();
									CLoader* loader = ctx->GetLoader();

									if (updater->UpdateAddon(tmpPath, addonInfo, false, 5 * 60))
									{
										loader->QueueAddon(ELoaderAction::Reload, tmpPath);

										alertctx->Notify(EAlertType::Info,
											String::Format("%s %s",
											addon->Definitions->Name,
											addon->State == EAddonState::LoadedLOCKED
											? langApi->Translate("((000079))")
											: langApi->Translate("((000081))")
										).c_str()
										);
									}
									else
									{
										alertctx->Notify(EAlertType::Info, String::Format("%s %s",
														 addon->Definitions->Name,
														 langApi->Translate("((000082))")).c_str());
									}
									Sleep(1000); // arbitrary sleep otherwise the user never even sees "is checking..."
									addon->IsCheckingForUpdates = false;
								}).detach();

								//Logger->Debug(CH_GUI, "Update called: %s", it.second->Definitions->Name);
								break;
							}
						}
					}
				}
				if (addonData.NexusAddon->State == EAddonState::LoadedLOCKED)
				{
					ImGui::TooltipGeneric(langApi->Translate("((000012))"));
				}
			}*/

			/* Check Updates Button */
			ImGui::Text("((BTN: Check for Updates))");

			/* Update Button */
			if (this->AddonData.Addon->IsUpdateAvailable())
			{
				ImGui::Text("((BTN: Update))");
			}

			if (config)
			{
				std::string updateMode;
				
				switch (config->UpdateMode)
				{
					default:
					case EUpdateMode::None:
						updateMode = "(null)";
						break;
					case EUpdateMode::Background:
						updateMode = "((Background))";
						break;
					case EUpdateMode::Notify:
						updateMode = "((Notify))";
						break;
					case EUpdateMode::Automatic:
						updateMode = "((Automatic))";
						break;
				}

				/* Update Mode Combo */
				if (ImGui::BeginCombo("##UpdateModeSelector", updateMode.c_str()))
				{
					if (ImGui::Selectable("((Background))", config->UpdateMode == EUpdateMode::Background))
					{
						config->UpdateMode = EUpdateMode::Background;
						cfgmgr->SaveConfigs();
					}

					if (ImGui::Selectable("((Notify))", config->UpdateMode == EUpdateMode::Notify))
					{
						config->UpdateMode = EUpdateMode::Notify;
						cfgmgr->SaveConfigs();
					}

					if (ImGui::Selectable("((Automatic))", config->UpdateMode == EUpdateMode::Automatic))
					{
						config->UpdateMode = EUpdateMode::Automatic;
						cfgmgr->SaveConfigs();
					}

					ImGui::EndCombo();
				}

				/* Pre-releases Checkbox */
				// TODO: if (addonData.Addon->GetProvider()->SupportsPreReleases())
				{
					if (ImGui::Checkbox((langApi->Translate("((000084))") + hashid).c_str(), &config->AllowPreReleases))
					{
						cfgmgr->SaveConfigs();
					}
				}

				/* GitHub Button */
				// TODO: if (addonData.Addon->GetProvider()->GetProjectPage())
				//{
					//if (ImGui::Button((langApi->Translate("((000030))") + id).c_str()))
					//{
					//	ShellExecuteA(0, 0, this->AddonData.GithubURL.c_str(), 0, 0, SW_SHOW);
					//}
				//}

				/* Disable until update Checkbox */
				bool disableUntilUpdate = config->DisableVersion == this->AddonData.Addon->GetMD5().string();
				if (ImGui::Checkbox((langApi->Translate("((000016))") + hashid).c_str(), &disableUntilUpdate))
				{
					this->Invalidate();

					if (disableUntilUpdate)
					{
						config->DisableVersion = this->AddonData.Addon->GetMD5().string();
						this->AddonData.Addon->Unload();
						skipOptions = true;
					}
					else
					{
						config->DisableVersion.clear();
					}
					cfgmgr->SaveConfigs();
				}
				if (this->AddonData.Addon->IsStateLocked())
				{
					ImGui::TooltipGeneric(langApi->Translate("((IsStateLocked))"));
				}

				/* Uninstall Button */
				if (ImGui::Button((langApi->Translate("((000018))") + hashid).c_str(), ImVec2(btnWidth, 0)))
				{
					this->UninstallConfirmationModal.SetTarget(this->AddonData.GetName(), this->AddonData.Addon->GetLocation());
				}
				if (this->AddonData.Addon->IsStateLocked())
				{
					ImGui::TooltipGeneric(langApi->Translate("((IsStateLocked))"));
				}

				/* Load/Unload Button */
				if (LoadUnloadButton(this->AddonData, btnWidth))
				{
					Config_t* config = cfgmgr->RegisterConfig(this->AddonData.GetSig());

					this->LoadConfirmationModal.SetTarget(config, this->AddonData.GetName(), this->AddonData.Addon->GetLocation());
				}
			}
			else
			{
				/* This should be in theory unreachable. */
				ImGui::Text("This addon does not have any configuration options.");
			}
		}

		if (this->AddonData.Addon && this->AddonData.Addon->IsLoaded())
		{
			/* Addon binds table. */
			if (this->AddonData.InputBinds.size() != 0)
			{
				if (ImGui::CollapsingHeader(langApi->Translate("((000060))"), ImGuiTreeNodeFlags_DefaultOpen))
				{
					this->RenderInputBindsTable(this->AddonData.InputBinds);
				}
			}

			/* Addon options callback. */
			if (this->AddonData.OptionsRender && !skipOptions)
			{
				if (ImGui::CollapsingHeader(langApi->Translate("((000004))"), ImGuiTreeNodeFlags_DefaultOpen))
				{
					this->AddonData.OptionsRender();
				}
			}
		}
		else
		{
			/* Notice to load the addon. */
			ImVec2 windowSize = ImGui::GetWindowSize();
			ImVec2 textSize = ImGui::CalcTextSize(langApi->Translate("((000100))"));
			ImVec2 position = ImGui::GetCursorPos();
			ImGui::SetCursorPos(ImVec2((position.x + (windowSize.x - textSize.x)) / 2, (position.y + (windowSize.y - textSize.y)) / 2));
			ImGui::TextDisabled(langApi->Translate("((000100))"));
		}
	}
	ImGui::EndChild();
}

void CAddonsWindow::RenderActionsBar(ImVec2& aSize)
{
	CContext*       ctx    = CContext::GetContext();
	CTextureLoader* texapi = ctx->GetTextureService();
	CLoader*        loader = ctx->GetLoader();
	CUiContext*     uictx  = ctx->GetUIContext();
	CLocalization*  lang   = uictx->GetLocalization();

	if (ImGui::BeginChild("Actions", aSize, false, ImGuiWindowFlags_NoBackground))
	{
		/* Static assets */
		static Texture_t* s_ReloadIcon = texapi->GetOrCreate("ICON_REFRESH", RES_ICON_REFRESH, ctx->GetModule());
		if (!s_ReloadIcon) { s_ReloadIcon = texapi->GetOrCreate("ICON_REFRESH", RES_ICON_REFRESH, ctx->GetModule()); }

		/* Open addons folder */
		if (ImGui::Button(lang->Translate("((000034))")))
		{
			std::string strAddons = Index(EPath::DIR_ADDONS).string();
			ShellExecuteA(NULL, "explore", strAddons.c_str(), NULL, NULL, SW_SHOW);
		}

		ImGui::SameLine();

		static int checkedForUpdates = -1;
		static int queuedForCheck = -1;
		static int updatedCount = -1;

		/* Update all */
		if (ImGui::Button(checkedForUpdates == -1 ? lang->Translate("((000035))") : lang->Translate("((000071))")))
		{
			throw "Not Implemented";
			/*if (checkedForUpdates == -1)
			{
				const std::lock_guard<std::mutex> lock(loader->Mutex);
				{
					checkedForUpdates = 0;
					queuedForCheck = 0;
					updatedCount = 0;

					// pre-iterate to get the count of how many need to be checked, else one call might finish before the count can be incremented
					for (auto addon : loader->Addons)
					{
						if (nullptr == addon->Definitions) { continue; }
						queuedForCheck++;
					}

					if (queuedForCheck == 0)
					{
						checkedForUpdates = -1;
					}

					for (auto addon : loader->Addons)
					{
						if (nullptr == addon->Definitions) { continue; }

						std::filesystem::path tmpPath = addon->Path.string();

						std::thread([this, tmpPath, addon]()
						{
							AddonInfo_t addonInfo
							{
								addon->Definitions->Signature,
								addon->Definitions->Name,
								addon->Definitions->Version,
								addon->Definitions->Provider,
								addon->Definitions->UpdateLink != nullptr
									? addon->Definitions->UpdateLink
									: "",
								addon->MD5,
								addon->AllowPrereleases
							};

							CContext* ctx = CContext::GetContext();
							CUiContext* uictx = ctx->GetUIContext();
							CLocalization* langApi = uictx->GetLocalization();
							CUpdater* updater = ctx->GetUpdater();
							CAlerts* alertctx = uictx->GetAlerts();

							if (addon->Definitions->Provider != EUpdateProvider::Self && updater->UpdateAddon(tmpPath, addonInfo, false, 5 * 60))
							{
								loader->QueueAddon(ELoaderAction::Reload, tmpPath);

								alertctx->Notify(EAlertType::Info,
									String::Format("%s %s",
									addon->Definitions->Name,
									addon->State == EAddonState::LoadedLOCKED
									? langApi->Translate("((000079))")
									: langApi->Translate("((000081))")
								).c_str()
								);

								updatedCount++;
							}
							checkedForUpdates++;

							if (checkedForUpdates == queuedForCheck)
							{
								checkedForUpdates = -1;
								queuedForCheck = 0;

								if (updatedCount == 0)
								{
									alertctx->Notify(EAlertType::Info, langApi->Translate("((000087))"));
								}
							}
						}).detach();
					}
				}
			}*/
		}
		ImGui::TooltipGeneric(lang->Translate("((000036))"));

		if (s_ReloadIcon)
		{
			ImGui::SameLine();

			/* Poll changes */
			if (ImGui::ImageButton(s_ReloadIcon->Resource, ImVec2(ImGui::GetFontSize(), ImGui::GetFontSize())))
			{
				loader->NotifyChanges();
			}
		}

		aSize.y = ImGui::GetCursorPos().y;
	}
	ImGui::EndChild();
}

void CAddonsWindow::RenderSubWindows()
{
	if (this->BindSetterModal.Render() && this->BindSetterModal.GetResult() != EModalResult::None)
	{
		this->Invalidate();
	}

	if (this->UninstallConfirmationModal.Render() && this->UninstallConfirmationModal.GetResult() == EModalResult::OK)
	{
		this->ClearContent();
		this->Invalidate();
	}

	if (this->LoadConfirmationModal.Render() && this->LoadConfirmationModal.GetResult() == EModalResult::OK)
	{
		this->Invalidate();
	}
}

void CAddonsWindow::RenderInputBindsTable(const std::unordered_map<std::string, InputBindPacked_t>& aInputBinds)
{
	if (ImGui::BeginTable("table_inputbinds_addons", 2, ImGuiTableFlags_BordersInnerH))
	{
		CContext* ctx = CContext::GetContext();
		CUiContext* uictx = ctx->GetUIContext();
		CLocalization* langApi = uictx->GetLocalization();

		for (auto& [identifier, inputBind] : aInputBinds)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(langApi->Translate(identifier.c_str()));

			ImGui::TableSetColumnIndex(1);
			ImGui::PushID(identifier.c_str());
			if (ImGui::Button(inputBind.KeysText.c_str(), ImVec2(ImGui::CalcTextSize("XXXXXXXXXXXXXXXXXXXXXXXX").x, 0.0f)))
			{
				this->BindSetterModal.SetTarget(identifier);
			}
			ImGui::PopID();
		}

		ImGui::EndTable();
	}
}

void CAddonsWindow::PopulateAddons()
{
	this->Addons.clear();

	CContext*    ctx = CContext::GetContext();
	CUiContext*  uictx = ctx->GetUIContext();
	CSettings*   settingsctx = ctx->GetSettingsCtx();
	CLoader*     loader = ctx->GetLoader();
	CLibraryMgr* libMgr = ctx->GetAddonLibrary();

	this->Filter = settingsctx->Get<EAddonsFilterFlags>(OPT_ADDONFILTERS, FILTER_INSTALLED);

	for (IAddon* addon : loader->GetAddons())
	{
		AddonListing_t addonlisting{};
		addonlisting.Addon = dynamic_cast<CAddon*>(addon);

		for (GUI_RENDER renderCb : uictx->GetOptionsCallbacks())
		{
			IAddon* owner = loader->GetOwner(renderCb);
			if (addon == owner)
			{
				addonlisting.OptionsRender = renderCb;
				break;
			}
		}

		this->Addons.push_back(addonlisting);

		if (this->HasContent && this->AddonData.GetSig() == addon->GetSignature())
		{
			this->SetContent(addonlisting);
		}
	}

	for (LibraryAddon_t libaddon : libMgr->GetLibrary())
	{
		bool installed = false;

		for (AddonListing_t& addonlisting : this->Addons)
		{
			if (addonlisting.GetSig() == libaddon.Signature)
			{
				installed = true;
				addonlisting.HasLibDef = true;
				addonlisting.LibraryDef = libaddon;
			}
		}

		if (!installed)
		{
			AddonListing_t addonlisting{};
			addonlisting.HasLibDef = true;
			addonlisting.LibraryDef = libaddon;
			this->Addons.push_back(addonlisting);
		}
	}
	
	this->AddonsAmtUnfiltered = static_cast<uint32_t>(this->Addons.size());

	for (auto it = this->Addons.begin(); it != this->Addons.end();)
	{
		bool matchesFilter = false;

		if ((this->Filter & EAddonsFilterFlags::ShowEnabled) == EAddonsFilterFlags::ShowEnabled)
		{
			/* Has local addon and is loaded -> Enabled */
			if (it->Addon && it->Addon->IsLoaded())
			{
				matchesFilter = true;
			}
		}

		if ((this->Filter & EAddonsFilterFlags::ShowDisabled) == EAddonsFilterFlags::ShowDisabled)
		{
			/* Has local addon and is not loaded -> Disabled */
			if (it->Addon && !it->Addon->IsLoaded())
			{
				matchesFilter = true;
			}
		}

		if ((this->Filter & EAddonsFilterFlags::ShowDownloadable) == EAddonsFilterFlags::ShowDownloadable)
		{
			/* Has no local addon, but a libdef -> Downloadable */
			if (!it->Addon && it->HasLibDef)
			{
				matchesFilter = true;
			}
		}

		if (!matchesFilter)
		{
			/* Doesn't match any filter flags -> Remove */
			it = this->Addons.erase(it);
		}
		else if (!this->SearchTerm.empty() &&
			!(String::Contains(String::ToLower(it->GetName()), this->SearchTerm) ||
			String::Contains(String::ToLower(it->GetDesc()), this->SearchTerm) ||
			String::Contains(String::ToLower(it->GetAuthor()), this->SearchTerm)))
		{
			/* Term matches name or description or author -> Filter */
			it = this->Addons.erase(it);
		}
		else
		{
			/* Continue to next addon listing. */
			it++;
		}
	}

	std::sort(this->Addons.begin(), this->Addons.end(), [](AddonListing_t& lhs, AddonListing_t& rhs)
	{
		Config_t* lhsConfig = lhs.Addon ? lhs.Addon->GetConfig() : nullptr;
		Config_t* rhsConfig = rhs.Addon ? rhs.Addon->GetConfig() : nullptr;

		// 1. Addons without config first
		if (lhsConfig == nullptr || rhsConfig == nullptr)
		{
			if (lhsConfig == nullptr && rhsConfig != nullptr)
			{
				return true;
			}
			if (lhsConfig != nullptr && rhsConfig == nullptr)
			{
				return false;
			}
		}

		// 2. Favorites first
		if (lhsConfig && rhsConfig)
		{
			if (lhsConfig->IsFavorite != rhsConfig->IsFavorite)
			{
				return lhsConfig->IsFavorite > rhsConfig->IsFavorite;
			}

			// 3. Non-empty DisableVersion first
			const bool lhsHasDisable = !lhsConfig->DisableVersion.empty();
			const bool rhsHasDisable = !rhsConfig->DisableVersion.empty();
			if (lhsHasDisable != rhsHasDisable)
			{
				return lhsHasDisable > rhsHasDisable;
			}
		}

		// 4. Case-insensitive alphabetical order
		return String::ToLower(lhs.GetName()) < String::ToLower(rhs.GetName());
	});
}
