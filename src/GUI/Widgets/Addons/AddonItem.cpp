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

namespace GUI
{
	float itemWidth = 520.0f;
	float itemHeight = 104.0f;

	float btnWidth = 7.5f;

	Texture* Background = nullptr;

	void AddonItem(Addon* aAddon)
	{
		if (aAddon == nullptr ||
			aAddon->Definitions == nullptr ||
			aAddon->State == EAddonState::NotLoadedDuplicate ||
			aAddon->State == EAddonState::NotLoadedIncompatible)
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
				ImGui::TextColored(ImVec4(1.0f, 0.933f, 0.733f, 1.0f), aAddon->Definitions->Name); ImGui::SameLine();
				ImGui::PopFont();
				ImGui::TextColored(ImVec4(0.666f, 0.666f, 0.666f, 1.0f), "(%s)", aAddon->Definitions->Version.ToString().c_str()); ImGui::SameLine();
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

				int amtBtns = 0;

				// just check if loaded, if it was not hot-reloadable it would be EAddonState::LoadedLOCKED
				if (aAddon->State == EAddonState::Loaded)
				{
					amtBtns++;
					if (ImGui::GW2Button(("Disable##" + sig).c_str(), ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
					{
						for (auto& it : Loader::Addons)
						{
							if (it.second->Definitions == aAddon->Definitions)
							{
								//LogDebug(CH_GUI, "Unload called: %s", it.second->Definitions->Name);
								Loader::QueueAddon(ELoaderAction::Unload, it.first);
							}
						}
					}
				}
				else if (aAddon->State == EAddonState::NotLoaded)
				{
					amtBtns++;
					if (ImGui::GW2Button(("Load##" + sig).c_str(), ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
					{
						for (auto& it : Loader::Addons)
						{
							if (it.second->Definitions == aAddon->Definitions)
							{
								//LogDebug(CH_GUI, "Load called: %s", it.second->Definitions->Name);
								Loader::QueueAddon(ELoaderAction::Load, it.first);
							}
						}
					}
				}
				{
					amtBtns++;
					if (ImGui::GW2Button(("Uninstall##" + sig).c_str(), ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
					{
						for (auto& it : Loader::Addons)
						{
							if (it.second->Definitions == aAddon->Definitions)
							{
								//LogDebug(CH_GUI, "Uninstall called: %s", it.second->Definitions->Name);
								Loader::QueueAddon(ELoaderAction::Uninstall, it.first);
							}
						}
					}
					if (aAddon->State == EAddonState::LoadedLOCKED)
					{
						ImGui::TooltipGeneric("This addon is currently locked and requires a restart to be removed.");
					}
				}
				if (aAddon->Definitions->Provider == EUpdateProvider::GitHub && aAddon->Definitions->UpdateLink)
				{
					amtBtns++;
					if (ImGui::GW2Button(("GitHub##" + sig).c_str(), ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
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

					if (ImGui::GW2Button(aAddon->IsInstalling ? ("Installing...##" + sig).c_str() : ("Install##" + sig).c_str(), ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
					{
						if (!aAddon->IsInstalling)
						{
							std::thread([aAddon]()
								{
									Loader::InstallAddon(aAddon);
									aAddon->IsInstalling = false;
								})
								.detach();
						}
					}
				}
				else
				{
					ImGui::Text("Already installed.");
				}
				if (aAddon->Provider == EUpdateProvider::GitHub && !aAddon->DownloadURL.empty())
				{
					if (ImGui::GW2Button(("GitHub##" + sig).c_str(), ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight)))
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