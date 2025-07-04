///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  BindSetterModal.cpp
/// Description  :  Modal for InputBind setter.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "BindSetterModal.h"

#include "Core/Context.h"
#include "GW2/Inputs/GameBinds/GbConst.h"
#include "Engine/Inputs/InputBinds/IbConst.h"

CBindSetterModal::CBindSetterModal()
{
	this->SetID("BindSetterModal");
	this->SetDisplayName("((000062))");
}

void CBindSetterModal::RenderContent()
{
	CContext*      ctx   = CContext::GetContext();
	CUiContext*    uictx = ctx->GetUIContext();
	CLocalization* lang  = uictx->GetLocalization();
	CInputBindApi* ibapi = ctx->GetInputBindApi();

	this->Capture = ibapi->GetCapture();

	/* Display current bind text, if no capture yet. */
	if (this->Capture == InputBind_t{})
	{
		ImGui::Text(this->PreviousBindText.c_str());
	}
	else
	{
		ImGui::Text(IBToString(this->Capture, true).c_str());
	}

	if (this->Type == EBindEditType::Nexus)
	{
		this->BindConflict = ibapi->IsInUse(this->Capture);

		if (!this->BindConflict.empty() && this->BindConflict != this->NexusBindID)
		{
			ImGui::TextColored(ImVec4(255, 0, 0, 255), (lang->Translate("((000063))") + this->BindConflict + ".").c_str());
		}
	}

	/* Unbind */
	if (ImGui::Button(lang->Translate("((000064))")))
	{
		this->SetResult(EModalResult::Secondary);
	}

	ImGui::SameLine();

	/* Accept */
	if (ImGui::Button(lang->Translate("((000065))")))
	{
		this->SetResult(EModalResult::Primary);
	}

	ImGui::SameLine();

	/* Cancel */
	if (ImGui::Button(lang->Translate("((000066))")))
	{
		this->SetResult(EModalResult::Cancel);
	}
}

void CBindSetterModal::OnOpening()
{
	CContext*      ctx   = CContext::GetContext();
	CInputBindApi* ibapi = ctx->GetInputBindApi();
	CGameBindsApi* gbapi = ctx->GetGameBindsApi();

	/* Fetch display text. */
	switch (this->Type)
	{
		case EBindEditType::Nexus:
		{
			const InputBind_t* ib = ibapi->Get(this->NexusBindID);

			assert(ib != nullptr);

			this->PreviousBindText = IBToString(*ib, true);
			break;
		}
		case EBindEditType::Game:
		{
			const MultiInputBind_t* gb = gbapi->Get(this->GameBindID);

			assert(gb != nullptr);

			this->PreviousBindText = IBToString(gb->Primary, true);
			break;
		}
		case EBindEditType::Game2:
		{
			const MultiInputBind_t* gb = gbapi->Get(this->GameBindID);

			assert(gb != nullptr);

			this->PreviousBindText = IBToString(gb->Secondary, true);
			break;
		}
	}

	ibapi->StartCapturing();
}

void CBindSetterModal::OnClosing()
{
	CContext*      ctx   = CContext::GetContext();
	CInputBindApi* ibapi = ctx->GetInputBindApi();
	CGameBindsApi* gbapi = ctx->GetGameBindsApi();
	ibapi->EndCapturing();

	switch (this->GetResult())
	{
		/* New bind. */
		case EModalResult::Primary:
		{
			switch (this->Type)
			{
				case EBindEditType::Nexus:
				{
					if (!this->BindConflict.empty())
					{
						/* unset the bind that's currently using this wombo combo */
						ibapi->Set(this->BindConflict, InputBind_t{});
					}

					ibapi->Set(this->NexusBindID, this->Capture);
					break;
				}
				case EBindEditType::Game:
				{
					gbapi->Set(this->GameBindID, this->Capture, true, false);
					break;
				}
				case EBindEditType::Game2:
				{
					gbapi->Set(this->GameBindID, this->Capture, false, false);
					break;
				}
			}

			break;
		}
		/* Unbind. */
		case EModalResult::Secondary:
		{
			switch (this->Type)
			{
				case EBindEditType::Nexus:
				{
					ibapi->Set(this->NexusBindID, InputBind_t{});
					break;
				}
				case EBindEditType::Game:
				{
					gbapi->Set(this->GameBindID, InputBind_t{}, true, false);
					break;
				}
				case EBindEditType::Game2:
				{
					gbapi->Set(this->GameBindID, InputBind_t{}, false, false);
					break;
				}
			}

			break;
		}
	}

	/* Reset data. */
	this->Type = EBindEditType::None;
	this->NexusBindID.clear();
	this->GameBindID = (EGameBinds)0;
	this->PreviousBindText.clear();
	this->Capture = {};
	this->BindConflict.clear();
}

void CBindSetterModal::SetTarget(std::string aBindIdentifier)
{
	assert(this->Type == EBindEditType::None);

	this->Type = EBindEditType::Nexus;
	this->NexusBindID = aBindIdentifier;

	this->SetTitle();

	this->OpenModal();
}

void CBindSetterModal::SetTarget(EGameBinds aBindIdentifier, bool aIsPrimary)
{
	assert(this->Type == EBindEditType::None);

	this->Type = aIsPrimary
		? EBindEditType::Game
		: EBindEditType::Game2;
	this->GameBindID = aBindIdentifier;

	this->SetTitle();

	this->OpenModal();
}

void CBindSetterModal::SetTitle()
{
	CContext*      ctx   = CContext::GetContext();
	CUiContext*    uictx = ctx->GetUIContext();
	CLocalization* lang  = uictx->GetLocalization();

	/* Override the title before opening the modal. */
	std::string title = lang->Translate("((000062))");

	switch (this->Type)
	{
		case EBindEditType::Nexus:
		{
			title.append(lang->Translate(this->NexusBindID.c_str()));
			break;
		}
		case EBindEditType::Game:
		case EBindEditType::Game2:
		{
			title.append(lang->Translate(NameFrom(this->GameBindID).c_str()));
			break;
		}
	}

	this->SetDisplayName(title);
}
