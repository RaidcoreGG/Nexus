///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LoadConfirmationModal.cpp
/// Description  :  Modal for addon load confirmation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "LoadConfirmationModal.h"

#include "Core/Context.h"

CLoadConfirmationModal::CLoadConfirmationModal()
{
	this->SetID("LoadConfirmationModal");
	this->SetDisplayName("((Load addon: ))");
}

void CLoadConfirmationModal::RenderContent()
{
	CContext*      ctx   = CContext::GetContext();
	CUiContext*    uictx = ctx->GetUIContext();
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
	CContext*      ctx   = CContext::GetContext();
	CUiContext*    uictx = ctx->GetUIContext();
	CLocalization* lang  = uictx->GetLocalization();

	/* Override the title before opening the modal. */
	std::string title = lang->Translate("((Load addon: ))");
	title.append(this->Name);

	this->SetDisplayName(title);
}
