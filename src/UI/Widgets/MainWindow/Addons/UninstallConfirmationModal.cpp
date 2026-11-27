///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  UninstallConfirmationModal.cpp
/// Description  :  Modal for addon uninstall confirmation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "UninstallConfirmationModal.h"

#include "Host/GW2/Context.h"

namespace Raidcore::UI
{
	CUninstallConfirmationModal::CUninstallConfirmationModal()
	{
		this->SetID("UninstallConfirmationModal");
		this->SetDisplayName(Lang::Addons::UninstallConfirm);
	}

	void CUninstallConfirmationModal::RenderContent()
	{
		ImGui::Text(Lang::Common::AreYouSure.Text());

		/* Accept */
		if (ImGui::Button(Lang::Common::Accept.Text()))
		{
			this->SetResult(UI::EModalResult::OK);
		}

		ImGui::SameLine();

		/* Cancel */
		if (ImGui::Button(Lang::Common::Cancel.Text()))
		{
			this->SetResult(UI::EModalResult::Cancel);
		}
	}

	void CUninstallConfirmationModal::OnClosing()
	{
		GW2::CContext*    ctx    = GW2::CContext::GetContext();
		Engine::CLoader*     loader = ctx->GetLoader();

		switch (this->GetResult())
		{
			case UI::EModalResult::OK:
			{
				// FIXME: loader->UninstallSafe(this->Path);
				break;
			}
			case UI::EModalResult::Cancel:
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
		/* Override the title before opening the modal. */
		this->SetDisplayName(Lang::Addons::UninstallConfirm, this->Name);
	}
}
