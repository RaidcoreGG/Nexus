///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LoadConfirmationModal.cpp
/// Description  :  Modal for addon load confirmation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "LoadConfirmationModal.h"

#include "Host/GW2/Context.h"

CLoadConfirmationModal::CLoadConfirmationModal()
{
	this->SetID("LoadConfirmationModal");
	this->SetDisplayName(Lang::Addons::LoadConfirm);
}

void CLoadConfirmationModal::RenderContent()
{
	ImGui::Text(Lang::Common::AreYouSure.Text());

	/* Accept */
	if (ImGui::Button(Lang::Common::Accept.Text()))
	{
		this->SetResult(EModalResult::OK);
	}

	ImGui::SameLine();

	/* Cancel */
	if (ImGui::Button(Lang::Common::Cancel.Text()))
	{
		this->SetResult(EModalResult::Cancel);
	}
}

void CLoadConfirmationModal::OnClosing()
{
	CContext*   ctx    = CContext::GetContext();
	CConfigMgr* cfgmgr = ctx->GetCfgMgr();
	CLoader*    loader = ctx->GetLoader();

	switch (this->GetResult())
	{
		case EModalResult::OK:
		{
			this->Config->LastGameBuild = 0;
			this->Config->LastLoadState = true;
			this->Config->DisableVersion = "";
			cfgmgr->SaveConfigs();
			loader->LoadSafe(this->Path);
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

void CLoadConfirmationModal::SetTarget(Config_t* aConfig, std::string aName, std::filesystem::path aPath)
{
	assert(aConfig != nullptr);

	this->Config = aConfig;
	this->Name = aName;
	this->Path = aPath;

	this->SetTitle();

	this->OpenModal();
}

void CLoadConfirmationModal::SetTitle()
{
	/* Override the title before opening the modal. */
	this->SetDisplayName(Lang::Addons::LoadConfirm, this->Name);
}
