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

#include "Context.h"
#include "Renderer.h"
#include "resource.h"
#include "Shared.h"
#include "Util/Strings.h"

#include "Index.h"
#include "Loader/ArcDPS.h"
#include "Loader/Library.h"
#include "Loader/Loader.h"

constexpr ImGuiWindowFlags ModalFlags = ImGuiWindowFlags_AlwaysAutoResize |
										ImGuiWindowFlags_NoResize |
										ImGuiWindowFlags_NoCollapse;

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
	if (this->HasContent && this->AddonData.NexusAddon->Definitions && this->AddonData.NexusAddon->Definitions->Signature == aAddonID)
	{
		this->HasContent = false;
	}
}

void CAddonsWindow::SetContent(AddonItemData& aAddonData)
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

void CAddonsWindow::AddonItem(AddonItemData& aAddonData, float aWidth)
{
	if (!aAddonData.NexusAddon || !aAddonData.NexusAddon->Definitions) { return; }

	std::string sig = std::to_string(aAddonData.NexusAddon->Definitions->Signature);
	
	ImGuiStyle& style = ImGui::GetStyle();

	ImVec2 itemSz = ImVec2(aWidth, ImGui::GetTextLineHeightWithSpacing() * 5);
	float innerHeight = itemSz.y - (style.WindowPadding.y * 2);

	ImVec2 btnTextSz = ImGui::CalcTextSize("############");

	float btnWidth = btnTextSz.x + (style.FramePadding.x * 2);
	float actionsAreaWidth = btnWidth + (style.WindowPadding.x * 2);

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

	static CContext* ctx = CContext::GetContext();
	static CLocalization* langApi = ctx->GetLocalization();

	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1);
	if (ImGui::BeginChild(("##AddonItem_" + sig).c_str(), itemSz, true))
	{
		if (isduu)
		{
			ImGui::PopStyleColor();
			poppedCol = true;
		}

		static CUiContext* uictx = ctx->GetUIContext();
		static CAlerts* alertctx = uictx->GetAlerts();

		if (ImGui::BeginChild("##AddonItem_Info_", ImVec2(itemSz.x - style.ItemSpacing.x - actionsAreaWidth, innerHeight)))
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

		if (ImGui::BeginChild("##AddonItem_Actions_", ImVec2(actionsAreaWidth, innerHeight)))
		{
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
						alertctx->Notify(String::Format("%s %s", aAddonData.NexusAddon->Definitions->Name, langApi->Translate("((000080))")).c_str());
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
			if (ImGui::Button("Configure", ImVec2(btnWidth, 0)))
			{
				this->SetContent(aAddonData);
			}

			static Texture* favoriteTex = nullptr;
			static Texture* canFavoriteTex = nullptr;

			Texture* favTex = aAddonData.NexusAddon->IsFavorite ? favoriteTex : canFavoriteTex;

			if ((aAddonData.NexusAddon->IsFavorite || aAddonData.IsHovered) && favTex)
			{
				float btnSz = ImGui::GetFontSize() * 1.5f;

				ImGui::SetCursorPos(ImVec2(initialX + btnWidth - btnSz, 0));
				if (ImGui::IconButton(favTex->Resource, ImVec2(btnSz, btnSz)))
				{
					aAddonData.NexusAddon->IsFavorite = !aAddonData.NexusAddon->IsFavorite;
					Loader::SaveAddonConfig();
					Loader::SortAddons(); // quite ugly to call from here, but oh well
					this->Invalidate();
					clickedFavorite = true;
				}
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
void CAddonsWindow::AddonItem(LibraryAddon* aAddon, float aWidth, bool aInstalled, bool aIsArcPlugin)
{
	if (!aAddon) { return; }

	std::string sig = std::to_string(aAddon->Signature);

	ImGuiStyle& style = ImGui::GetStyle();

	ImVec2 itemSz = ImVec2(aWidth, ImGui::GetTextLineHeightWithSpacing() * 5);
	float innerHeight = itemSz.y - (style.WindowPadding.y * 2);

	ImVec2 btnTextSz = ImGui::CalcTextSize("############");

	float btnWidth = btnTextSz.x + (style.FramePadding.x * 2);
	float actionsAreaWidth = btnWidth + (style.WindowPadding.x * 2);

	ImVec2 curPos = ImGui::GetCursorPos();

	/* above visible space                        || under visible space */
	if (curPos.y < ImGui::GetScrollY() - itemSz.y || curPos.y > ImGui::GetScrollY() + ImGui::GetWindowHeight())
	{
		ImGui::Dummy(itemSz);
		return;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1);
	if (ImGui::BeginChild(("##AddonItem_" + sig).c_str(), itemSz, true))
	{
		static CContext* ctx = CContext::GetContext();
		static CLocalization* langApi = ctx->GetLocalization();
		static CUiContext* uictx = ctx->GetUIContext();
		static CAlerts* alertctx = uictx->GetAlerts();

		if (ImGui::BeginChild("##AddonItem_Info_", ImVec2(itemSz.x - style.ItemSpacing.x - actionsAreaWidth, innerHeight)))
		{
			ImGui::Text(aAddon->Name.c_str());
			ImGui::TextDisabled("by %s", aAddon->Author.c_str());

			/* description */
			ImGui::TextWrapped(aAddon->Description.c_str());
		}
		ImGui::EndChild();
		
		ImGui::SameLine();

		if (ImGui::BeginChild("##AddonItem_Actions_", ImVec2(actionsAreaWidth, innerHeight)))
		{
			float initialX = ImGui::GetCursorPosX();

			static CTextureLoader* textureApi = ctx->GetTextureService();
			static Texture* T1 = textureApi->GetOrCreate("ICON_ADDONPOLICY_TIER1", RES_ICON_TIER1, ctx->GetModule());
			static Texture* T2 = textureApi->GetOrCreate("ICON_ADDONPOLICY_TIER2", RES_ICON_TIER2, ctx->GetModule());
			static Texture* T3 = textureApi->GetOrCreate("ICON_ADDONPOLICY_TIER3", RES_ICON_TIER3, ctx->GetModule());
			static Texture* TX = textureApi->GetOrCreate("ICON_ADDONPOLICY_TIER_UNKNOWN", RES_ICON_TIER_UNKNOWN, ctx->GetModule());

			Texture* noticeIcon = nullptr;
			std::string noticeURL;
			std::string noticeTT;

			if (aAddon->PolicyTier != 0)
			{
				switch (aAddon->PolicyTier)
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
			else if (!aAddon->ToSComplianceNotice.empty())
			{
				noticeIcon = T1;
				noticeURL = "https://help.guildwars2.com/hc/en-us/articles/360013625034-Policy-Third-Party-Programs";

				std::string tosNotice = langApi->Translate("((000074))");
				tosNotice.append("\n");
				tosNotice.append(aAddon->ToSComplianceNotice);

				noticeTT = tosNotice;
			}

			if (noticeIcon)
			{
				float btnSz = ImGui::GetFontSize() * 1.5f;

				ImGui::SetCursorPos(ImVec2(initialX + btnWidth - btnSz, 0));
				if (ImGui::IconButton(noticeIcon->Resource, ImVec2(btnSz, btnSz)))
				{
					ShellExecuteA(0, 0, noticeURL.c_str(), 0, 0, SW_SHOW);
				}

				ImGui::TooltipGeneric(langApi->Translate(noticeTT.c_str()));
			}

			/* Install Button */
			ImGui::SetCursorPos(ImVec2(initialX, (ImGui::GetWindowHeight() - (btnTextSz.y * 2) - style.ItemSpacing.y - (style.FramePadding.y * 2)) / 2));
			// just check if loaded, if it was not hot-reloadable it would be EAddonState::LoadedLOCKED
			if (!aInstalled)
			{
				if (ImGui::Button(aAddon->IsInstalling
					? (langApi->Translate("((000027))") + sig).c_str()
					: (langApi->Translate("((000028))") + sig).c_str(),
					ImVec2(btnWidth, 0)))
				{
					if (!aAddon->IsInstalling)
					{
						std::thread([aAddon, aIsArcPlugin]()
						{
							CContext* ctx = CContext::GetContext();
							CUpdater* updater = ctx->GetUpdater();
							updater->InstallAddon(aAddon, aIsArcPlugin);

							if (aIsArcPlugin)
							{
								ArcDPS::AddToAtlasBySig(aAddon->Signature);
							}
							else
							{
								Loader::NotifyChanges();
								//Loader::QueueAddon(ELoaderAction::Reload, installPath);
							}

							CUiContext* uictx = ctx->GetUIContext();
							uictx->Invalidate();

							aAddon->IsInstalling = false;
						}).detach();

						//Loader::AddonConfig[aAddon->Signature].IsLoaded = true;
					}
				}
			}
			else
			{
				ImGui::Text(langApi->Translate("((000029))"));
			}

			/* GitHub Button */
			if (aAddon->Provider == EUpdateProvider::GitHub && !aAddon->DownloadURL.empty())
			{
				ImGui::SetCursorPos(ImVec2(initialX, ImGui::GetCursorPosY()));
				if (ImGui::Button((langApi->Translate("((000030))") + sig).c_str(), ImVec2(btnWidth, 0)))
				{
					ShellExecuteA(0, 0, aAddon->DownloadURL.c_str(), 0, 0, SW_SHOW);
				}
			}
		}
		ImGui::EndChild();
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
		this->PopulateLibrary();

		this->IsInvalid = false;
	}

	static char filter[400] = {};
	static int quickFilterMode = 0; // 0 = nexus, 1 = library, 2 = arcdps // FIXME: merge it all into one, do actual filtering
	static bool showInstalled = false;

	static CContext* ctx = CContext::GetContext();
	static CLocalization* langApi = ctx->GetLocalization();
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

	static bool isListMode = settingsctx->Get<bool>(OPT_ISLISTMODE);

	if (ImGui::BeginChild("##Addons_Filters", filterAreaSz))
	{
		/* quick filters */
		if (ImGui::Button(langApi->Translate("((000031))")))
		{
			quickFilterMode = 0;
			this->ClearContent();
		}

		ImGui::SameLine();

		if (ImGui::Button(langApi->Translate("((000032))")))
		{
			quickFilterMode = 1;
			this->ClearContent();
		}

		if (ArcDPS::IsLoaded)
		{
			ImGui::SameLine();

			std::string legacyNotice = langApi->Translate("((000076))");
			legacyNotice.append("\n");
			legacyNotice.append(langApi->Translate("((000077))"));

			if (ImGui::Button(langApi->Translate("((000075))")))
			{
				quickFilterMode = 2;
				this->ClearContent();
			}
			ImGui::TooltipGeneric(legacyNotice.c_str());
		}

		/* filter search */
		ImGui::Text("Search");
		ImGui::SameLine();
		if (ImGui::InputText("##addonsfilter", &filter[0], 400))
		{
			this->Filter = String::ToLower(filter);
			this->Invalidate();
		}

		/* clear filter */
		static Texture* clearFilterTex = nullptr;
		if (clearFilterTex)
		{
			ImGui::SameLine();
			if (ImGui::ImageButton(clearFilterTex->Resource, ImVec2(ImGui::GetFontSize(), ImGui::GetFontSize())))
			{
				memset(filter, 0, 400);
				this->Filter = String::ToLower(filter);
				this->Invalidate();
			}
		}
		else
		{
			CTextureLoader* texapi = ctx->GetTextureService();
			clearFilterTex = texapi->GetOrCreate("ICON_CLOSE", RES_ICON_CLOSE, ctx->GetModule());
		}

		/* view mode */
		static Texture* viewModeTex = nullptr;
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

		filterAreaEndY = ImGui::GetCursorPos().y;
	}
	ImGui::EndChild();
	
	float addonItemWidth = isListMode
		? (region.x - style.ItemSpacing.x - style.ScrollbarSize)
		: ((region.x - (style.ItemSpacing.x * 2) - style.ScrollbarSize) / 2);

	/* list */
	//ImVec2 listP1 = ImGui::GetWindowPos();
	//ImVec2 listP2 = ImVec2(listP1.x + listAreaSz.x - detailsAreaWidth, listP1.y + listAreaSz.y + filterAreaSz.y + style.ItemSpacing.y);

	//ImVec2 posList = ImGui::GetCursorPos();

	//ImGui::PushClipRect(listP1, listP2, true);
	if (ImGui::BeginChild("##Addons_List", listAreaSz))
	{
		if (configuring)
		{
			this->RenderDetails();
		}
		else
		{
			if (quickFilterMode == 0)
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

					for (AddonItemData& addon : this->Addons)
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
			else if (quickFilterMode == 1)
			{
				ImGui::Checkbox(langApi->Translate("((000038))"), &showInstalled);

				int downloadable = 0;

				const std::lock_guard<std::mutex> lockLoader(Loader::Mutex);
				if (this->Library.size() != 0)
				{
					int i = 0;

					for (LibraryAddon* libAddon : this->Library)
					{
						bool exists = false;
						{
							for (Addon* addon : Loader::Addons)
							{
								// if libAddon already exist in installed addons
								// or if arcdps is loaded another way and the libAddon is arc
								if ((addon->Definitions != nullptr && addon->Definitions->Signature == libAddon->Signature) ||
									(ArcDPS::IsLoaded && libAddon->Signature == 0xFFF694D1))
								{
									exists = true;
									break;
								}
							}
						}

						if (!exists || showInstalled)
						{
							if (!isListMode && i % 2 == 1)
							{
								ImGui::SameLine();
							}

							AddonItem(libAddon, addonItemWidth, exists);
							i++;
							downloadable++;
						}
					}
				}

				if (this->Library.size() == 0 || downloadable == 0)
				{
					ImVec2 windowSize = ImGui::GetWindowSize();
					ImVec2 textSize = ImGui::CalcTextSize(langApi->Translate("((000098))"));
					ImVec2 position = ImGui::GetCursorPos();
					ImGui::SetCursorPos(ImVec2((position.x + (windowSize.x - textSize.x)) / 2, (position.y + (windowSize.y - textSize.y)) / 2));
					ImGui::TextDisabled(langApi->Translate("((000098))"));
				}
			}
			else if (quickFilterMode == 2)
			{
				ImGui::Checkbox(langApi->Translate("((000038))"), &showInstalled);

				if (ArcDPS::IsPluginAtlasBuilt)
				{
					int downloadable = 0;
					const std::lock_guard<std::mutex> lockLoader(ArcDPS::Mutex);
					if (this->ArcLibrary.size() != 0)
					{
						int i = 0;

						for (LibraryAddon* arclibAddon : this->ArcLibrary)
						{
							bool exists = false;
							{
								for (int& arcAddonSig : ArcDPS::Plugins)
								{
									// if arclibAddon already exist in installed addons
									// or if arcdps is loaded another way and the arclibAddon is arc
									if (arclibAddon->Signature == arcAddonSig)
									{
										exists = true;
										break;
									}
								}
							}
							if (!exists || true == showInstalled)
							{
								if (!isListMode && i % 2 == 1)
								{
									ImGui::SameLine();
								}

								AddonItem(arclibAddon, addonItemWidth, exists, true);
								i++;
								downloadable++;
							}
						}
					}

					if (this->ArcLibrary.size() == 0 || downloadable == 0)
					{
						ImVec2 windowSize = ImGui::GetWindowSize();
						ImVec2 textSize = ImGui::CalcTextSize(langApi->Translate("((000098))"));
						ImVec2 position = ImGui::GetCursorPos();
						ImGui::SetCursorPos(ImVec2((position.x + (windowSize.x - textSize.x)) / 2, (position.y + (windowSize.y - textSize.y)) / 2));
						ImGui::TextDisabled(langApi->Translate("((000098))"));
					}
				}
				else
				{
					ImVec2 windowSize = ImGui::GetWindowSize();
					ImVec2 textSize = ImGui::CalcTextSize(langApi->Translate("((000098))"));
					ImVec2 position = ImGui::GetCursorPos();
					ImGui::SetCursorPos(ImVec2((position.x + (windowSize.x - textSize.x)) / 2, (position.y + (windowSize.y - textSize.y)) / 2));
					ImGui::TextDisabled(langApi->Translate("((000098))"));
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
	if (ImGui::BeginChild("##Addons_Details", detailsAreaSz, true))
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
	

	if (ImGui::BeginChild("##Addons_Actions", actionsAreaSz))
	{
		if (ImGui::Button(langApi->Translate("((000034))")))
		{
			std::string strAddons = Index::D_GW2_ADDONS.string();
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
							AddonInfo addonInfo
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
							CLocalization* langApi = ctx->GetLocalization();
							CUpdater* updater = ctx->GetUpdater();
							CUiContext* uictx = ctx->GetUIContext();
							CAlerts* alertctx = uictx->GetAlerts();

							if (updater->UpdateAddon(tmpPath, addonInfo, false, 5 * 60))
							{
								Loader::QueueAddon(ELoaderAction::Reload, tmpPath);

								alertctx->Notify(
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
									alertctx->Notify(langApi->Translate("((000087))"));
								}
							}
						}).detach();
					}
				}
			}
		}
		ImGui::TooltipGeneric(langApi->Translate("((000036))"));

		actionsAreaEndY = ImGui::GetCursorPos().y;
	}
	ImGui::EndChild();
}

void CAddonsWindow::RenderSubWindows()
{
	this->DrawBindSetterModal();
}

void CAddonsWindow::RenderDetails()
{
	AddonItemData addonData = this->AddonData;

	ImGuiStyle& style = ImGui::GetStyle();

	float btnSz = ImGui::GetFontSize() * 1.5f;

	static Texture* chevronRt = nullptr;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	if (ImGui::BeginChild("##Addons_Details_Collapse", ImVec2(btnSz, 0)))
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

	if (ImGui::BeginChild("##Addons_Details_Content"))
	{
		static CContext* ctx = CContext::GetContext();
		static CLocalization* langApi = ctx->GetLocalization();

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

							Addon* addon = addonData.NexusAddon;

							std::filesystem::path tmpPath = addon->Path.string();
							std::thread([addon, tmpPath]()
							{
								AddonInfo addonInfo
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
								CLocalization* langApi = ctx->GetLocalization();

								if (updater->UpdateAddon(tmpPath, addonInfo, false, 5 * 60))
								{
									Loader::QueueAddon(ELoaderAction::Reload, tmpPath);

									alertctx->Notify(
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
									alertctx->Notify(String::Format("%s %s",
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
				Loader::SortAddons(); // quite ugly to call from here, but oh well
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

			if (ImGui::Button((langApi->Translate("((000018))") + sigid).c_str()))
			{
				//Logger->Debug(CH_GUI, "Uninstall called: %s", it.second->Definitions->Name);
				Loader::QueueAddon(ELoaderAction::Uninstall, addonData.NexusAddon->Path);
				skipOptions = true;
				this->ClearContent();
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

void CAddonsWindow::RenderInputBindsTable(const std::map<std::string, InputBindPacked>& aInputBinds)
{
	if (ImGui::BeginTable("table_inputbinds_addons", 2, ImGuiTableFlags_BordersInnerH))
	{
		CContext* ctx = CContext::GetContext();
		CLocalization* langApi = ctx->GetLocalization();

		for (auto& [identifier, inputBind] : aInputBinds)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(langApi->Translate(identifier.c_str()));

			ImGui::TableSetColumnIndex(1);
			ImGui::PushID(identifier.c_str());
			if (ImGui::Button(inputBind.KeysText.c_str(), ImVec2(ImGui::CalcTextSize("XXXXXXXXXXXXXXXXXXXXXXXX").x, 0.0f)))
			{
				this->Editing_Identifier = identifier;
				this->Editing_BindText = inputBind.KeysText;

				this->OpenModalNextFrame = true;
				this->IsEditing = EBindEditType::Nexus;

				this->ModalTitle = langApi->Translate("((000062))");
				this->ModalTitle.append(langApi->Translate(this->Editing_Identifier.c_str()));
				this->ModalTitle.append("##InputBindSetter_Addons");
			}
			ImGui::PopID();
		}

		ImGui::EndTable();
	}
}

void CAddonsWindow::DrawBindSetterModal()
{
	if (this->OpenModalNextFrame == true)
	{
		ImGui::OpenPopup(this->ModalTitle.c_str());
		this->OpenModalNextFrame = false;
	}

	ImVec2 center(Renderer::Width * 0.5f, Renderer::Height * 0.5f);
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal(this->ModalTitle.c_str(), NULL, ModalFlags))
	{
		CContext* ctx = CContext::GetContext();
		CLocalization* langApi = ctx->GetLocalization();
		CInputBindApi* inputBindApi = ctx->GetInputBindApi();
		CGameBindsApi* gameBindsApi = ctx->GetGameBindsApi();

		inputBindApi->StartCapturing();

		InputBind currInputBind = inputBindApi->GetCapturedInputBind();
		std::string usedBy = inputBindApi->IsInUse(currInputBind);

		if (currInputBind == InputBind{})
		{
			ImGui::Text(this->Editing_BindText.c_str());
		}
		else
		{
			ImGui::Text(CInputBindApi::IBToString(currInputBind, true).c_str());
		}

		bool overwriting = false;

		if (usedBy != this->Editing_Identifier && !usedBy.empty())
		{
			ImGui::TextColored(ImVec4(255, 0, 0, 255), (langApi->Translate("((000063))") + usedBy + ".").c_str());
			overwriting = true;
		}

		bool close = false;

		if (ImGui::Button(langApi->Translate("((000064))")))
		{
			if (this->IsEditing == EBindEditType::Nexus)
			{
				inputBindApi->Set(this->Editing_Identifier, InputBind{});
			}
			else if (this->IsEditing == EBindEditType::Game)
			{
				gameBindsApi->Set(this->Editing_GameIdentifier, InputBind{});
			}

			this->Invalidate();

			close = true;
		}
		ImGui::SameLine();
		if (ImGui::Button(langApi->Translate("((000065))")))
		{
			if (this->IsEditing == EBindEditType::Nexus)
			{
				if (overwriting)
				{
					/* unset the bind that's currently using this wombo combo */
					inputBindApi->Set(usedBy, InputBind{});
				}

				inputBindApi->Set(this->Editing_Identifier, currInputBind);
			}
			else if (this->IsEditing == EBindEditType::Game)
			{
				gameBindsApi->Set(this->Editing_GameIdentifier, currInputBind);
			}

			this->Invalidate();

			close = true;
		}
		ImGui::SameLine();
		if (ImGui::Button(langApi->Translate("((000066))")))
		{
			close = true;
		}

		if (close)
		{
			/* unset all editing vars */
			this->Editing_Identifier = "";
			this->Editing_GameIdentifier = (EGameBinds)-1;
			this->Editing_BindText = "";
			this->IsEditing = EBindEditType::None;

			inputBindApi->EndCapturing();
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void CAddonsWindow::PopulateAddons()
{
	this->Addons.clear();

	const std::lock_guard<std::mutex> lock(Loader::Mutex);

	CContext* ctx = CContext::GetContext();
	CUiContext* uictx = ctx->GetUIContext();

	for (Addon* addon : Loader::Addons)
	{
		if (addon->Path.filename() == "arcdps_integration64.dll") { continue; }
		if (!addon->Definitions) { continue; }

		if (!this->Filter.empty() && !String::Contains(String::ToLower(addon->Definitions->Name), this->Filter)) { continue; }

		AddonItemData addonItem{};
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
}

void CAddonsWindow::PopulateLibrary()
{
	this->Library.clear();
	this->ArcLibrary.clear();

	{
		const std::lock_guard<std::mutex> lock(Loader::Library::Mutex);

		for (LibraryAddon* addon : Loader::Library::Addons)
		{
			if (!this->Filter.empty() && !String::Contains(String::ToLower(addon->Name), this->Filter)) { continue; }

			this->Library.push_back(addon);
		}
	}
	
	{
		const std::lock_guard<std::mutex> lockLoader(ArcDPS::Mutex);

		for (LibraryAddon* addon : ArcDPS::PluginLibrary)
		{
			if (!this->Filter.empty() && !String::Contains(String::ToLower(addon->Name), this->Filter)) { continue; }

			this->ArcLibrary.push_back(addon);
		}
	}
}
