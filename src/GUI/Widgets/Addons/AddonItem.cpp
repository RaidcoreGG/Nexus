#include "AddonItem.h"

#include <shellapi.h>

#include "Shared.h"
#include "Consts.h"
#include "Renderer.h"

#include "Loader/AddonDefinition.h"
#include "Loader/EAddonFlags.h"
#include "Loader/Loader.h"
#include "Loader/Library.h"
#include "Loader/ArcDPS.h"
#include "Services/Textures/TextureLoader.h"
#include "Services/Textures/Texture.h"

#include "Services/Updater/Updater.h"

#include "Events/EventHandler.h"

#include "GUI/Widgets/Alerts/Alerts.h"

#include "imgui.h"
#include "imgui_extensions.h"

#include "resource.h"

namespace GUI
{
	float itemWidth = 520.0f;
	float itemHeight = 104.0f;

	float btnWidth = 7.5f;

	float size = 24.0f;

	Texture* Background = nullptr;
	Texture* BackgroundHighlight = nullptr;
	Texture* BackgroundDUU = nullptr;
	Texture* BtnOptions = nullptr;
	Texture* CtxMenuBullet = nullptr;
	Texture* CtxMenuHighlight = nullptr;
	Texture* ToSComplianceWarning = nullptr;

	void AddonItem(std::filesystem::path aPath, Addon* aAddon)
	{
		if (aAddon == nullptr ||
			aAddon->Definitions == nullptr ||
			aAddon->State == EAddonState::NotLoadedDuplicate ||
			aAddon->State == EAddonState::NotLoadedIncompatible ||
			aAddon->IsFlaggedForUninstall)
		{
			return;
		}

		float btnHeight = 22.0f * Renderer::Scaling;
		float itemWidthScaled = itemWidth * Renderer::Scaling;
		float itemHeightScaled = itemHeight * Renderer::Scaling;

		std::string sig = std::to_string(aAddon->Definitions->Signature); // helper for unique chkbxIds

		// initial is explicitly set within the window, therefore all positions should be relative to it
		ImVec2 initial = ImGui::GetCursorPos();

		if (aAddon->IsDisabledUntilUpdate)
		{
			if (BackgroundDUU)
			{
				ImGui::SetCursorPos(initial);
				ImGui::Image(BackgroundDUU->Resource, ImVec2(itemWidthScaled, itemHeightScaled));
			}
			else
			{
				BackgroundDUU = TextureService->GetOrCreate("TEX_ADDONITEM_BACKGROUND_DUU", RES_TEX_ADDONITEM_DUU, NexusHandle);
			}
		}
		else
		{
			if (Background)
			{
				ImGui::SetCursorPos(initial);
				ImGui::Image(Background->Resource, ImVec2(itemWidthScaled, itemHeightScaled));
			}
			else
			{
				Background = TextureService->GetOrCreate("TEX_ADDONITEM_BACKGROUND", RES_TEX_ADDONITEM, NexusHandle);
			}
		}

		if (!BtnOptions) { BtnOptions = TextureService->GetOrCreate("ICON_OPTIONS", RES_ICON_OPTIONS, NexusHandle); }
		if (!CtxMenuBullet) { CtxMenuBullet = TextureService->GetOrCreate("TEX_CTXMENU_BULLET", RES_TEX_CONTEXTMENU_BULLET, NexusHandle); }
		if (!CtxMenuHighlight) { CtxMenuHighlight = TextureService->GetOrCreate("TEX_CTXMENU_HIGHLIGHT", RES_TEX_CONTEXTMENU_HIGHLIGHT, NexusHandle); }

		{
			ImGui::SetCursorPos(initial);
			ImGui::BeginChild(("##AddonItemContent" + sig).c_str(), ImVec2(itemWidthScaled, itemHeightScaled));

			float actionsWidth = btnWidth * ImGui::GetFontSize();
			float descWidth = itemWidthScaled - actionsWidth - 12.0f - 12.0f - 12.0f; // padding left, right and in between

			{
				ImGui::SetCursorPos(ImVec2(12.0f, 12.0f));
				ImGui::BeginChild("##AddonItemDescription", ImVec2(descWidth, itemHeightScaled - 12.0f - 12.0f));

				ImGui::PushFont(Font);
				ImGui::TextColored(ImVec4(1.0f, 0.933f, 0.733f, 1.0f), aAddon->Definitions->Name); ImGui::SameLine();
				ImGui::PopFont();
				ImGui::TextColored(ImVec4(0.666f, 0.666f, 0.666f, 1.0f), "(%s)", aAddon->Definitions->Version.string().c_str()); ImGui::SameLine();
				ImGui::TextColored(ImVec4(0.666f, 0.666f, 0.666f, 1.0f), "by %s", aAddon->Definitions->Author);

				if (aAddon->State == EAddonState::NotLoadedIncompatibleAPI)
				{
					ImGui::TextColored(ImVec4(255, 255, 0, 255), Language->Translate("((000010))"), aAddon->Definitions->APIVersion);
				}
				else
				{
					ImGui::TextWrapped(aAddon->Definitions->Description);
				}

				ImGui::EndChild();
			}

			{
				ImGui::SetCursorPos(ImVec2(descWidth + 12.0f + 12.0f, 12.0f));
				ImGui::BeginChild("##AddonItemActions", ImVec2(actionsWidth, itemHeightScaled - 12.0f - 12.0f));

				if (BtnOptions && BtnOptions->Resource)
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
					ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.863f, 0.863f, 0.863f, 1));
					ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.13f, 0.13f, 0.13f, 1));
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 0.f });
					ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 1.0f);
					ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 0);

					ImGui::SetCursorPos(ImVec2(actionsWidth - (size * Renderer::Scaling), ImGui::GetCursorPosY()));
					ImGui::ImageButton(BtnOptions->Resource, ImVec2(size * Renderer::Scaling, size * Renderer::Scaling));
					if (ImGui::BeginPopupContextItem("##AddonItemActionsMore"))
					{
						if (ImGui::GW2::ContextMenuItem(("Update##" + sig).c_str(), !aAddon->IsCheckingForUpdates ? Language->Translate("((000011))") : Language->Translate("((000071))"), CtxMenuBullet->Resource, CtxMenuHighlight->Resource, ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
						{
							if (!aAddon->IsCheckingForUpdates)
							{
								for (auto addon : Loader::Addons)
								{
									if (addon->Definitions == aAddon->Definitions)
									{
										aAddon->IsCheckingForUpdates = true;

										std::filesystem::path tmpPath = addon->Path.string();
										std::thread([aAddon, tmpPath]()
											{
												AddonInfo addonInfo
												{
													aAddon->Definitions->Signature,
													aAddon->Definitions->Name,
													aAddon->Definitions->Version,
													aAddon->Definitions->Provider,
													aAddon->Definitions->UpdateLink != nullptr 
														? aAddon->Definitions->UpdateLink 
														: "",
													aAddon->MD5,
													aAddon->AllowPrereleases
												};

												if (UpdateService->UpdateAddon(tmpPath, addonInfo))
												{
													Loader::QueueAddon(ELoaderAction::Reload, tmpPath);

													GUI::Alerts::Notify(
														String::Format("%s %s",
															aAddon->Definitions->Name,
															aAddon->State == EAddonState::LoadedLOCKED
															? Language->Translate("((000079))")
															: Language->Translate("((000081))")
														).c_str()
													);
												}
												else
												{
													GUI::Alerts::Notify(String::Format("%s %s",
														aAddon->Definitions->Name,
														Language->Translate("((000082))")).c_str());
												}
												Sleep(1000); // arbitrary sleep otherwise the user never even sees "is checking..."
												aAddon->IsCheckingForUpdates = false;
											})
											.detach();

										//LogDebug(CH_GUI, "Update called: %s", it.second->Definitions->Name);
										break;
									}
								}
							}
						}
						if (aAddon->State == EAddonState::LoadedLOCKED)
						{
							ImGui::GW2::TooltipGeneric(Language->Translate("((000012))"));
						}

						if (ImGui::GW2::ContextMenuItem(("ToggleUpdates##" + sig).c_str(), aAddon->IsPausingUpdates ? Language->Translate("((000013))") : Language->Translate("((000014))"), CtxMenuBullet->Resource, CtxMenuHighlight->Resource, ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
						{
							aAddon->IsPausingUpdates = !aAddon->IsPausingUpdates;
							Loader::SaveAddonConfig();
						}

						if (aAddon->Definitions->Provider == EUpdateProvider::GitHub)
						{
							if (ImGui::GW2::ContextMenuItem(("TogglePrereleases##" + sig).c_str(), aAddon->AllowPrereleases ? Language->Translate("((000085))") : Language->Translate("((000084))"), CtxMenuBullet->Resource, CtxMenuHighlight->Resource, ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
							{
								aAddon->AllowPrereleases = !aAddon->AllowPrereleases;
								Loader::SaveAddonConfig();
							}
						}

						ImGui::Separator();

						if (ImGui::GW2::ContextMenuItem(("ToggleDUU##" + sig).c_str(), aAddon->IsDisabledUntilUpdate ? Language->Translate("((000015))") : Language->Translate("((000016))"), CtxMenuBullet->Resource, CtxMenuHighlight->Resource, ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
						{
							//LogDebug(CH_GUI, "ToggleDUU called: %s", it.second->Definitions->Name);
							aAddon->IsDisabledUntilUpdate = !aAddon->IsDisabledUntilUpdate;
							if (aAddon->IsDisabledUntilUpdate)
							{
								/* if explicitly disabled until update from menu, override pausing update setting */
								aAddon->IsPausingUpdates = false;
							}
							Loader::SaveAddonConfig();

							if (aAddon->State == EAddonState::Loaded)
							{
								Loader::QueueAddon(ELoaderAction::Unload, aPath);
							}
						}
						if (aAddon->State == EAddonState::LoadedLOCKED)
						{
							ImGui::GW2::TooltipGeneric(Language->Translate("((000017))"));
						}

						ImGui::Separator();

						if (ImGui::GW2::ContextMenuItem(("Uninstall##" + sig).c_str(), Language->Translate("((000018))"), CtxMenuBullet->Resource, CtxMenuHighlight->Resource, ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
						{
							//LogDebug(CH_GUI, "Uninstall called: %s", it.second->Definitions->Name);
							Loader::QueueAddon(ELoaderAction::Uninstall, aPath);
						}
						if (aAddon->State == EAddonState::LoadedLOCKED)
						{
							ImGui::GW2::TooltipGeneric(Language->Translate("((000019))"));
						}

						ImGui::EndPopup();
					}
					ImGui::OpenPopupOnItemClick("##AddonItemActionsMore", 0);

					ImGui::PopStyleVar(3);
					ImGui::PopStyleColor(5);
				}

				// just check if loaded, if it was not hot-reloadable it would be EAddonState::LoadedLOCKED
				if (aAddon->State == EAddonState::Loaded)
				{
					if (ImGui::GW2::Button((aAddon->IsWaitingForUnload ? Language->Translate("((000078))") : Language->Translate("((000020))") + sig).c_str(), ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
					{
						if (!aAddon->IsWaitingForUnload)
						{
							//LogDebug(CH_GUI, "Unload called: %s", it.second->Definitions->Name);
							Loader::QueueAddon(ELoaderAction::Unload, aPath);
						}
					}
					if (RequestedAddons.size() > 0)
					{
						ImGui::GW2::TooltipGeneric(Language->Translate("((000021))"));
					}
				}
				else if (aAddon->State == EAddonState::LoadedLOCKED)
				{
					std::string additionalInfo;

					if (RequestedAddons.size() > 0)
					{
						additionalInfo.append("\n");
						additionalInfo.append(Language->Translate("((000021))"));
					}

					if (ImGui::GW2::Button((Language->Translate(aAddon->IsFlaggedForDisable ? "((000024))" : "((000022))") + sig).c_str(), ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
					{
						aAddon->IsFlaggedForDisable = !aAddon->IsFlaggedForDisable;
						Loader::SaveAddonConfig();
					}
					ImGui::GW2::TooltipGeneric(Language->Translate(aAddon->IsFlaggedForDisable ? "((000025))" : "((000023))"), additionalInfo.c_str());
				}
				else if (aAddon->State == EAddonState::NotLoaded && (aAddon->Definitions->HasFlag(EAddonFlags::OnlyLoadDuringGameLaunchSequence) || aAddon->Definitions->Signature == 0xFFF694D1) && !IsGameLaunchSequence)
				{
					/* if it's too late to load this addon */
					if (ImGui::GW2::Button((Language->Translate(aAddon->IsFlaggedForEnable ? "((000020))" : "((000024))") + sig).c_str(), ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
					{
						aAddon->IsFlaggedForEnable = !aAddon->IsFlaggedForEnable;

						if (aAddon->IsFlaggedForEnable)
						{
							aAddon->IsDisabledUntilUpdate = false; // explicitly loaded
							GUI::Alerts::Notify(String::Format("%s %s", aAddon->Definitions->Name, Language->Translate("((000080))")).c_str());
						}

						Loader::SaveAddonConfig();
					}
					if (aAddon->IsFlaggedForEnable)
					{
						ImGui::GW2::TooltipGeneric(Language->Translate("((000025))"), "");
					}
				}
				else if (aAddon->State == EAddonState::NotLoaded)
				{
					if (ImGui::GW2::Button((Language->Translate("((000026))") + sig).c_str(), ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
					{
						//LogDebug(CH_GUI, "Load called: %s", it.second->Definitions->Name);
						aAddon->IsDisabledUntilUpdate = false; // explicitly loaded
						Loader::QueueAddon(ELoaderAction::Load, aPath);
					}
					if (RequestedAddons.size() > 0)
					{
						ImGui::GW2::TooltipGeneric(Language->Translate("((000021))"));
					}
				}

				if (aAddon->Definitions->Provider == EUpdateProvider::GitHub && aAddon->Definitions->UpdateLink)
				{
					if (ImGui::GW2::Button((Language->Translate("((000030))") + sig).c_str(), ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
					{
						ShellExecuteA(0, 0, aAddon->Definitions->UpdateLink, 0, 0, SW_SHOW);
					}
				}

				ImGui::EndChild();
			}

			ImGui::EndChild();

			ImGui::SetCursorPos(ImVec2(0, initial.y + itemHeightScaled + 3.0f));
		}
	}

	void AddonItem(LibraryAddon* aAddon, bool aInstalled, bool aIsArcPlugin)
	{
		float btnHeight = 22.0f * Renderer::Scaling;
		float itemWidthScaled = itemWidth * Renderer::Scaling;
		float itemHeightScaled = itemHeight * Renderer::Scaling;

		std::string sig = std::to_string(aAddon->Signature); // helper for unique chkbxIds

		// initial is explicitly set within the window, therefore all positions should be relative to it
		ImVec2 initial = ImGui::GetCursorPos();

		if (aAddon->IsNew)
		{
			if (BackgroundHighlight)
			{
				ImGui::SetCursorPos(initial);
				ImGui::Image(BackgroundHighlight->Resource, ImVec2(itemWidthScaled, itemHeightScaled));
			}
			else
			{
				BackgroundHighlight = TextureService->GetOrCreate("TEX_ADDONITEM_BACKGROUND_HIGHLIGHT", RES_TEX_ADDONITEM_HIGHLIGHT, NexusHandle);
			}
		}
		else
		{
			if (Background)
			{
				ImGui::SetCursorPos(initial);
				ImGui::Image(Background->Resource, ImVec2(itemWidthScaled, itemHeightScaled));
			}
			else
			{
				Background = TextureService->GetOrCreate("TEX_ADDONITEM_BACKGROUND", RES_TEX_ADDONITEM, NexusHandle);
			}
		}

		{
			ImGui::SetCursorPos(initial);
			ImGui::BeginChild(("##AddonItemContent" + sig).c_str(), ImVec2(itemWidthScaled, itemHeightScaled));

			float actionsWidth = btnWidth * ImGui::GetFontSize();
			float descWidth = itemWidthScaled - actionsWidth - 12.0f - 12.0f - 12.0f; // padding left, right and in between

			{
				ImGui::SetCursorPos(ImVec2(12.0f, 12.0f));
				ImGui::BeginChild("##AddonItemDescription", ImVec2(descWidth, itemHeightScaled - 12.0f - 12.0f));

				ImGui::PushFont(Font);
				ImGui::TextColored(ImVec4(1.0f, 0.933f, 0.733f, 1.0f), aAddon->Name.c_str());
				ImGui::PopFont();
				if (!aAddon->Author.empty())
				{
					ImGui::SameLine();
					ImGui::TextColored(ImVec4(0.666f, 0.666f, 0.666f, 1.0f), "by %s", aAddon->Author.c_str());
				}

				ImGui::TextWrapped(aAddon->Description.c_str());

				ImGui::EndChild();
			}

			{
				ImGui::SetCursorPos(ImVec2(descWidth + 12.0f + 12.0f, 12.0f));
				ImGui::BeginChild("##AddonItemActions", ImVec2(actionsWidth, itemHeightScaled - 12.0f - 12.0f));

				if (!aAddon->ToSComplianceNotice.empty())
				{
					if (ToSComplianceWarning)
					{
						std::string tosNotice = Language->Translate("((000074))");
						tosNotice.append("\n");
						tosNotice.append(aAddon->ToSComplianceNotice);

						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
						ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.863f, 0.863f, 0.863f, 1));
						ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 0.f });

						ImGui::SetCursorPos(ImVec2(actionsWidth - (size * Renderer::Scaling), ImGui::GetCursorPosY()));
						if (ImGui::ImageButton(ToSComplianceWarning->Resource, ImVec2(size * Renderer::Scaling, size * Renderer::Scaling)))
						{
							ShellExecuteA(0, 0, "https://help.guildwars2.com/hc/en-us/articles/360013625034-Policy-Third-Party-Programs", 0, 0, SW_SHOW);
						}
						ImGui::PopStyleColor(4);
						ImGui::PopStyleVar();

						ImGui::GW2::TooltipGeneric(tosNotice.c_str());
					}
					else
					{
						ToSComplianceWarning = TextureService->GetOrCreate(ICON_WARNING, RES_ICON_WARNING, NexusHandle);
					}
				}

				// just check if loaded, if it was not hot-reloadable it would be EAddonState::LoadedLOCKED
				if (!aInstalled)
				{
					if (ImGui::GW2::Button(aAddon->IsInstalling ? (Language->Translate("((000027))") + sig).c_str() : (Language->Translate("((000028))") + sig).c_str(), ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
					{
						if (!aAddon->IsInstalling)
						{
							std::thread([aAddon, aIsArcPlugin]()
								{
									UpdateService->InstallAddon(aAddon, aIsArcPlugin);

									if (aIsArcPlugin)
									{
										ArcDPS::AddToAtlasBySig(aAddon->Signature);
									}
									else
									{
										Loader::NotifyChanges();
										//Loader::QueueAddon(ELoaderAction::Reload, installPath);
									}

									aAddon->IsInstalling = false;
								})
								.detach();

							//Loader::AddonConfig[aAddon->Signature].IsLoaded = true;
						}
					}
				}
				else
				{
					ImGui::Text(Language->Translate("((000029))"));
				}
				if (aAddon->Provider == EUpdateProvider::GitHub && !aAddon->DownloadURL.empty())
				{
					if (ImGui::GW2::Button((Language->Translate("((000030))") + sig).c_str(), ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
					{
						ShellExecuteA(0, 0, aAddon->DownloadURL.c_str(), 0, 0, SW_SHOW);
					}
				}

				ImGui::EndChild();
			}

			ImGui::EndChild();

			ImGui::SetCursorPos(ImVec2(0, initial.y + itemHeightScaled + 4.0f));
		}
	}
}