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
#include "Resources/ResConst.h"
#include "Util/Strings.h"

constexpr EAddonsFilterFlags FILTER_INSTALLED
	= EAddonsFilterFlags::ShowEnabled
	| EAddonsFilterFlags::ShowDisabled;

constexpr EAddonsFilterFlags FILTER_LIBRARY = EAddonsFilterFlags::ShowDownloadable;

static CAddonsWindow* AddonsWindow = nullptr;

void OnAddonLoadUnload(void* aEventData)
{
	if (!AddonsWindow) { return; }

	AddonsWindow->Invalidate(*(signed int*)aEventData);
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

	CContext* ctx = CContext::GetContext();
	CEventApi* evtapi = ctx->GetEventApi();

	evtapi->Subscribe("EV_ADDON_LOADED", OnAddonLoadUnload);
	evtapi->Subscribe("EV_ADDON_UNLOADED", OnAddonLoadUnload);
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

	ImVec2 itemSz = ImVec2(aWidth, ImGui::GetTextLineHeightWithSpacing() * 5);
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

	static CContext* ctx = CContext::GetContext();
	static CUiContext* uictx = ctx->GetUIContext();
	static CLocalization* langApi = uictx->GetLocalization();
	static CAlerts* alertctx = uictx->GetAlerts();
	static CConfigMgr* cfgmgr = ctx->GetCfgMgr();

	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1);
	if (ImGui::BeginChild(("Addon_" + id).c_str(), itemSz, true))
	{
		if (ImGui::BeginChild("Info", ImVec2(itemSz.x - style.ItemSpacing.x - actionsAreaWidth - (style.WindowPadding.x * 2), innerHeight), false, ImGuiWindowFlags_NoBackground))
		{
			/* TODO / FIXME : Add error state, like invalid api as a flag/tag. */

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
				ImGui::TextDisabled("by %s", aAddonData.GetAuthor().c_str());
			}

			/* Description */
			if (!aAddonData.GetDesc().empty())
			{
				ImGui::TextWrapped(aAddonData.GetDesc().c_str());
			}
		}
		ImGui::EndChild();

		ImGui::SameLine();

		/* Helper to not affect window padding in tooltips. */
		bool poppedActionsPad = false;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		if (ImGui::BeginChild("Actions", ImVec2(actionsAreaWidth, innerHeight), false, ImGuiWindowFlags_NoBackground))
		{
			ImGui::PopStyleVar();
			poppedActionsPad = true;

			float initialX = ImGui::GetCursorPosX();

			/* If only libdef, show install and github button. */
			if (aAddonData.HasLibDef && !aAddonData.Addon)
			{
				/* Install button */
				ImGui::SetCursorPos(ImVec2(initialX, (ImGui::GetWindowHeight() - (btnTextSz.y * 2) - style.ItemSpacing.y - (style.FramePadding.y * 2)) / 2));
				if (ImGui::Button(aAddonData.IsInstalling
					? (langApi->Translate("((000027))") + id).c_str()
					: (langApi->Translate("((000028))") + id).c_str(),
					ImVec2(btnWidth, 0)))
				{
					if (!aAddonData.IsInstalling)
					{
						CLibraryMgr* library = CContext::GetContext()->GetAddonLibrary();
						library->Install(aAddonData.GetSig());
					}
				}

				/* GitHub button */
				if (!aAddonData.GithubURL.empty())
				{
					ImGui::SetCursorPos(ImVec2(initialX, ImGui::GetCursorPosY()));
					if (ImGui::Button((langApi->Translate("((000030))") + id).c_str(), ImVec2(btnWidth, 0)))
					{
						ShellExecuteA(0, 0, aAddonData.GithubURL.c_str(), 0, 0, SW_SHOW);
					}
				}
			}

			/* If nexusdef, show load/unload and configure button. */
			if (aAddonData.Addon)
			{
				std::string buttonText;
				std::string buttonTT;

				if (aAddonData.Addon->IsLoaded())
				{
					buttonText = "((Disable))";
				}
				else
				{
					buttonText = "((Load))";
				}

				if (aAddonData.Addon->IsStateLocked())
				{
					buttonTT = "((Loading or unloading won't take effect until next game start.))";
				}

				if (cfgmgr->IsReadOnly())
				{
					buttonTT = "((Addon state won't be saved. Game was started with addons via start parameter.))";
				}

				/* Load/Unload button */
				ImGui::SetCursorPos(ImVec2(initialX, (ImGui::GetWindowHeight() - (btnTextSz.y * 2) - style.ItemSpacing.y - (style.FramePadding.y * 2)) / 2));
				if (ImGui::Button(buttonText.c_str(), ImVec2(btnWidth, 0)))
				{
					if (!aAddonData.Addon->IsStateLocked())
					{
						if (aAddonData.Addon->IsLoaded())
						{
							aAddonData.Addon->Unload();
						}
						else
						{
							aAddonData.Addon->Load();
							// TODO: Signal to save the ShouldLoad config flag somehow.
						}
					}
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::TooltipGeneric(buttonTT.c_str());
				}

				/* Configure button */
				ImGui::SetCursorPos(ImVec2(initialX, ImGui::GetCursorPosY()));
				if (ImGui::Button(langApi->Translate("((000105))"), ImVec2(btnWidth, 0)))
				{
					this->SetContent(aAddonData);
				}
			}

			/* If arc extension only, show notice. */
			/*if (aAddonData.HasArcDef && !aAddonData.HasNexusDef)
			{
				float width = ImGui::CalcTextSize("via ArcDPS").x;
				ImGui::SetCursorPosX((btnWidth - width) / 2);
				ImGui::Text("via ArcDPS");
			}*/
		}
		ImGui::EndChild();

		if (!poppedActionsPad)
		{
			ImGui::PopStyleVar();
		}
	}
	ImGui::EndChild();

	ImGui::PopStyleVar();
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

	static char searchTerm[400] = {};
	
	static CContext* ctx = CContext::GetContext();
	static CUiContext* uictx = ctx->GetUIContext();
	static CLocalization* langApi = uictx->GetLocalization();
	static CSettings* settingsctx = ctx->GetSettingsCtx();
	static CLoaderBase* loader = ctx->GetLoaderBase();

	ImVec2 region = ImGui::GetContentRegionAvail();

	static float filterAreaEndY = region.y;
	ImVec2 filterAreaSz = ImVec2(region.x, filterAreaEndY);

	ImGuiStyle& style = ImGui::GetStyle();

	if (!this->IsPoppedOut)
	{
		float btnSz = ImGui::GetFontSize() * 1.5f;
		filterAreaSz.x = region.x - style.ItemSpacing.x - btnSz - style.ItemSpacing.x;
	}

	static float actionsAreaEndY = region.y;
	ImVec2 actionsAreaSz = ImVec2(region.x, actionsAreaEndY);

	//static float detailsAreaWidth = 0;

	bool configuring = this->HasContent;

	/*if (configuring)
	{
		detailsAreaWidth = region.x;
	}
	else
	{
		detailsAreaWidth = 0;
	}*/

	//ImVec2 detailsAreaSz = ImVec2(detailsAreaWidth - style.ItemSpacing.x, region.y - filterAreaSz.y - actionsAreaSz.y - style.ItemSpacing.y - style.ItemSpacing.y);
	ImVec2 listAreaSz = ImVec2(region.x, region.y - filterAreaSz.y - actionsAreaSz.y - style.ItemSpacing.y - style.ItemSpacing.y);

	static bool isListMode = settingsctx->Get<bool>(OPT_ISLISTMODE, false);
	
	if (ImGui::BeginChild("Filters", filterAreaSz, false, ImGuiWindowFlags_NoBackground))
	{
		/* search term */
		if (ImGui::InputTextWithHint("##SearchTerm", langApi->Translate("((000104))"), &searchTerm[0], 400))
		{
			this->SearchTerm = String::ToLower(searchTerm);
			this->Invalidate();
			this->ClearContent();
		}

		/* clear search term */
		static Texture_t* clearSearchTermTex = nullptr;
		if (clearSearchTermTex)
		{
			ImGui::SameLine();
			if (ImGui::ImageButton(clearSearchTermTex->Resource, ImVec2(ImGui::GetFontSize(), ImGui::GetFontSize())))
			{
				memset(searchTerm, 0, 400);
				this->SearchTerm = String::ToLower(searchTerm);
				this->Invalidate();
				this->ClearContent();
			}
		}
		else
		{
			CTextureLoader* texapi = ctx->GetTextureService();
			clearSearchTermTex = texapi->GetOrCreate("ICON_CLOSE", RES_ICON_CLOSE, ctx->GetModule());
		}

		/* view mode */
		static Texture_t* viewModeTex = nullptr;
		if (viewModeTex)
		{
			ImGui::SameLine();
			if (ImGui::ImageButton(viewModeTex->Resource, ImVec2(ImGui::GetFontSize(), ImGui::GetFontSize())))
			{
				viewModeTex = nullptr;
				isListMode = !isListMode;
				settingsctx->Set(OPT_ISLISTMODE, isListMode);
			}
		}
		else
		{
			CTextureLoader* texapi = ctx->GetTextureService();
			viewModeTex = !isListMode
				? texapi->GetOrCreate("ICON_LIST", RES_ICON_LIST, ctx->GetModule())
				: texapi->GetOrCreate("ICON_TILES", RES_ICON_TILES, ctx->GetModule());
		}

		/* advanced filters */
		bool doPopHighlight = false;
		
		static Texture_t* filtersTex = nullptr;
		if (filtersTex)
		{
			/* If filter does not match quick filter, highlight manual filter icon. */
			if (this->Filter != FILTER_INSTALLED && this->Filter != FILTER_LIBRARY)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_ButtonActive]);
				doPopHighlight = true;
			}

			if (ImGui::ImageButton(filtersTex->Resource, ImVec2(ImGui::GetFontSize(), ImGui::GetFontSize())))
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
				if (ImGui::Checkbox(langApi->Translate("((000106))"), &showEnabled))
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
				if (ImGui::Checkbox(langApi->Translate("((000107))"), &showDisabled))
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
				if (ImGui::Checkbox(langApi->Translate("((000108))"), &showDownloadable))
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

				/*if (ArcDPS::IsLoaded)
				{
					if (ImGui::Checkbox(langApi->Translate("((000109))"), &showInstalledArc))
					{
						if (showInstalledArc)
						{
							this->Filter = (EAddonsFilterFlags)((int)this->Filter | (int)EAddonsFilterFlags::ShowInstalled_Arc);
						}
						else
						{
							this->Filter = (EAddonsFilterFlags)((int)this->Filter & ~(int)EAddonsFilterFlags::ShowInstalled_Arc);
						}
						settingsctx->Set(OPT_ADDONFILTERS, this->Filter);
						this->Invalidate();
						this->ClearContent();
					}

					if (ImGui::Checkbox(langApi->Translate("((000110))"), &showDownloadableArc))
					{
						if (showDownloadableArc)
						{
							this->Filter = (EAddonsFilterFlags)((int)this->Filter | (int)EAddonsFilterFlags::ShowDownloadable_Arc);
						}
						else
						{
							this->Filter = (EAddonsFilterFlags)((int)this->Filter & ~(int)EAddonsFilterFlags::ShowDownloadable_Arc);
						}
						settingsctx->Set(OPT_ADDONFILTERS, this->Filter);
						this->Invalidate();
						this->ClearContent();
					}
				}*/

				ImGui::EndPopup();
			}
		}
		else
		{
			CTextureLoader* texapi = ctx->GetTextureService();
			filtersTex = texapi->GetOrCreate("ICON_FILTER", RES_ICON_FILTER, ctx->GetModule());
		}

		ImGui::SameLine();

		/* quick filters */
		
		if ((this->Filter & FILTER_INSTALLED) == FILTER_INSTALLED &&
			(this->Filter & ~FILTER_INSTALLED) == EAddonsFilterFlags::None)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_ButtonActive]);
			doPopHighlight = true;
		}

		/* Quick Filter: Installed */
		if (ImGui::Button(langApi->Translate("((000031))")))
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

		if ((this->Filter & FILTER_LIBRARY) == FILTER_LIBRARY &&
			(this->Filter & ~FILTER_LIBRARY) == EAddonsFilterFlags::None)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_ButtonActive]);
			doPopHighlight = true;
		}

		/* Quick Filter: Library */
		if (ImGui::Button(langApi->Translate("((000032))")))
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

		/*if (ArcDPS::IsLoaded)
		{
			ImGui::SameLine();

			std::string legacyNotice = langApi->Translate("((000076))");
			legacyNotice.append("\n");
			legacyNotice.append(langApi->Translate("((000077))"));

			if (this->Filter == quickFilter_ArcPlugins)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_ButtonActive]);
				doPopHighlight = true;
			}

			if (ImGui::Button(langApi->Translate("((000075))")))
			{
				this->Filter = quickFilter_ArcPlugins;
				settingsctx->Set(OPT_ADDONFILTERS, this->Filter);
				this->Invalidate();
				this->ClearContent();
			}

			if (doPopHighlight)
			{
				ImGui::PopStyleColor();
				doPopHighlight = false;
			}

			ImGui::TooltipGeneric(legacyNotice.c_str());
		}*/

		filterAreaEndY = ImGui::GetCursorPos().y;
	}
	ImGui::EndChild();
	
	float addonItemWidth = isListMode
		? (region.x - style.ScrollbarSize)
		: ((region.x - style.ItemSpacing.x - style.ScrollbarSize) / 2);

	/* list */
	//ImVec2 listP1 = ImGui::GetWindowPos();
	//ImVec2 listP2 = ImVec2(listP1.x + listAreaSz.x - detailsAreaWidth, listP1.y + listAreaSz.y + filterAreaSz.y + style.ItemSpacing.y);

	//ImVec2 posList = ImGui::GetCursorPos();

	//ImGui::PushClipRect(listP1, listP2, true);
	if (ImGui::BeginChild("List", listAreaSz, false, ImGuiWindowFlags_NoBackground))
	{
		if (configuring)
		{
			this->RenderDetails();
		}
		else
		{
			if (this->Addons.size() == 0)
			{
				ImVec2 windowSize = ImGui::GetWindowSize();
				ImVec2 textSize = ImGui::CalcTextSize(langApi->Translate("((000098))"));
				ImVec2 position = ImGui::GetCursorPos();
				ImGui::SetCursorPos(ImVec2((position.x + (windowSize.x - textSize.x)) / 2, (position.y + (windowSize.y - textSize.y)) / 2));
				ImGui::TextDisabled(langApi->Translate("((000098))"));
			}
			else
			{
				int i = 0;

				for (AddonListing_t& addon : this->Addons)
				{
					if (!isListMode && i % 2 == 1)
					{
						ImGui::SameLine();
					}

					AddonItem(addon, addonItemWidth);
					i++;
				}
			}
		}
	}
	ImGui::EndChild();
	//ImGui::PopClipRect();

	/* details */
	/*ImGui::SetCursorPos(ImVec2(posList.x + (region.x - detailsAreaWidth) + style.ItemSpacing.x, posList.y));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1);
	bool poppedBorder = false;
	if (ImGui::BeginChild("Details", detailsAreaSz, true))
	{
		poppedBorder = true;
		ImGui::PopStyleVar();

		if (detailsExpanded)
		{
			this->RenderDetails();
		}
	}
	ImGui::EndChild();

	if (!poppedBorder)
	{
		ImGui::PopStyleVar();
	}*/
	
	if (ImGui::BeginChild("Actions", actionsAreaSz, false, ImGuiWindowFlags_NoBackground))
	{
		if (ImGui::Button(langApi->Translate("((000034))")))
		{
			std::string strAddons = Index(EPath::DIR_ADDONS).string();
			ShellExecuteA(NULL, "explore", strAddons.c_str(), NULL, NULL, SW_SHOW);
		}

		ImGui::SameLine();

		static int checkedForUpdates = -1;
		static int queuedForCheck = -1;
		static int updatedCount = -1;

		if (ImGui::Button(checkedForUpdates == -1 ? langApi->Translate("((000035))") : langApi->Translate("((000071))")))
		{
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
		ImGui::TooltipGeneric(langApi->Translate("((000036))"));

		static Texture_t* refreshTex = nullptr;
		if (refreshTex)
		{
			ImGui::SameLine();

			if (ImGui::ImageButton(refreshTex->Resource, ImVec2(ImGui::GetFontSize(), ImGui::GetFontSize())))
			{
				loader->NotifyChanges();
			}
		}
		else
		{
			CTextureLoader* texapi = ctx->GetTextureService();
			refreshTex = texapi->GetOrCreate("ICON_REFRESH", RES_ICON_REFRESH, ctx->GetModule());
		}

		actionsAreaEndY = ImGui::GetCursorPos().y;
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
}

void CAddonsWindow::RenderDetails()
{
	AddonListing_t& addonData = this->AddonData;

	ImGuiStyle& style = ImGui::GetStyle();

	float btnSz = ImGui::GetFontSize() * 1.5f;

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
		static CContext* ctx = CContext::GetContext();
		static CUiContext* uictx = ctx->GetUIContext();
		static CLocalization* langApi = uictx->GetLocalization();

		std::string sig = std::to_string(addonData.GetSig());
		std::string sigid = "##" + sig;

		ImGuiStyle& style = ImGui::GetStyle();

		float btnSz = ImGui::GetFontSize() * 1.5f;

		// helper variable in case we unload or do anything else that might modify the callback before ->Invalidate() is invoked
		bool skipOptions = false;

		std::string headerStr = langApi->Translate("((000099))");
		headerStr.append(" ");
		headerStr.append(addonData.GetName());

		/*if (ImGui::CollapsingHeader(headerStr.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (addonData.NexusAddon->Definitions->Provider != EUpdateProvider::Self)
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
			}
			
			if (!addonData.GithubURL.empty())
			{
				ImGui::SameLine();
				if (ImGui::Button((langApi->Translate("((000030))") + sig).c_str()))
				{
					ShellExecuteA(0, 0, addonData.GithubURL.c_str(), 0, 0, SW_SHOW);
				}
			}

			if (ImGui::Checkbox((langApi->Translate("((000014))") + sigid).c_str(), &addonData.NexusAddon->IsPausingUpdates))
			{
				loader->SaveAddonConfig();
			}

			if (ImGui::Checkbox((langApi->Translate("((000016))") + sigid).c_str(), &addonData.NexusAddon->IsDisabledUntilUpdate))
			{
				//Logger->Debug(CH_GUI, "ToggleDUU called: %s", it.second->Definitions->Name);

				loader->SaveAddonConfig();
				this->Invalidate();

				if (addonData.NexusAddon->State == EAddonState::Loaded)
				{
					loader->QueueAddon(ELoaderAction::Unload, addonData.NexusAddon->Path);
					skipOptions = true;
				}
			}
			if (addonData.NexusAddon->State == EAddonState::LoadedLOCKED)
			{
				ImGui::TooltipGeneric(langApi->Translate("((000017))"));
			}

			if (addonData.NexusAddon->Definitions->Provider == EUpdateProvider::GitHub)
			{
				if (ImGui::Checkbox((langApi->Translate("((000084))") + sigid).c_str(), &addonData.NexusAddon->AllowPrereleases))
				{
					loader->SaveAddonConfig();
				}
			}

			// Load/Unload Button
			// just check if loaded, if it was not hot-reloadable it would be EAddonState::LoadedLOCKED
			if (addonData.NexusAddon->State == EAddonState::Loaded)
			{
				if (ImGui::Button((addonData.NexusAddon->IsWaitingForUnload ? langApi->Translate("((000078))") : langApi->Translate("((000020))") + sig).c_str()))
				{
					if (!addonData.NexusAddon->IsWaitingForUnload)
					{
						//Logger->Debug(CH_GUI, "Unload called: %s", it.second->Definitions->Name);
						loader->QueueAddon(ELoaderAction::Unload, addonData.NexusAddon->Path);
					}
				}
				if (loader->HasCustomConfig)
				{
					ImGui::TooltipGeneric(langApi->Translate("((000021))"));
				}
			}
			else if (addonData.NexusAddon->State == EAddonState::LoadedLOCKED)
			{
				std::string additionalInfo;

				if (loader->HasCustomConfig)
				{
					additionalInfo.append("\n");
					additionalInfo.append(langApi->Translate("((000021))"));
				}

				if (ImGui::Button((langApi->Translate(addonData.NexusAddon->IsFlaggedForDisable ? "((000024))" : "((000022))") + sig).c_str()))
				{
					addonData.NexusAddon->IsFlaggedForDisable = !addonData.NexusAddon->IsFlaggedForDisable;
					loader->SaveAddonConfig();
				}
				ImGui::TooltipGeneric(langApi->Translate(addonData.NexusAddon->IsFlaggedForDisable ? "((000025))" : "((000023))"), additionalInfo.c_str());
			}
			else if (addonData.NexusAddon->State == EAddonState::NotLoaded && (addonData.NexusAddon->Definitions->HasFlag(EAddonFlags::OnlyLoadDuringGameLaunchSequence) || addonData.NexusAddon->Definitions->Signature == 0xFFF694D1) && !loader->IsGameLaunchSequence)
			{
				// if it's too late to load this addon
				if (ImGui::Button((langApi->Translate(addonData.NexusAddon->IsFlaggedForEnable ? "((000020))" : "((000024))") + sig).c_str()))
				{
					addonData.NexusAddon->IsFlaggedForEnable = !addonData.NexusAddon->IsFlaggedForEnable;

					if (addonData.NexusAddon->IsFlaggedForEnable)
					{
						addonData.NexusAddon->IsDisabledUntilUpdate = false; // explicitly loaded
						ctx->GetUIContext()->GetAlerts()->Notify(EAlertType::Info, String::Format("%s %s", addonData.NexusAddon->Definitions->Name, langApi->Translate("((000080))")).c_str());
					}

					loader->SaveAddonConfig();
				}
				if (addonData.NexusAddon->IsFlaggedForEnable)
				{
					ImGui::TooltipGeneric(langApi->Translate("((000025))"), "");
				}
			}
			else if (addonData.NexusAddon->State == EAddonState::NotLoaded)
			{
				if (ImGui::Button((langApi->Translate("((000026))") + sig).c_str()))
				{
					//Logger->Debug(CH_GUI, "Load called: %s", it.second->Definitions->Name);
					addonData.NexusAddon->IsDisabledUntilUpdate = false; // explicitly loaded
					loader->QueueAddon(ELoaderAction::Load, addonData.NexusAddon->Path);
				}
				if (loader->HasCustomConfig)
				{
					ImGui::TooltipGeneric(langApi->Translate("((000021))"));
				}
			}

			ImGui::SameLine();

			if (ImGui::Button((langApi->Translate("((000018))") + sigid).c_str()))
			{
				this->UninstallConfirmationModal.SetTarget(addonData.NexusAddon->Definitions->Name, addonData.NexusAddon->Path);
			}
			if (addonData.NexusAddon->State == EAddonState::LoadedLOCKED)
			{
				ImGui::TooltipGeneric(langApi->Translate("((000019))"));
			}
		}*/

		/*if (!(addonData.NexusAddon->State == EAddonState::Loaded || addonData.NexusAddon->State == EAddonState::LoadedLOCKED))
		{
			ImVec2 windowSize = ImGui::GetWindowSize();
			ImVec2 textSize = ImGui::CalcTextSize(langApi->Translate("((000100))"));
			ImVec2 position = ImGui::GetCursorPos();
			ImGui::SetCursorPos(ImVec2((position.x + (windowSize.x - textSize.x)) / 2, (position.y + (windowSize.y - textSize.y)) / 2));
			ImGui::TextDisabled(langApi->Translate("((000100))"));
		}
		else
		{
			if (addonData.InputBinds.size() != 0)
			{
				if (ImGui::CollapsingHeader(langApi->Translate("((000060))"), ImGuiTreeNodeFlags_DefaultOpen))
				{
					this->RenderInputBindsTable(addonData.InputBinds);
				}
			}

			if (addonData.OptionsRender && !skipOptions)
			{
				if (ImGui::CollapsingHeader(langApi->Translate("((000004))"), ImGuiTreeNodeFlags_DefaultOpen))
				{
					addonData.OptionsRender();
				}
			}
		}*/
	}
	ImGui::EndChild();
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
	CLoaderBase* loader = ctx->GetLoaderBase();
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
				if (String::Contains(libaddon.DownloadURL, "https://github.com"))
				{
					addonlisting.GithubURL = libaddon.DownloadURL;
				}
			}
		}

		if (!installed)
		{
			AddonListing_t addonlisting{};
			addonlisting.HasLibDef = true;
			addonlisting.LibraryDef = libaddon;
			if (String::Contains(libaddon.DownloadURL, "https://github.com"))
			{
				addonlisting.GithubURL = libaddon.DownloadURL;
			}
			this->Addons.push_back(addonlisting);
		}
	}
	
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
		return String::ToLower(lhs.GetName()) < String::ToLower(rhs.GetName());
	});
}
