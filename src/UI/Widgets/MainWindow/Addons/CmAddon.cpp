///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  CmAddon.cpp
/// Description  :  Context menu for addons.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "CmAddon.h"

#include "imgui.h"

#include "CtlAddonToggle.h"

void CAddonContextMenu::RenderContent()
{
	if (this->Data->Addon)
	{
		if (this->Data->Addon->SupportsLoading())
		{
			if (ImGui::Selectable(AddonToggleCtl::GetButtonText(this->Data->Addon).c_str()))
			{
				/* Prompt if true, otherwise it already toggled now. */
				if (AddonToggleCtl::Toggle(this->Data->Addon))
				{
					Config_t* config = this->Data->Addon->GetConfig();

					// TODO: This is a parent object.
					//this->LoadConfirmationModal.SetTarget(config, this->Data->GetName(), this->Data->Addon->GetLocation());
				}
			}

			if (!this->Data->Addon->IsVersionDisabled())
			{
				if (ImGui::Selectable("((Disable until Update))"))
				{
					// TODO: Config disable version
				}
			}

			if (this->Data->Addon->IsUpdateAvailable())
			{
				if (ImGui::Selectable("((Update))"))
				{
					this->Data->Addon->Update();
				}
			}
			else
			{
				if (ImGui::Selectable("((Check for Update))"))
				{
					this->Data->Addon->CheckForUpdate();
				}
			}

			ImGui::Separator();
		}

		if (ImGui::Selectable("((Uninstall))"))
		{
			this->Data->Addon->Uninstall();
		}
	}
}

void CAddonContextMenu::SetContent(AddonListing_t& aAddonData)
{
	this->Data = &aAddonData;
	this->OpenContextMenu();
}
