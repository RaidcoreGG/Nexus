///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Addons.cpp
/// Description  :  Contains the content of the addons window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Addons.h"

#include <shellapi.h>

#include "imgui_extensions.h"
#include "imgui/imgui.h"

#include "Core/Context.h"
#include "resource.h"
#include "Shared.h"
#include "Util/Strings.h"

#include "Engine/Index/Index.h"
#include "Engine/Loader/ArcDPS.h"
#include "Engine/Loader/Library.h"
#include "Engine/Loader/Loader.h"

constexpr ImGuiWindowFlags ModalFlags = ImGuiWindowFlags_AlwaysAutoResize |
										ImGuiWindowFlags_NoResize |
										ImGuiWindowFlags_NoCollapse;

constexpr EAddonsFilterFlags quickFilter_Installed = (EAddonsFilterFlags)((int)EAddonsFilterFlags::ShowEnabled | (int)EAddonsFilterFlags::ShowDisabled | (int)EAddonsFilterFlags::ShowInstalled_Arc);
constexpr EAddonsFilterFlags quickFilter_Library = EAddonsFilterFlags::ShowDownloadable;
constexpr EAddonsFilterFlags quickFilter_ArcPlugins = EAddonsFilterFlags::ShowDownloadable_Arc;

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
}

void CAddonsWindow::SetContent(AddonItemData_t& aAddonData)
{
	CContext* ctx = CContext::GetContext();
	CUiContext* uictx = ctx->GetUIContext();

	this->AddonData = aAddonData;
	this->AddonData.InputBinds = uictx->GetInputBinds(this->AddonData.NexusAddon->Definitions->Name);
	this->HasContent = true;
}

void CAddonsWindow::ClearContent()
{
	this->AddonData.InputBinds.clear();
	this->HasContent = false;
}

void CAddonsWindow::AddonItem(AddonItemData_t& aAddonData, float aWidth)
{
	if (aAddonData.Type == EAddonType::Nexus)
	{
		if (!aAddonData.NexusAddon || !aAddonData.NexusAddon->Definitions) { return; }

		std::string sig = std::to_string(aAddonData.NexusAddon->Definitions->Signature);

		ImGuiStyle& style = ImGui::GetStyle();

		ImVec2 itemSz = ImVec2(aWidth, ImGui::GetTextLineHeightWithSpacing() * 5);
		float innerHeight = itemSz.y - (style.WindowPadding.y * 2);

		ImVec2 btnTextSz = ImGui::CalcTextSize("############");

		float btnWidth = btnTextSz.x + (style.FramePadding.x * 2);
		float actionsAreaWidth = btnWidth;

		ImVec2 curPos = ImGui::GetCursorPos();

		bool hoveredFavorite = false;
		bool clickedFavorite = false;

		/* above visible space                        || under visible space */
		if (curPos.y < ImGui::GetScrollY() - itemSz.y || curPos.y > ImGui::GetScrollY() + ImGui::GetWindowHeight())
		{
			ImGui::Dummy(itemSz);
			return;
		}

		bool isduu = aAddonData.NexusAddon->IsDisabledUntilUpdate;

		if (isduu)
		{
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(.675f, .349f, .349f, 1.0f));
		}

		bool poppedCol = false;

		static CContext*      ctx     = CContext::GetContext();
		static CUiContext*    uictx   = ctx->GetUIContext();
		static CLocalization* langApi = uictx->GetLocalization();

		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1);
		if (ImGui::BeginChild(("AddonItem_" + sig).c_str(), itemSz, true))
		{
			if (isduu)
			{
				ImGui::PopStyleColor();
				poppedCol = true;
			}

			static CUiContext* uictx = ctx->GetUIContext();
			static CAlerts* alertctx = uictx->GetAlerts();

			if (ImGui::BeginChild("Info", ImVec2(itemSz.x - style.ItemSpacing.x - actionsAreaWidth - (style.WindowPadding.x * 2), innerHeight), false, ImGuiWindowFlags_NoBackground))
			{
				ImGui::Text(aAddonData.NexusAddon->Definitions->Name);
				ImGui::SameLine();
				ImGui::TextDisabled(aAddonData.NexusAddon->Definitions->Version.string().c_str());
				ImGui::TextDisabled("by %s", aAddonData.NexusAddon->Definitions->Author);

				/* Incompatible notice or description */
				if (aAddonData.NexusAddon->State == EAddonState::NotLoadedIncompatibleAPI)
				{
					ImGui::TextColored(ImVec4(255, 255, 0, 255), langApi->Translate("((000010))"), aAddonData.NexusAddon->Definitions->APIVersion);
				}
				else
				{
					ImGui::TextWrapped(aAddonData.NexusAddon->Definitions->Description);
				}
			}
			ImGui::EndChild();

			ImGui::SameLine();

			bool poppedActionsPad = false;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			if (ImGui::BeginChild("Actions", ImVec2(actionsAreaWidth, innerHeight), false, ImGuiWindowFlags_NoBackground))
			{
				ImGui::PopStyleVar();
				poppedActionsPad = true;

				float initialX = ImGui::GetCursorPosX();

				/* Load/Unload Button */
				ImGui::SetCursorPos(ImVec2(initialX, (ImGui::GetWindowHeight() - (btnTextSz.y * 2) - style.ItemSpacing.y - (style.FramePadding.y * 2)) / 2));
				// just check if loaded, if it was not hot-reloadable it would be EAddonState::LoadedLOCKED
				if (aAddonData.NexusAddon->State == EAddonState::Loaded)
				{
					if (ImGui::Button((aAddonData.NexusAddon->IsWaitingForUnload ? langApi->Translate("((000078))") : langApi->Translate("((000020))") + sig).c_str(), ImVec2(btnWidth, 0)))
					{
						if (!aAddonData.NexusAddon->IsWaitingForUnload)
						{
							//Logger->Debug(CH_GUI, "Unload called: %s", it.second->Definitions->Name);
							Loader::QueueAddon(ELoaderAction::Unload, aAddonData.NexusAddon->Path);
						}
					}
					if (Loader::HasCustomConfig)
					{
						ImGui::TooltipGeneric(langApi->Translate("((000021))"));
					}
				}
				else if (aAddonData.NexusAddon->State == EAddonState::LoadedLOCKED)
				{
					std::string additionalInfo;

					if (Loader::HasCustomConfig)
					{
						additionalInfo.append("\n");
						additionalInfo.append(langApi->Translate("((000021))"));
					}

					if (ImGui::Button((langApi->Translate(aAddonData.NexusAddon->IsFlaggedForDisable ? "((000024))" : "((000022))") + sig).c_str(), ImVec2(btnWidth, 0)))
					{
						aAddonData.NexusAddon->IsFlaggedForDisable = !aAddonData.NexusAddon->IsFlaggedForDisable;
						Loader::SaveAddonConfig();
					}
					ImGui::TooltipGeneric(langApi->Translate(aAddonData.NexusAddon->IsFlaggedForDisable ? "((000025))" : "((000023))"), additionalInfo.c_str());
				}
				else if (aAddonData.NexusAddon->State == EAddonState::NotLoaded && (aAddonData.NexusAddon->Definitions->HasFlag(EAddonFlags::OnlyLoadDuringGameLaunchSequence) || aAddonData.NexusAddon->Definitions->Signature == 0xFFF694D1) && !IsGameLaunchSequence)
				{
					/* if it's too late to load this addon */
					if (ImGui::Button((langApi->Translate(aAddonData.NexusAddon->IsFlaggedForEnable ? "((000020))" : "((000024))") + sig).c_str(), ImVec2(btnWidth, 0)))
					{
						aAddonData.NexusAddon->IsFlaggedForEnable = !aAddonData.NexusAddon->IsFlaggedForEnable;

						if (aAddonData.NexusAddon->IsFlaggedForEnable)
						{
							aAddonData.NexusAddon->IsDisabledUntilUpdate = false; // explicitly loaded
							alertctx->Notify(EAlertType::Info, String::Format("%s %s", aAddonData.NexusAddon->Definitions->Name, langApi->Translate("((000080))")).c_str());
						}

						Loader::SaveAddonConfig();
					}
					if (aAddonData.NexusAddon->IsFlaggedForEnable)
					{
						ImGui::TooltipGeneric(langApi->Translate("((000025))"), "");
					}
				}
				else if (aAddonData.NexusAddon->State == EAddonState::NotLoaded)
				{
					if (ImGui::Button((langApi->Translate("((000026))") + sig).c_str(), ImVec2(btnWidth, 0)))
					{
						//Logger->Debug(CH_GUI, "Load called: %s", it.second->Definitions->Name);
						aAddonData.NexusAddon->IsDisabledUntilUpdate = false; // explicitly loaded
						Loader::QueueAddon(ELoaderAction::Load, aAddonData.NexusAddon->Path);
					}
					if (Loader::HasCustomConfig)
					{
						ImGui::TooltipGeneric(langApi->Translate("((000021))"));
					}
				}

				/* Configure Button */
				ImGui::SetCursorPos(ImVec2(initialX, ImGui::GetCursorPosY()));
				if (ImGui::Button(langApi->Translate("((000105))"), ImVec2(btnWidth, 0)))
				{
					this->SetContent(aAddonData);
				}

				static Texture_t* favoriteTex = nullptr;
				static Texture_t* canFavoriteTex = nullptr;

				Texture_t* favTex = aAddonData.NexusAddon->IsFavorite ? favoriteTex : canFavoriteTex;

				if ((aAddonData.NexusAddon->IsFavorite || aAddonData.IsHovered) && favTex)
				{
					float btnSz = ImGui::GetFontSize() * 1.5f;

					ImGui::SetCursorPos(ImVec2(initialX + btnWidth - btnSz, 0));
					ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
					if (ImGui::IconButton(favTex->Resource, ImVec2(btnSz, btnSz)))
					{
						aAddonData.NexusAddon->IsFavorite = !aAddonData.NexusAddon->IsFavorite;
						Loader::SaveAddonConfig();
						this->Invalidate();
						clickedFavorite = true;
					}
					ImGui::PopStyleVar();
					hoveredFavorite = ImGui::IsItemHovered();
				}
				else if (!favTex)
				{
					CTextureLoader* texapi = ctx->GetTextureService();
					favoriteTex = texapi->GetOrCreate("ICON_FAVORITE", RES_ICON_FAVORITE, ctx->GetModule());
					canFavoriteTex = texapi->GetOrCreate("ICON_CANFAVORITE", RES_ICON_CANFAVORITE, ctx->GetModule());
				}
			}
			ImGui::EndChild();

			if (!poppedActionsPad)
			{
				ImGui::PopStyleVar();
			}
		}
		aAddonData.IsHovered = hoveredFavorite || clickedFavorite || ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
		ImGui::EndChild();

		if (isduu && aAddonData.IsHovered)
		{
			ImGui::BeginTooltip();
			ImGui::Text(langApi->Translate("((000103))"));
			ImGui::EndTooltip();
		}

		if (isduu && !poppedCol)
		{
			ImGui::PopStyleColor();
		}

		ImGui::PopStyleVar();
	}
	else if (aAddonData.Type == EAddonType::Library || aAddonData.Type == EAddonType::Arc || aAddonData.Type == EAddonType::LibraryArc)
	{
		if (!aAddonData.LibraryAddon_t) { return; }

		std::string sig = std::to_string(aAddonData.LibraryAddon_t->Signature);

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

		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1);
		if (ImGui::BeginChild(("AddonItem_" + sig).c_str(), itemSz, true))
		{
			static CContext* ctx = CContext::GetContext();
			static CUiContext* uictx = ctx->GetUIContext();
			static CLocalization* langApi = uictx->GetLocalization();
			static CAlerts* alertctx = uictx->GetAlerts();

			if (ImGui::BeginChild("Info", ImVec2(itemSz.x - style.ItemSpacing.x - actionsAreaWidth - (style.WindowPadding.x * 2), innerHeight), false, ImGuiWindowFlags_NoBackground))
			{
				ImGui::Text(aAddonData.LibraryAddon_t->Name.c_str());
				ImGui::TextDisabled("by %s", aAddonData.LibraryAddon_t->Author.c_str());

				/* description */
				ImGui::TextWrapped(aAddonData.LibraryAddon_t->Description.c_str());
			}
			ImGui::EndChild();

			ImGui::SameLine();

			bool poppedActionsPad = false;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			if (ImGui::BeginChild("Actions", ImVec2(actionsAreaWidth, innerHeight), false, ImGuiWindowFlags_NoBackground))
			{
				ImGui::PopStyleVar();
				poppedActionsPad = true;

				float initialX = ImGui::GetCursorPosX();

				static CTextureLoader* textureApi = ctx->GetTextureService();
				static Texture_t* T1 = textureApi->GetOrCreate("ICON_ADDONPOLICY_TIER1", RES_ICON_TIER1, ctx->GetModule());
				static Texture_t* T2 = textureApi->GetOrCreate("ICON_ADDONPOLICY_TIER2", RES_ICON_TIER2, ctx->GetModule());
				static Texture_t* T3 = textureApi->GetOrCreate("ICON_ADDONPOLICY_TIER3", RES_ICON_TIER3, ctx->GetModule());
				static Texture_t* TX = textureApi->GetOrCreate("ICON_ADDONPOLICY_TIER_UNKNOWN", RES_ICON_TIER_UNKNOWN, ctx->GetModule());

				Texture_t* noticeIcon = nullptr;
				std::string noticeURL;
				std::string noticeTT;

				if (aAddonData.LibraryAddon_t->PolicyTier != 0)
				{
					switch (aAddonData.LibraryAddon_t->PolicyTier)
					{
						case -1:
						{
							noticeIcon = TX;
							noticeURL = "https://raidcore.gg/Legal#addon-policy";
							noticeTT = langApi->Translate("((000090))");

							if (!TX)
							{
								TX = textureApi->GetOrCreate("ICON_ADDONPOLICY_TIER_UNKNOWN", RES_ICON_TIER_UNKNOWN, ctx->GetModule());
							}

							break;
						}

						case 1:
						{
							noticeIcon = T1;
							noticeURL = "https://raidcore.gg/Legal#addon-policy";
							noticeTT = String::Format(langApi->Translate("((000089))"), 1);

							if (!T1)
							{
								T1 = textureApi->GetOrCreate("ICON_ADDONPOLICY_TIER1", RES_ICON_TIER1, ctx->GetModule());
							}

							break;
						}
						case 2:
						{
							noticeIcon = T2;
							noticeURL = "https://raidcore.gg/Legal#addon-policy";
							noticeTT = String::Format(langApi->Translate("((000089))"), 2);

							if (!T2)
							{
								T2 = textureApi->GetOrCreate("ICON_ADDONPOLICY_TIER2", RES_ICON_TIER2, ctx->GetModule());
							}

							break;
						}
						case 3:
						{
							noticeIcon = T3;
							noticeURL = "https://raidcore.gg/Legal#addon-policy";
							noticeTT = String::Format(langApi->Translate("((000089))"), 3);

							if (!T3)
							{
								T3 = textureApi->GetOrCreate("ICON_ADDONPOLICY_TIER3", RES_ICON_TIER3, ctx->GetModule());
							}

							break;
						}
					}
				}
				else if (!aAddonData.LibraryAddon_t->ToSComplianceNotice.empty())
				{
					noticeIcon = T1;
					noticeURL = "https://help.guildwars2.com/hc/en-us/articles/360013625034-Policy-Third-Party-Programs";

					std::string tosNotice = langApi->Translate("((000074))");
					tosNotice.append("\n");
					tosNotice.append(aAddonData.LibraryAddon_t->ToSComplianceNotice);

					noticeTT = tosNotice;
				}

				if (noticeIcon)
				{
					float btnSz = ImGui::GetFontSize() * 1.5f;

					ImGui::SetCursorPos(ImVec2(initialX + btnWidth - btnSz, 0));
					ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
					if (ImGui::IconButton(noticeIcon->Resource, ImVec2(btnSz, btnSz)))
					{
						ShellExecuteA(0, 0, noticeURL.c_str(), 0, 0, SW_SHOW);
					}
					ImGui::PopStyleVar();
					ImGui::TooltipGeneric(langApi->Translate(noticeTT.c_str()));
				}

				/* Install Button */
				ImGui::SetCursorPos(ImVec2(initialX, (ImGui::GetWindowHeight() - (btnTextSz.y * 2) - style.ItemSpacing.y - (style.FramePadding.y * 2)) / 2));
				if (aAddonData.Type != EAddonType::Arc)
				{
					if (ImGui::Button(aAddonData.LibraryAddon_t->IsInstalling
						? (langApi->Translate("((000027))") + sig).c_str()
						: (langApi->Translate("((000028))") + sig).c_str(),
						ImVec2(btnWidth, 0)))
					{
						if (!aAddonData.LibraryAddon_t->IsInstalling)
						{
							bool isArcPlugin = aAddonData.Type == EAddonType::LibraryArc;

							std::thread([aAddonData, isArcPlugin]()
							{
								CContext* ctx = CContext::GetContext();
								CUpdater* updater = ctx->GetUpdater();
								updater->InstallAddon(aAddonData.LibraryAddon_t, isArcPlugin);

								if (isArcPlugin)
								{
									ArcDPS::AddToAtlasBySig(aAddonData.LibraryAddon_t->Signature);
								}
								else
								{
									Loader::NotifyChanges();
									//Loader::QueueAddon(ELoaderAction::Reload, installPath);
								}

								CUiContext* uictx = ctx->GetUIContext();
								uictx->Invalidate();

								aAddonData.LibraryAddon_t->IsInstalling = false;
							}).detach();

							//Loader::AddonConfig[aAddon->Signature].IsLoaded = true;
						}
					}
				}
				else
				{
					float width = ImGui::CalcTextSize("via ArcDPS").x;
					ImGui::SetCursorPosX((btnWidth - width) / 2);
					ImGui::Text("via ArcDPS");
				}
				
				/* GitHub Button */
				if (aAddonData.LibraryAddon_t->Provider == EUpdateProvider::GitHub && !aAddonData.LibraryAddon_t->DownloadURL.empty())
				{
					ImGui::SetCursorPos(ImVec2(initialX, ImGui::GetCursorPosY()));
					if (ImGui::Button((langApi->Translate("((000030))") + sig).c_str(), ImVec2(btnWidth, 0)))
					{
						ShellExecuteA(0, 0, aAddonData.LibraryAddon_t->DownloadURL.c_str(), 0, 0, SW_SHOW);
					}
				}
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
			static bool showEnabled = (int)this->Filter & (int)EAddonsFilterFlags::ShowEnabled;
			static bool showDisabled = (int)this->Filter & (int)EAddonsFilterFlags::ShowDisabled;
			static bool showDownloadable = (int)this->Filter & (int)EAddonsFilterFlags::ShowDownloadable;
			static bool showInstalledArc = (int)this->Filter & (int)EAddonsFilterFlags::ShowInstalled_Arc;
			static bool showDownloadableArc = (int)this->Filter & (int)EAddonsFilterFlags::ShowDownloadable_Arc;

			if (!(this->Filter == quickFilter_Installed ||
				this->Filter == quickFilter_Library ||
				this->Filter == quickFilter_ArcPlugins))
			{
				ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_ButtonActive]);
				doPopHighlight = true;
			}

			if (ImGui::ImageButton(filtersTex->Resource, ImVec2(ImGui::GetFontSize(), ImGui::GetFontSize())))
			{
				showEnabled = (int)this->Filter & (int)EAddonsFilterFlags::ShowEnabled;
				showDisabled = (int)this->Filter & (int)EAddonsFilterFlags::ShowDisabled;
				showDownloadable = (int)this->Filter & (int)EAddonsFilterFlags::ShowDownloadable;
				showInstalledArc = (int)this->Filter & (int)EAddonsFilterFlags::ShowInstalled_Arc;
				showDownloadableArc = (int)this->Filter & (int)EAddonsFilterFlags::ShowDownloadable_Arc;
				ImGui::OpenPopup("Filters");
			}

			if (doPopHighlight)
			{
				ImGui::PopStyleColor();
				doPopHighlight = false;
			}

			if (ImGui::BeginPopupContextItem("Filters"))
			{
				if (ImGui::Checkbox(langApi->Translate("((000106))"), &showEnabled))
				{
					if (showEnabled)
					{
						this->Filter = (EAddonsFilterFlags)((int)this->Filter | (int)EAddonsFilterFlags::ShowEnabled);
					}
					else
					{
						this->Filter = (EAddonsFilterFlags)((int)this->Filter & ~(int)EAddonsFilterFlags::ShowEnabled);
					}
					settingsctx->Set(OPT_ADDONFILTERS, this->Filter);
					this->Invalidate();
					this->ClearContent();
				}

				if (ImGui::Checkbox(langApi->Translate("((000107))"), &showDisabled))
				{
					if (showDisabled)
					{
						this->Filter = (EAddonsFilterFlags)((int)this->Filter | (int)EAddonsFilterFlags::ShowDisabled);
					}
					else
					{
						this->Filter = (EAddonsFilterFlags)((int)this->Filter & ~(int)EAddonsFilterFlags::ShowDisabled);
					}
					settingsctx->Set(OPT_ADDONFILTERS, this->Filter);
					this->Invalidate();
					this->ClearContent();
				}

				if (ImGui::Checkbox(langApi->Translate("((000108))"), &showDownloadable))
				{
					if (showDownloadable)
					{
						this->Filter = (EAddonsFilterFlags)((int)this->Filter | (int)EAddonsFilterFlags::ShowDownloadable);
					}
					else
					{
						this->Filter = (EAddonsFilterFlags)((int)this->Filter & ~(int)EAddonsFilterFlags::ShowDownloadable);
					}
					settingsctx->Set(OPT_ADDONFILTERS, this->Filter);
					this->Invalidate();
					this->ClearContent();
				}

				if (ArcDPS::IsLoaded)
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
				}

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
		
		if (this->Filter == quickFilter_Installed)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_ButtonActive]);
			doPopHighlight = true;
		}

		if (ImGui::Button(langApi->Translate("((000031))")))
		{
			this->Filter = quickFilter_Installed;
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

		if (this->Filter == quickFilter_Library)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_ButtonActive]);
			doPopHighlight = true;
		}

		if (ImGui::Button(langApi->Translate("((000032))")))
		{
			this->Filter = quickFilter_Library;
			settingsctx->Set(OPT_ADDONFILTERS, this->Filter);
			this->Invalidate();
			this->ClearContent();
		}

		if (doPopHighlight)
		{
			ImGui::PopStyleColor();
			doPopHighlight = false;
		}

		if (ArcDPS::IsLoaded)
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
		}

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

				for (AddonItemData_t& addon : this->Addons)
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
			if (checkedForUpdates == -1)
			{
				const std::lock_guard<std::mutex> lock(Loader::Mutex);
				{
					checkedForUpdates = 0;
					queuedForCheck = 0;
					updatedCount = 0;

					/* pre-iterate to get the count of how many need to be checked, else one call might finish before the count can be incremented */
					for (auto addon : Loader::Addons)
					{
						if (nullptr == addon->Definitions) { continue; }
						queuedForCheck++;
					}

					if (queuedForCheck == 0)
					{
						checkedForUpdates = -1;
					}

					for (auto addon : Loader::Addons)
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
								Loader::QueueAddon(ELoaderAction::Reload, tmpPath);

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
			}
		}
		ImGui::TooltipGeneric(langApi->Translate("((000036))"));

		static Texture_t* refreshTex = nullptr;
		if (refreshTex)
		{
			ImGui::SameLine();

			if (ImGui::ImageButton(refreshTex->Resource, ImVec2(ImGui::GetFontSize(), ImGui::GetFontSize())))
			{
				Loader::NotifyChanges();
				std::thread([this]() {
					Loader::Library::Fetch();
					this->Invalidate();
				}).detach();
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
	AddonItemData_t addonData = this->AddonData;

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

		std::string sig = std::to_string(addonData.NexusAddon->Definitions->Signature);
		std::string sigid = "##" + sig;

		ImGuiStyle& style = ImGui::GetStyle();

		float btnSz = ImGui::GetFontSize() * 1.5f;

		// helper variable in case we unload or do anything else that might modify the callback before ->Invalidate() is invoked
		bool skipOptions = false;

		std::string headerStr = langApi->Translate("((000099))");
		headerStr.append(" ");
		headerStr.append(addonData.NexusAddon->Definitions->Name);

		if (ImGui::CollapsingHeader(headerStr.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (addonData.NexusAddon->Definitions->Provider != EUpdateProvider::Self)
			{
				if (ImGui::Button((!addonData.NexusAddon->IsCheckingForUpdates
					? langApi->Translate("((000035))")
					: langApi->Translate("((000071))") + sigid).c_str()))
				{
					if (!addonData.NexusAddon->IsCheckingForUpdates)
					{
						for (auto addon : Loader::Addons)
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

									if (updater->UpdateAddon(tmpPath, addonInfo, false, 5 * 60))
									{
										Loader::QueueAddon(ELoaderAction::Reload, tmpPath);

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
			
			if (addonData.NexusAddon->Definitions->Provider == EUpdateProvider::GitHub && addonData.NexusAddon->Definitions->UpdateLink)
			{
				ImGui::SameLine();
				if (ImGui::Button((langApi->Translate("((000030))") + sig).c_str()))
				{
					ShellExecuteA(0, 0, addonData.NexusAddon->Definitions->UpdateLink, 0, 0, SW_SHOW);
				}
			}

			if (ImGui::Checkbox((langApi->Translate("((000014))") + sigid).c_str(), &addonData.NexusAddon->IsPausingUpdates))
			{
				Loader::SaveAddonConfig();
			}

			if (ImGui::Checkbox((langApi->Translate("((000016))") + sigid).c_str(), &addonData.NexusAddon->IsDisabledUntilUpdate))
			{
				//Logger->Debug(CH_GUI, "ToggleDUU called: %s", it.second->Definitions->Name);

				Loader::SaveAddonConfig();
				this->Invalidate();

				if (addonData.NexusAddon->State == EAddonState::Loaded)
				{
					Loader::QueueAddon(ELoaderAction::Unload, addonData.NexusAddon->Path);
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
					Loader::SaveAddonConfig();
				}
			}

			/* Load/Unload Button */
			// just check if loaded, if it was not hot-reloadable it would be EAddonState::LoadedLOCKED
			if (addonData.NexusAddon->State == EAddonState::Loaded)
			{
				if (ImGui::Button((addonData.NexusAddon->IsWaitingForUnload ? langApi->Translate("((000078))") : langApi->Translate("((000020))") + sig).c_str()))
				{
					if (!addonData.NexusAddon->IsWaitingForUnload)
					{
						//Logger->Debug(CH_GUI, "Unload called: %s", it.second->Definitions->Name);
						Loader::QueueAddon(ELoaderAction::Unload, addonData.NexusAddon->Path);
					}
				}
				if (Loader::HasCustomConfig)
				{
					ImGui::TooltipGeneric(langApi->Translate("((000021))"));
				}
			}
			else if (addonData.NexusAddon->State == EAddonState::LoadedLOCKED)
			{
				std::string additionalInfo;

				if (Loader::HasCustomConfig)
				{
					additionalInfo.append("\n");
					additionalInfo.append(langApi->Translate("((000021))"));
				}

				if (ImGui::Button((langApi->Translate(addonData.NexusAddon->IsFlaggedForDisable ? "((000024))" : "((000022))") + sig).c_str()))
				{
					addonData.NexusAddon->IsFlaggedForDisable = !addonData.NexusAddon->IsFlaggedForDisable;
					Loader::SaveAddonConfig();
				}
				ImGui::TooltipGeneric(langApi->Translate(addonData.NexusAddon->IsFlaggedForDisable ? "((000025))" : "((000023))"), additionalInfo.c_str());
			}
			else if (addonData.NexusAddon->State == EAddonState::NotLoaded && (addonData.NexusAddon->Definitions->HasFlag(EAddonFlags::OnlyLoadDuringGameLaunchSequence) || addonData.NexusAddon->Definitions->Signature == 0xFFF694D1) && !IsGameLaunchSequence)
			{
				/* if it's too late to load this addon */
				if (ImGui::Button((langApi->Translate(addonData.NexusAddon->IsFlaggedForEnable ? "((000020))" : "((000024))") + sig).c_str()))
				{
					addonData.NexusAddon->IsFlaggedForEnable = !addonData.NexusAddon->IsFlaggedForEnable;

					if (addonData.NexusAddon->IsFlaggedForEnable)
					{
						addonData.NexusAddon->IsDisabledUntilUpdate = false; // explicitly loaded
						ctx->GetUIContext()->GetAlerts()->Notify(EAlertType::Info, String::Format("%s %s", addonData.NexusAddon->Definitions->Name, langApi->Translate("((000080))")).c_str());
					}

					Loader::SaveAddonConfig();
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
					Loader::QueueAddon(ELoaderAction::Load, addonData.NexusAddon->Path);
				}
				if (Loader::HasCustomConfig)
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
		}

		if (!(addonData.NexusAddon->State == EAddonState::Loaded || addonData.NexusAddon->State == EAddonState::LoadedLOCKED))
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
		}
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

	const std::lock_guard<std::mutex> lock(Loader::Mutex);

	CContext* ctx = CContext::GetContext();
	CUiContext* uictx = ctx->GetUIContext();
	CSettings* settingsctx = ctx->GetSettingsCtx();

	this->Filter = settingsctx->Get<EAddonsFilterFlags>(OPT_ADDONFILTERS, quickFilter_Installed);

	for (Addon_t* addon : Loader::Addons)
	{
		if (addon->Path.filename() == "arcdps_integration64.dll") { continue; }
		if (!addon->Definitions) { continue; }

		if (!this->SearchTerm.empty() &&
			!(String::Contains(String::ToLower(addon->Definitions->Name), this->SearchTerm) ||
			String::Contains(String::ToLower(addon->Definitions->Description), this->SearchTerm)))
		{
			continue;
		}

		if ((addon->State == EAddonState::Loaded || addon->State == EAddonState::LoadedLOCKED) &&
			((int)this->Filter & (int)EAddonsFilterFlags::ShowEnabled) == 0)
		{
			continue;
		}

		if ((addon->State == EAddonState::NotLoaded || addon->State == EAddonState::NotLoadedDuplicate ||
			addon->State == EAddonState::NotLoadedIncompatible || addon->State == EAddonState::NotLoadedIncompatibleAPI) &&
			((int)this->Filter & (int)EAddonsFilterFlags::ShowDisabled) == 0)
		{
			continue;
		}

		AddonItemData_t addonItem{};
		addonItem.Type = EAddonType::Nexus;
		addonItem.NexusAddon = addon;

		for (GUI_RENDER renderCb : uictx->GetOptionsCallbacks())
		{
			std::string parent = Loader::GetOwner(renderCb);
			if (addon->Definitions && addon->Definitions->Name == parent)
			{
				addonItem.OptionsRender = renderCb;
				break;
			}
		}

		this->Addons.push_back(addonItem);

		if (this->HasContent && this->AddonData.NexusAddon->Definitions && 
			this->AddonData.NexusAddon->Definitions->Signature == addon->Definitions->Signature)
		{
			this->SetContent(addonItem);
		}
	}

	{
		const std::lock_guard<std::mutex> lock(Loader::Library::Mutex);

		for (LibraryAddon_t* addon : Loader::Library::Addons)
		{
			if (!this->SearchTerm.empty() && !String::Contains(String::ToLower(addon->Name), this->SearchTerm)) { continue; }

			bool installed = false;

			for (Addon_t* installedAddon : Loader::Addons)
			{
				// filter out already installed
				if (addon->Signature == installedAddon->MatchSignature && installedAddon->State != EAddonState::None)
				{
					installed = true;
					break;
				}
			}

			if (installed)
			{
				continue;
			}

			if (((int)this->Filter & (int)EAddonsFilterFlags::ShowDownloadable) == 0)
			{
				continue;
			}

			AddonItemData_t addonItem{};
			addonItem.Type = EAddonType::Library;
			addonItem.LibraryAddon_t = addon;

			if (ArcDPS::IsLoaded && addonItem.LibraryAddon_t->Signature == 0xFFF694D1) { continue; }

			this->Addons.push_back(addonItem);
		}
	}
	
	{
		const std::lock_guard<std::mutex> lockLoader(ArcDPS::Mutex);

		for (LibraryAddon_t* addon : ArcDPS::PluginLibrary)
		{
			if (!this->SearchTerm.empty() && !String::Contains(String::ToLower(addon->Name), this->SearchTerm)) { continue; }

			bool installed = false;

			for (int& arcAddonSig : ArcDPS::Plugins)
			{
				// if arclibAddon already exist in installed addons
				// or if arcdps is loaded another way and the arclibAddon is arc
				if (addon->Signature == arcAddonSig)
				{
					installed = true;
					break;
				}
			}

			if (installed && ((int)this->Filter & (int)EAddonsFilterFlags::ShowInstalled_Arc) == 0)
			{
				continue;
			}

			if (!installed && ((int)this->Filter & (int)EAddonsFilterFlags::ShowDownloadable_Arc) == 0)
			{
				continue;
			}

			AddonItemData_t addonItem{};
			addonItem.Type = installed ? EAddonType::Arc : EAddonType::LibraryArc;
			addonItem.LibraryAddon_t = addon;

			this->Addons.push_back(addonItem);
		}
	}

	std::sort(this->Addons.begin(), this->Addons.end(), [](AddonItemData_t& lhs, AddonItemData_t& rhs)
	{
		std::string lcmp;
		std::string rcmp;

		if (lhs.Type == EAddonType::Nexus)
		{
			lcmp = lhs.NexusAddon->Definitions && lhs.NexusAddon->Definitions->Name
				? String::ToLower(String::Normalize(lhs.NexusAddon->Definitions->Name))
				: String::ToLower(lhs.NexusAddon->Path.filename().string());
		}
		else if (lhs.Type == EAddonType::Library || lhs.Type == EAddonType::Arc || lhs.Type == EAddonType::LibraryArc)
		{
			lcmp = String::ToLower(String::Normalize(lhs.LibraryAddon_t->Name));
		}

		if (rhs.Type == EAddonType::Nexus)
		{
			rcmp = rhs.NexusAddon->Definitions && rhs.NexusAddon->Definitions->Name
				? String::ToLower(String::Normalize(rhs.NexusAddon->Definitions->Name))
				: String::ToLower(rhs.NexusAddon->Path.filename().string());
		}
		else if (rhs.Type == EAddonType::Library || rhs.Type == EAddonType::Arc || rhs.Type == EAddonType::LibraryArc)
		{
			rcmp = String::ToLower(String::Normalize(rhs.LibraryAddon_t->Name));
		}

		return
			(lhs.Type <= rhs.Type) &&
			(lhs.NexusAddon->IsFavorite > rhs.NexusAddon->IsFavorite) ||
			((lhs.NexusAddon->IsFavorite == rhs.NexusAddon->IsFavorite) && (lhs.NexusAddon->IsDisabledUntilUpdate > rhs.NexusAddon->IsDisabledUntilUpdate)) ||
			((lhs.NexusAddon->IsFavorite == rhs.NexusAddon->IsFavorite) && (lhs.NexusAddon->IsDisabledUntilUpdate == rhs.NexusAddon->IsDisabledUntilUpdate) && lcmp < rcmp);
	});
}
