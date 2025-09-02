///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  CtlContextMenu.cpp
/// Description  :  Contains the functionality for a context menu.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "CtlContextMenu.h"

#include "imgui.h"

void IContextMenu::Render()
{
	if (this->ShouldOpenNextFrame)
	{
		ImGui::OpenPopup(this->GetID().c_str());
		this->ShouldOpenNextFrame = false;
	}

	constexpr ImGuiWindowFlags ContextMenuFlags
		= ImGuiWindowFlags_NoMove;

	if (!ImGui::BeginPopup(this->GetID().c_str(), ContextMenuFlags))
	{
		return;
	}

	this->RenderContent();

	ImGui::EndPopup();
}

const std::string& IContextMenu::GetID() const
{
	return this->ID;
}

void IContextMenu::OpenContextMenu()
{
	this->ShouldOpenNextFrame = true;
}

void IContextMenu::SetID(std::string aID)
{
	this->ID = aID;
}
