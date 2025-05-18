///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  CtlModal.cpp
/// Description  :  Contains the functionality for a modal dialog/window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "CtlModal.h"

#include "Context.h"
#include "Renderer.h"

bool IModal::Render()
{
	if (this->ShouldOpenNextFrame)
	{
		ImGui::OpenPopup(this->GetName().c_str());
		this->OnOpening();
		this->ShouldOpenNextFrame = false;
		this->SetResult(EModalResult::None); /* Reset last result. */
	}

	ImVec2 center(Renderer::Width * 0.5f, Renderer::Height * 0.5f);
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (!ImGui::BeginPopupModal(this->GetName().c_str(), NULL, ModalFlags))
	{
		return false;
	}

	this->RenderContent();

	/* Notify callback. */
	if (this->GetResult() != EModalResult::None)
	{
		this->OnClosing();
	}

	/* If there is still a result. Modal can be closed. */
	/* If the result is EModalResult::None again, the modal failed or similar and should NOT be closed. */
	if (this->GetResult() != EModalResult::None)
	{
		ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
		return true;
	}

	ImGui::EndPopup();

	return false;
}

std::string IModal::GetName() const
{
	assert(!this->GetID().empty());
	assert(!this->GetDisplayName().empty());

	CContext* ctx = CContext::GetContext();
	CLocalization* lang = ctx->GetLocalization();

	return std::string{lang->Translate(this->GetDisplayName().c_str())} + "##" + this->ID;
}

const std::string& IModal::GetID() const
{
	return this->ID;
}

const std::string& IModal::GetDisplayName() const
{
	return this->DisplayName;
}

EModalResult IModal::GetResult() const
{
	return this->Result;
}

void IModal::OpenModal()
{
	this->ShouldOpenNextFrame = true;
}

void IModal::SetID(std::string aID)
{
	this->ID = aID;
}

void IModal::SetDisplayName(std::string aDisplayName)
{
	this->DisplayName = aDisplayName;
}

void IModal::SetResult(EModalResult aResult)
{
	this->Result = aResult;
}
