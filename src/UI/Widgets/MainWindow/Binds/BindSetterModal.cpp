///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  BindSetterModal.cpp
/// Description  :  Modal for InputBind setter.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "BindSetterModal.h"

#include "Host/GW2/Context.h"
#include "Host/GW2/Inputs/GameBinds/GbConst.h"
#include "Engine/Inputs/InputBinds/IbConst.h"

namespace Raidcore::UI
{
	CBindSetterModal::CBindSetterModal()
	{
		this->SetID("BindSetterModal");
		this->SetDisplayName(Lang::Binds::SetKeybind);
	}

	void CBindSetterModal::RenderContent()
	{
		GW2::CContext*             ctx   = GW2::CContext::GetContext();
		Engine::Input::CInputBindApi* ibapi = ctx->GetInputBindApi();

		this->Capture = ibapi->GetCapture();

		/* Display current bind text, if no capture yet. */
		if (this->Capture == Engine::Input::Bind_t{})
		{
			ImGui::Text(this->PreviousBindText.c_str());
		}
		else
		{
			ImGui::Text(IBToString(this->Capture, true).c_str());
		}

		if (this->Type == UI::EBindEditType::Nexus)
		{
			this->BindConflict = ibapi->IsInUse(this->Capture);

			if (!this->BindConflict.empty() && this->BindConflict != this->NexusBindID)
			{
				ImGui::TextColored(ImVec4(255, 0, 0, 255), (Lang::Binds::WillOverwrite.Text() + this->BindConflict + ".").c_str());
			}
		}

		/* Unbind */
		if (ImGui::Button(Lang::Binds::Unbind.Text()))
		{
			this->SetResult(UI::EModalResult::Secondary);
		}

		ImGui::SameLine();

		/* Accept */
		if (ImGui::Button(Lang::Common::Accept.Text()))
		{
			this->SetResult(UI::EModalResult::Primary);
		}

		ImGui::SameLine();

		/* Cancel */
		if (ImGui::Button(Lang::Common::Cancel.Text()))
		{
			this->SetResult(UI::EModalResult::Cancel);
		}
	}

	void CBindSetterModal::OnOpening()
	{
		GW2::CContext*             ctx   = GW2::CContext::GetContext();
		Engine::Input::CInputBindApi* ibapi = ctx->GetInputBindApi();
		GW2::CGameBindsApi*        gbapi = ctx->GetGameBindsApi();

		/* Fetch display text. */
		switch (this->Type)
		{
			case UI::EBindEditType::Nexus:
			{
				const Engine::Input::Bind_t* ib = ibapi->Get(this->NexusBindID);

				assert(ib != nullptr);

				this->PreviousBindText = Engine::Input::IBToString(*ib, true);
				break;
			}
			case UI::EBindEditType::Game:
			{
				const GW2::MultiInputBind_t* gb = gbapi->Get(this->GameBindID);

				assert(gb != nullptr);

				this->PreviousBindText = Engine::Input::IBToString(gb->Primary, true);
				break;
			}
			case UI::EBindEditType::Game2:
			{
				const GW2::MultiInputBind_t* gb = gbapi->Get(this->GameBindID);

				assert(gb != nullptr);

				this->PreviousBindText = Engine::Input::IBToString(gb->Secondary, true);
				break;
			}
		}

		ibapi->StartCapturing();
	}

	void CBindSetterModal::OnClosing()
	{
		GW2::CContext*             ctx   = GW2::CContext::GetContext();
		Engine::Input::CInputBindApi* ibapi = ctx->GetInputBindApi();
		GW2::CGameBindsApi*        gbapi = ctx->GetGameBindsApi();
		ibapi->EndCapturing();

		switch (this->GetResult())
		{
			/* New bind. */
			case UI::EModalResult::Primary:
			{
				switch (this->Type)
				{
					case UI::EBindEditType::Nexus:
					{
						if (!this->BindConflict.empty())
						{
							/* unset the bind that's currently using this wombo combo */
							ibapi->Set(this->BindConflict, Engine::Input::Bind_t{});
						}

						ibapi->Set(this->NexusBindID, this->Capture);
						break;
					}
					case UI::EBindEditType::Game:
					{
						gbapi->Set(this->GameBindID, this->Capture, true, false);
						break;
					}
					case UI::EBindEditType::Game2:
					{
						gbapi->Set(this->GameBindID, this->Capture, false, false);
						break;
					}
				}

				break;
			}
			/* Unbind. */
			case UI::EModalResult::Secondary:
			{
				switch (this->Type)
				{
					case UI::EBindEditType::Nexus:
					{
						ibapi->Set(this->NexusBindID, Engine::Input::Bind_t{});
						break;
					}
					case UI::EBindEditType::Game:
					{
						gbapi->Set(this->GameBindID, Engine::Input::Bind_t{}, true, false);
						break;
					}
					case UI::EBindEditType::Game2:
					{
						gbapi->Set(this->GameBindID, Engine::Input::Bind_t{}, false, false);
						break;
					}
				}

				break;
			}
		}

		/* Reset data. */
		this->Type = UI::EBindEditType::None;
		this->NexusBindID.clear();
		this->GameBindID = (GW2::EGameBinds)0;
		this->PreviousBindText.clear();
		this->Capture = {};
		this->BindConflict.clear();
	}

	void CBindSetterModal::SetTarget(std::string aBindIdentifier)
	{
		assert(this->Type == UI::EBindEditType::None);

		this->Type = UI::EBindEditType::Nexus;
		this->NexusBindID = aBindIdentifier;

		this->SetTitle();

		this->OpenModal();
	}

	void CBindSetterModal::SetTarget(GW2::EGameBinds aBindIdentifier, bool aIsPrimary)
	{
		assert(this->Type == UI::EBindEditType::None);

		this->Type = aIsPrimary
			? UI::EBindEditType::Game
			: UI::EBindEditType::Game2;
		this->GameBindID = aBindIdentifier;

		this->SetTitle();

		this->OpenModal();
	}

	void CBindSetterModal::SetTitle()
	{
		GW2::CContext*      ctx   = GW2::CContext::GetContext();
		UI::CUiContext*    uictx = ctx->GetUIContext();
		UI::CLocalization* lang  = uictx->GetLocalization();

		/* Override the title before opening the modal. */
		std::string bindName;

		switch (this->Type)
		{
			case UI::EBindEditType::Nexus:
			{
				bindName = lang->Translate(this->NexusBindID.c_str());
				break;
			}
			case UI::EBindEditType::Game:
			case UI::EBindEditType::Game2:
			{
				bindName = lang->Translate(GW2::NameFrom(this->GameBindID).c_str());
				break;
			}
		}

		this->SetDisplayName(Lang::Binds::SetKeybind, bindName);
	}
}
