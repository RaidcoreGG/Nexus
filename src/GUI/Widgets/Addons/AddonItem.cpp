#include "AddonItem.h"

namespace GUI
{
	void AddonItem(Addon* aAddon)
	{
		if (aAddon->State == EAddonState::NotLoadedDuplicate ||
			aAddon->State == EAddonState::Incompatible ||
			aAddon->Definitions == nullptr)
		{
			return;
		}
		
		std::string sig = std::to_string(aAddon->Definitions->Signature); // helper for unique chkbxIds

		if (ImGui::BeginTable(("#AddonItem" + sig).c_str(), 2, ImGuiTableFlags_BordersOuter, ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f)))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			{
				ImGui::BeginGroup();

				ImGui::Text(aAddon->Definitions->Name); ImGui::SameLine(); ImGui::Text("by %s", aAddon->Definitions->Author);
				ImGui::TextWrapped(aAddon->Definitions->Description);

				if (aAddon->State == EAddonState::IncompatibleAPI)
				{
					ImGui::TextColored(ImVec4(255, 255, 0, 255), "Addon requested incompatible API Version: %d", aAddon->Definitions->APIVersion);
				}

				ImGui::EndGroup();
			}

			ImGui::TableSetColumnIndex(1);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - 120.0f);
			{
				ImGui::BeginGroup();
				if (aAddon->State == EAddonState::Loaded &&
					!(aAddon->Definitions->Unload == nullptr || aAddon->Definitions->HasFlag(EAddonFlags::DisableHotloading)))
				{
					if (ImGui::Button(("Disable##" + sig).c_str(), ImVec2(120.0f, 24.0f)))
					{
						for (auto& it : Loader::Addons)
						{
							if (it.second->Definitions == aAddon->Definitions)
							{
								LogDebug(CH_GUI, "Unload called: %s", it.second->Definitions->Name);
								Loader::QueueAddon(ELoaderAction::Unload, it.first);
							}
						}
					}
				}
				else if (aAddon->State == EAddonState::NotLoaded)
				{
					if (ImGui::Button(("Load##" + sig).c_str(), ImVec2(120.0f, 24.0f)))
					{
						for (auto& it : Loader::Addons)
						{
							if (it.second->Definitions == aAddon->Definitions)
							{
								LogDebug(CH_GUI, "Load called: %s", it.second->Definitions->Name);
								Loader::QueueAddon(ELoaderAction::Load, it.first);
							}
						}
					}
				}
				if (!(aAddon->Definitions->Unload == nullptr || aAddon->Definitions->HasFlag(EAddonFlags::DisableHotloading)))
				{
					if (ImGui::Button(("Uninstall##" + sig).c_str(), ImVec2(120.0f, 24.0f)))
					{
						for (auto& it : Loader::Addons)
						{
							if (it.second->Definitions == aAddon->Definitions)
							{
								LogDebug(CH_GUI, "Uninstall called: %s", it.second->Definitions->Name);
								Loader::QueueAddon(ELoaderAction::Uninstall, it.first);
							}
						}
					}
				}
				
				ImGui::TextCenteredColumn(aAddon->Definitions->Version.ToString().c_str());

				ImGui::EndGroup();
			}

			ImGui::EndTable();
		}
	}
}