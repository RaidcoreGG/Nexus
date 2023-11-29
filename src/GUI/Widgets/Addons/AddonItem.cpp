#include "AddonItem.h"

#include "Shared.h"
#include "Consts.h"

#include "Loader/AddonDefinition.h"
#include "Loader/EAddonFlags.h"
#include "Loader/Loader.h"

#include "Events/EventHandler.h"

#include "imgui.h"
#include "imgui_extensions.h"

namespace GUI
{
	float btnWidth = 7.5f;
	float btnHeight = 1.5f;

	void AddonItem(Addon* aAddon)
	{
		if (aAddon->State == EAddonState::NotLoadedDuplicate ||
			aAddon->State == EAddonState::Incompatible ||
			aAddon->Definitions.Signature == 0)
		{
			return;
		}

		std::string sig = std::to_string(aAddon->Definitions.Signature); // helper for unique chkbxIds

		if (ImGui::BeginTable(("#AddonItem" + sig).c_str(), 1, ImGuiTableFlags_BordersOuter, ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f)))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			{
				ImGui::BeginGroup();

				ImGui::Text(aAddon->Definitions.Name); ImGui::SameLine();
				ImGui::TextDisabled("(%s)", aAddon->Definitions.Version.ToString().c_str()); ImGui::SameLine();
				ImGui::TextDisabled("by %s", aAddon->Definitions.Author);
				ImGui::TextWrapped(aAddon->Definitions.Description);

				if (aAddon->State == EAddonState::IncompatibleAPI)
				{
					ImGui::TextColored(ImVec4(255, 255, 0, 255), "Addon requested incompatible API Version: %d", aAddon->Definitions.APIVersion);
				}

				ImGui::EndGroup();
			}

			{
				int amtBtns = 0;

				ImGui::BeginGroup();
				if (aAddon->State == EAddonState::Loaded &&
					!(aAddon->Definitions.Unload == nullptr || aAddon->Definitions.HasFlag(EAddonFlags::DisableHotloading)))
				{
					amtBtns++;
					if (ImGui::Button(("Disable##" + sig).c_str(), ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight * ImGui::GetFontSize())))
					{
						for (auto& it : Loader::Addons)
						{
							if (it.second->Definitions.Signature == aAddon->Definitions.Signature)
							{
								LogDebug(CH_GUI, "Unload called: %s", it.second->Definitions.Name);
								Loader::QueueAddon(ELoaderAction::Unload, it.first);
							}
						}
					}
				}
				else if (aAddon->State == EAddonState::NotLoaded)
				{
					amtBtns++;
					if (ImGui::Button(("Load##" + sig).c_str(), ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight * ImGui::GetFontSize())))
					{
						for (auto& it : Loader::Addons)
						{
							if (it.second->Definitions.Signature == aAddon->Definitions.Signature)
							{
								LogDebug(CH_GUI, "Load called: %s", it.second->Definitions.Name);
								Loader::QueueAddon(ELoaderAction::Load, it.first);
							}
						}
					}
				}
				if (!(aAddon->Definitions.Unload == nullptr || aAddon->Definitions.HasFlag(EAddonFlags::DisableHotloading)))
				{
					if (amtBtns > 0)
					{
						ImGui::SameLine();
					}
					if (ImGui::Button(("Uninstall##" + sig).c_str(), ImVec2(btnWidth * ImGui::GetFontSize(), btnHeight * ImGui::GetFontSize())))
					{
						for (auto& it : Loader::Addons)
						{
							if (it.second->Definitions.Signature == aAddon->Definitions.Signature)
							{
								LogDebug(CH_GUI, "Uninstall called: %s", it.second->Definitions.Name);
								Loader::QueueAddon(ELoaderAction::Uninstall, it.first);
							}
						}
					}
				}

				ImGui::EndGroup();
			}

			ImGui::EndTable();
		}
	}
}