///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  CmAddon.cpp
/// Description  :  Context menu for addons.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "CmAddon.h"

#include "imgui.h"

void CAddonContextMenu::RenderContent()
{
	ImGui::Text("Test");
	ImGui::Selectable(this->Data.GetName().c_str());
}

void CAddonContextMenu::SetContent(AddonListing_t& aAddonData)
{
	this->Data = aAddonData;
	this->OpenContextMenu();
}
