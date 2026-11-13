///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  CmAddon.cpp
/// Description  :  Context menu for addons.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "CmAddon.h"

#include "imgui/imgui.h"

#include "CtlAddonToggle.h"

void CAddonContextMenu::RenderContent()
{
	if (this->Data->Addon)
	{
		if (this->Data->Addon->SupportsLoading())
		{
			if (!this->Data->Addon->IsVersionDisabled())
			{
				if (ImGui::Selectable(Lang::Addons::DisableUntilUpdate.Text()))
				{
					// TODO: Config disable version
				}
			}

			if (this->Data->Addon->IsUpdateAvailable())
			{
				if (ImGui::Selectable(Lang::Addons::Update.Text()))
				{
					this->Data->Addon->Update();
				}
			}
			else
			{
				if (ImGui::Selectable(Lang::Addons::CheckForUpdates.Text()))
				{
					this->Data->Addon->CheckUpdate();
				}
			}

			ImGui::Separator();
		}

		if (ImGui::Selectable(Lang::Addons::Uninstall.Text()))
		{
			// TODO: This is a parent object.
			//this->UninstallConfirmationModal.SetTarget(config, this->Data->GetName(), this->Data->Addon->GetLocation());
		}
	}
}

void CAddonContextMenu::SetContent(AddonListing_t& aAddonData)
{
	this->Data = &aAddonData;
	this->OpenContextMenu();
}
