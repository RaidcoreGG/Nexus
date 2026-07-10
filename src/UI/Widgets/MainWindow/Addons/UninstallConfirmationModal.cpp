///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  UninstallConfirmationModal.cpp
/// Description  :  Modal for addon uninstall confirmation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "UninstallConfirmationModal.h"

#include "Runtime/Runtime.h"
using namespace Raidcore::Nexus;

CUninstallConfirmationModal::CUninstallConfirmationModal()
{
	this->SetID("UninstallConfirmationModal");
	this->SetDisplayName("((000117))");
}

void CUninstallConfirmationModal::RenderContent()
{
	Runtime&      ctx   = Runtime::Get();
	CUiContext*    uictx = ctx.GetUIContext();
	CLocalization* lang  = uictx->GetLocalization();

	ImGui::Text(lang->Translate("((000116))"));

	/* Accept */
	if (ImGui::Button(lang->Translate("((000065))")))
	{
		this->SetResult(EModalResult::OK);
	}

	ImGui::SameLine();

	/* Cancel */
	if (ImGui::Button(lang->Translate("((000066))")))
	{
		this->SetResult(EModalResult::Cancel);
	}
}

void CUninstallConfirmationModal::OnClosing()
{
	Runtime&    ctx    = Runtime::Get();
	Host::CLoader&    loader = ctx.Host().Loader();

	switch (this->GetResult())
	{
		case EModalResult::OK:
		{
			// FIXME: loader->UninstallSafe(this->Path);
			break;
		}
		case EModalResult::Cancel:
		{
			break;
		}
	}

	/* Reset data. */
	this->Name.clear();
	this->Path.clear();
}

void CUninstallConfirmationModal::SetTarget(std::string aName, std::filesystem::path aPath)
{
	this->Name = aName;
	this->Path = aPath;

	this->SetTitle();

	this->OpenModal();
}

void CUninstallConfirmationModal::SetTitle()
{
	Runtime& ctx = Runtime::Get();
	CUiContext* uictx = ctx.GetUIContext();
	CLocalization* lang = uictx->GetLocalization();

	/* Override the title before opening the modal. */
	std::string title = lang->Translate("((000117))");
	title.append(this->Name);

	this->SetDisplayName(title);
}
