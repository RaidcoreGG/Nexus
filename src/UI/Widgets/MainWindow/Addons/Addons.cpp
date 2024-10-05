///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Addons.cpp
/// Description  :  Contains the content of the addons window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Addons.h"

#include "imgui/imgui.h"
#include "imgui_extensions.h"

#include "resource.h"

CAddonsWindow::CAddonsWindow()
{
	this->Name           = "Addons";
	this->IconIdentifier = "ICON_ADDONS";
	this->IconID         = RES_ICON_ADDONS;

	this->IsHost         = true; /* set to true, to be able to pop-out addon details */

	this->Details = new CAddonDetailsWindow();
	this->Invalidate();
}

void CAddonsWindow::RenderContent()
{
	if (this->IsInvalid)
	{
		this->IsInvalid = false;
	}

	/* filters */
	static char filter[400] = {};

	ImGui::Text("Filter:");
	ImGui::SameLine();
	ImGui::InputText("##addonsfilter", &filter[0], 400);

	std::string filterStr = filter;
	
	/* list */
	if (ImGui::BeginChild("##Addons_List"))
	{

	}
	ImGui::EndChild();

	/* details */
	if (!this->Details->IsPopOut())
	{
		if (ImGui::BeginChild("##Addons_Details"))
		{
			this->Details->Render();
		}
		ImGui::EndChild();
	}
}

void CAddonsWindow::RenderSubWindows()
{
	if (this->Details->IsPopOut())
	{
		this->Details->Render();
	}
}

void CAddonsWindow::Invalidate()
{
	this->IsInvalid = true;
	this->Details->Invalidate();
}
