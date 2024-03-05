#include "AddonItem.h"

#include <shellapi.h>

#include "Shared.h"
#include "Consts.h"
#include "Renderer.h"

#include "Loader/AddonDefinition.h"
#include "Loader/EAddonFlags.h"
#include "Loader/Loader.h"
#include "Textures/TextureLoader.h"
#include "Textures/Texture.h"

#include "Events/EventHandler.h"

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
	Texture* BtnOptions = nullptr;
	Texture* CtxMenuBullet = nullptr;
	Texture* CtxMenuHighlight = nullptr;

	void AddonItem(std::filesystem::path aPath, Addon* aAddon)
	{
		if (aAddon == nullptr ||
			aAddon->Definitions == nullptr ||
			aAddon->State == EAddonState::NotLoadedDuplicate ||
			aAddon->State == EAddonState::NotLoadedIncompatible ||
			aAddon->WillBeUninstalled)
		{
			return;
		}

		float btnHeight = 22.0f * Renderer::Scaling;
		float itemWidthScaled = itemWidth * Renderer::Scaling;
		float itemHeightScaled = itemHeight * Renderer::Scaling;

		std::string sig = std::to_string(aAddon->Definitions->Signature); // helper for unique chkbxIds

		// initial is explicitly set within the window, therefore all positions should be relative to it
		ImVec2 initial = ImGui::GetCursorPos();

		if (Background)
		{
			ImGui::SetCursorPos(initial);
			ImGui::Image(Background->Resource, ImVec2(itemWidthScaled, itemHeightScaled));
		}
		else
		{
			Background = TextureLoader::GetOrCreate("TEX_ADDONITEM_BACKGROUND", RES_TEX_ADDONITEM, NexusHandle);
		}

		if (!BtnOptions) { BtnOptions = TextureLoader::GetOrCreate("ICON_OPTIONS", RES_ICON_OPTIONS, NexusHandle); }
		if (!CtxMenuBullet) { CtxMenuBullet = TextureLoader::GetOrCreate("TEX_CTXMENU_BULLET", RES_TEX_CONTEXTMENU_BULLET, NexusHandle); }
		if (!CtxMenuHighlight) { CtxMenuHighlight = TextureLoader::GetOrCreate("TEX_CTXMENU_HIGHLIGHT", RES_TEX_CONTEXTMENU_HIGHLIGHT, NexusHandle); }

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
				ImGui::GW2::TooltipGeneric("Version: %s", aAddon->Definitions->Version.ToString().c_str());
				ImGui::TextColored(ImVec4(0.666f, 0.666f, 0.666f, 1.0f), "by %s", aAddon->Definitions->Author);

				if (aAddon->State == EAddonState::NotLoadedIncompatibleAPI)
				{
					ImGui::TextColored(ImVec4(255, 255, 0, 255), "Addon requested incompatible API Version: %d", aAddon->Definitions->APIVersion);
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
						if (ImGui::GW2::ContextMenuItem(("Update##" + sig).c_str(), "Update", CtxMenuBullet->Resource, CtxMenuHighlight->Resource, ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
						{
							for (auto& it : Loader::Addons)
							{
								if (it.second->Definitions == aAddon->Definitions)
								{
									std::filesystem::path tmpPath = it.first.string();
									signed int tmpSig = aAddon->Definitions->Signature;
									std::string tmpName = aAddon->Definitions->Name;
									AddonVersion tmpVers = aAddon->Definitions->Version;
									EUpdateProvider tmpProv = aAddon->Definitions->Provider;
									std::string tmpLink = aAddon->Definitions->UpdateLink != nullptr ? aAddon->Definitions->UpdateLink : "";

									std::thread([tmpPath, tmpSig, tmpName, tmpVers, tmpProv, tmpLink]()
										{
											if (Loader::UpdateAddon(tmpPath, tmpSig, tmpName, tmpVers, tmpProv, tmpLink))
											{
												Loader::QueueAddon(ELoaderAction::Reload, tmpPath);
											}
										})
										.detach();

										//LogDebug(CH_GUI, "Update called: %s", it.second->Definitions->Name);
										break;
								}
							}
						}
						if (aAddon->State == EAddonState::LoadedLOCKED)
						{
							ImGui::GW2::TooltipGeneric("This addon is currently locked and requires a restart for the update to take effect.");
						}

						if (ImGui::GW2::ContextMenuItem((aAddon->IsPausingUpdates ? "Resume Updates##" : "Pause Updates##" + sig).c_str(),
							aAddon->IsPausingUpdates ? "Resume Updates" : "Pause Updates",
							CtxMenuBullet->Resource, CtxMenuHighlight->Resource, ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
						{
							aAddon->IsPausingUpdates = !aAddon->IsPausingUpdates;
						}

						ImGui::Separator();

						if (ImGui::GW2::ContextMenuItem(("ToggleDUU##" + sig).c_str(), aAddon->IsDisabledUntilUpdate ? "Re-Enable" : "Disable until Update", CtxMenuBullet->Resource, CtxMenuHighlight->Resource, ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
						{
							//LogDebug(CH_GUI, "ToggleDUU called: %s", it.second->Definitions->Name);
							aAddon->IsDisabledUntilUpdate = !aAddon->IsDisabledUntilUpdate;
							Loader::SaveAddonConfig();

							if (aAddon->State == EAddonState::Loaded)
							{
								Loader::QueueAddon(ELoaderAction::Unload, aPath);
							}
						}
						if (aAddon->State == EAddonState::LoadedLOCKED)
						{
							ImGui::GW2::TooltipGeneric("This addon is currently locked disabling won't take effect until next game start.");
						}

						ImGui::Separator();

						if (ImGui::GW2::ContextMenuItem(("Uninstall##" + sig).c_str(), "Uninstall", CtxMenuBullet->Resource, CtxMenuHighlight->Resource, ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
						{
							//LogDebug(CH_GUI, "Uninstall called: %s", it.second->Definitions->Name);
							Loader::QueueAddon(ELoaderAction::Uninstall, aPath);
						}
						if (aAddon->State == EAddonState::LoadedLOCKED)
						{
							ImGui::GW2::TooltipGeneric("This addon is currently locked and requires a restart to be removed.");
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
					if (ImGui::GW2::Button(("Disable##" + sig).c_str(), ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
					{
						//LogDebug(CH_GUI, "Unload called: %s", it.second->Definitions->Name);
						Loader::QueueAddon(ELoaderAction::Unload, aPath);
					}
					if (RequestedAddons.size() > 0)
					{
						ImGui::GW2::TooltipGeneric("Addon state won't be saved. Game was started with addons via start parameters.");
					}
				}
				else if (aAddon->State == EAddonState::LoadedLOCKED && aAddon->ShouldDisableNextLaunch == false)
				{
					std::string additionalInfo;

					if (RequestedAddons.size() > 0)
					{
						additionalInfo = "\nAddon state won't be saved. Game was started with addons via start parameters.";
					}

					if (ImGui::GW2::Button(("Disable*##" + sig).c_str(), ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
					{
						aAddon->ShouldDisableNextLaunch = true;
					}
					ImGui::GW2::TooltipGeneric("* Disabling won't take effect until next game start.%s", additionalInfo.c_str());
				}
				else if (aAddon->State == EAddonState::LoadedLOCKED && aAddon->ShouldDisableNextLaunch == true)
				{
					std::string additionalInfo;

					if (RequestedAddons.size() > 0)
					{
						additionalInfo = "\nAddon state won't be saved. Game was started with addons via start parameters.";
					}

					if (ImGui::GW2::Button(("Load*##" + sig).c_str(), ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
					{
						aAddon->ShouldDisableNextLaunch = false;
					}
					ImGui::GW2::TooltipGeneric("* Load addon next game start.%s", additionalInfo.c_str());
				}
				else if (aAddon->State == EAddonState::NotLoaded)
				{
					if (ImGui::GW2::Button(("Load##" + sig).c_str(), ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
					{
						//LogDebug(CH_GUI, "Load called: %s", it.second->Definitions->Name);
						aAddon->IsDisabledUntilUpdate = false; // explicitly loaded
						Loader::QueueAddon(ELoaderAction::Load, aPath);
					}
					if (RequestedAddons.size() > 0)
					{
						ImGui::GW2::TooltipGeneric("Addon state won't be saved. Game was started with addons via start parameters.");
					}
				}
				if (aAddon->Definitions->Provider == EUpdateProvider::GitHub && aAddon->Definitions->UpdateLink)
				{
					if (ImGui::GW2::Button(("GitHub##" + sig).c_str(), ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
					{
						ShellExecuteA(0, 0, aAddon->Definitions->UpdateLink, 0, 0, SW_SHOW);
					}
				}

				ImGui::EndChild();
			}

			ImGui::EndChild();

			ImGui::SetCursorPos(ImVec2(0, initial.y + itemHeightScaled + 4.0f));
		}
	}

	void AddonItem(LibraryAddon* aAddon, bool aInstalled)
	{
		float btnHeight = 22.0f * Renderer::Scaling;
		float itemWidthScaled = itemWidth * Renderer::Scaling;
		float itemHeightScaled = itemHeight * Renderer::Scaling;

		std::string sig = std::to_string(aAddon->Signature); // helper for unique chkbxIds

		// initial is explicitly set within the window, therefore all positions should be relative to it
		ImVec2 initial = ImGui::GetCursorPos();

		if (Background)
		{
			ImGui::SetCursorPos(initial);
			ImGui::Image(Background->Resource, ImVec2(itemWidthScaled, itemHeightScaled));
		}
		else
		{
			Background = TextureLoader::Get("TEX_ADDONITEM_BACKGROUND");
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

				ImGui::TextWrapped(aAddon->Description.c_str());

				ImGui::EndChild();
			}

			{
				ImGui::SetCursorPos(ImVec2(descWidth + 12.0f + 12.0f, 12.0f));
				ImGui::BeginChild("##AddonItemActions", ImVec2(actionsWidth, itemHeightScaled - 12.0f - 12.0f));

				// just check if loaded, if it was not hot-reloadable it would be EAddonState::LoadedLOCKED
				if (!aInstalled)
				{
					if (ImGui::GW2::Button(aAddon->IsInstalling ? ("Installing...##" + sig).c_str() : ("Install##" + sig).c_str(), ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
					{
						if (!aAddon->IsInstalling)
						{
							std::thread([aAddon]()
								{
									Loader::InstallAddon(aAddon);
									aAddon->IsInstalling = false;
								})
								.detach();

							Loader::AddonConfig[aAddon->Signature].IsLoaded = true;
						}
					}
				}
				else
				{
					ImGui::Text("Already installed.");
				}
				if (aAddon->Provider == EUpdateProvider::GitHub && !aAddon->DownloadURL.empty())
				{
					if (ImGui::GW2::Button(("GitHub##" + sig).c_str(), ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
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