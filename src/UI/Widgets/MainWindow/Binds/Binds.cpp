///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Binds.cpp
/// Description  :  Contains the content of the binds window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Binds.h"

#include <map>

#include "imgui/imgui.h"
#include "imgui_extensions.h"

#include "Consts.h"
#include "Context.h"
#include "Inputs/InputBinds/ManagedInputBind.h"
#include "Renderer.h"
#include "resource.h"

constexpr ImGuiWindowFlags ModalFlags = ImGuiWindowFlags_AlwaysAutoResize |
									    ImGuiWindowFlags_NoResize         |
									    ImGuiWindowFlags_NoCollapse;

CBindsWindow::CBindsWindow()
{
	this->Name           = "Binds";
	this->IconIdentifier = "ICON_BINDS";
	this->IconID         = RES_ICON_BINDS;
	this->IsHost         = true;

	this->Invalidate();
}

void CBindsWindow::RenderContent()
{
	if (this->IsInvalid)
	{
		this->PopulateInputBinds();
		this->PopulateGameInputBinds();
		this->IsInvalid = false;
	}

	/* controls whether to display the nexus or game binds */
	static bool isShowingNexusBinds = true;

	ImGuiStyle& style = ImGui::GetStyle();

	float fontScaleFactor = ImGui::GetFontSize() / 16.0f;
	float btnSz = 28.0f * fontScaleFactor * 0.75f;

	float width = ImGui::GetContentRegionAvailWidth();
	float btnWidth;
	if (!this->IsPoppedOut)
	{
		btnWidth = ((width - style.ItemSpacing.x - btnSz) / 2) - style.ItemSpacing.x;
	}
	else
	{
		btnWidth = (width - style.ItemSpacing.x) / 2;
	}

	ImVec2 initial = ImGui::GetCursorPos();

	CContext* ctx = CContext::GetContext();
	CLocalization* langApi = ctx->GetLocalization();

	ImGui::SetCursorPos(initial);
	if (ImGui::Button("Nexus", ImVec2(btnWidth, ImGui::GetTextLineHeight() * 2)))
	{
		isShowingNexusBinds = true;
	}

	ImGui::SetCursorPos(ImVec2(initial.x + btnWidth + style.ItemSpacing.x, initial.y));
	if (ImGui::Button("Guild Wars 2", ImVec2(btnWidth, ImGui::GetTextLineHeight() * 2)))
	{
		isShowingNexusBinds = false;
	}

	if (ImGui::BeginChild("##BindsScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f)))
	{
		if (isShowingNexusBinds)
		{
			for (InputBindCategory cat : this->IBCategories)
			{
				if (ImGui::CollapsingHeader(
					cat.Name != NULLSTR
					? cat.Name.c_str()
					: langApi->Translate("((000088))"),
					ImGuiTreeNodeFlags_DefaultOpen))
				{
					RenderInputBindsTable(cat.InputBinds);
				}
			}
		}
		else
		{
			ImGui::Text("Info:");
			ImGui::TextWrapped("These binds are used by addons to emulate key presses for you in order to execute macros or similar.");
			ImGui::TextWrapped("The binds you set here should match the ones you use in-game.");

			for (GameInputBindCategory cat : this->GIBCategories)
			{
				if (ImGui::CollapsingHeader(langApi->Translate(cat.Name.c_str()), ImGuiTreeNodeFlags_DefaultOpen))
				{
					RenderGameInputBindsTable(cat.GameInputBinds);
				}
			}
		}
	}
	ImGui::EndChild();
}

void CBindsWindow::RenderSubWindows()
{
	DrawBindSetterModal();
}

void CBindsWindow::RenderInputBindsTable(const std::map<std::string, InputBindPacked>& aInputBinds)
{
	if (ImGui::BeginTable("table_inputbinds", 3, ImGuiTableFlags_BordersInnerH))
	{
		CContext* ctx = CContext::GetContext();
		CLocalization* langApi = ctx->GetLocalization();

		for (auto& [identifier, inputBind] : aInputBinds)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(langApi->Translate(identifier.c_str()));

			ImGui::TableSetColumnIndex(1);
			ImGui::PushID(identifier.c_str());
			if (ImGui::Button(inputBind.KeysText.c_str(), ImVec2(ImGui::CalcTextSize("XXXXXXXXXXXXXXXXXXXXXXXX").x, 0.0f)))
			{
				this->Editing_Identifier = identifier;
				this->Editing_BindText = inputBind.KeysText;

				this->OpenModalNextFrame = true;
				this->IsEditing = EBindEditType::Nexus;

				this->ModalTitle = langApi->Translate("((000062))");
				this->ModalTitle.append(langApi->Translate(this->Editing_Identifier.c_str()));
				this->ModalTitle.append("###inputbind_setter_modal");
			}
			ImGui::PopID();

			ImGui::TableSetColumnIndex(2);
			if (inputBind.Bind.Handler == nullptr)
			{
				if (ImGui::SmallButton(("X##" + identifier).c_str()))
				{
					DeleteStaleBind(identifier);
				}
				ImGui::SameLine();
				ImGui::TextDisabled(langApi->Translate("((000061))"));
			}
		}

		ImGui::EndTable();
	}
}

void CBindsWindow::RenderGameInputBindsTable(const std::map<EGameBinds, GameInputBindPacked>& aInputBinds)
{
	if (ImGui::BeginTable("table_gameinputbinds", 2, ImGuiTableFlags_BordersInnerH))
	{
		CContext* ctx = CContext::GetContext();
		CLocalization* langApi = ctx->GetLocalization();

		for (auto& [identifier, inputBind] : aInputBinds)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(langApi->Translate(CGameBindsApi::ToString(identifier).c_str())); // get translation here

			ImGui::TableSetColumnIndex(1);
			ImGui::PushID(inputBind.Name.c_str());
			if (ImGui::Button(inputBind.KeysText.c_str(), ImVec2(ImGui::CalcTextSize("XXXXXXXXXXXXXXXXXXXXXXXX").x, 0.0f)))
			{
				this->Editing_GameIdentifier = identifier;
				this->Editing_BindText = inputBind.KeysText;

				this->OpenModalNextFrame = true;
				this->IsEditing = EBindEditType::Game;

				this->ModalTitle = langApi->Translate("((000062))");
				this->ModalTitle.append(langApi->Translate(CGameBindsApi::ToString(this->Editing_GameIdentifier).c_str()));
				this->ModalTitle.append("###inputbind_setter_modal");
			}
			ImGui::PopID();
		}

		ImGui::EndTable();
	}
}

void CBindsWindow::PopulateInputBinds()
{
	this->IBCategories.clear();

	CContext* ctx = CContext::GetContext();
	CInputBindApi* inputBindApi = ctx->GetInputBindApi();

	/* copy of all InputBinds */
	std::map<std::string, ManagedInputBind> InputBindRegistry = inputBindApi->GetRegistry();

	/* acquire categories */
	for (auto& [identifier, inputBind] : InputBindRegistry)
	{
		std::string owner = Loader::GetOwner(inputBind.Handler);

		auto it = std::find_if(this->IBCategories.begin(), this->IBCategories.end(), [owner](InputBindCategory category) { return category.Name == owner; });

		if (it == this->IBCategories.end())
		{
			InputBindCategory cat{};
			cat.Name = owner;
			cat.InputBinds[identifier] =
			{
				CInputBindApi::IBToString(inputBind.Bind, true),
				inputBind
			};
			this->IBCategories.push_back(cat);
		}
		else
		{
			it->InputBinds[identifier] =
			{
				CInputBindApi::IBToString(inputBind.Bind, true),
				inputBind
			};
		}
	}
}

void CBindsWindow::PopulateGameInputBinds()
{
	this->GIBCategories.clear();

	CContext* ctx = CContext::GetContext();
	CGameBindsApi* gameBindsApi = ctx->GetGameBindsApi();

	/* copy of all InputBinds */
	std::map<EGameBinds, InputBind> InputBindRegistry = gameBindsApi->GetRegistry();

	/* acquire categories */
	for (auto& [identifier, inputBind] : InputBindRegistry)
	{
		std::string catName = CGameBindsApi::GetCategory(identifier);

		auto it = std::find_if(this->GIBCategories.begin(), this->GIBCategories.end(), [catName](GameInputBindCategory category) { return category.Name == catName; });

		if (it == this->GIBCategories.end())
		{
			GameInputBindCategory cat{};
			cat.Name = catName;
			cat.GameInputBinds[identifier] =
			{
				CGameBindsApi::ToString(identifier),
				CInputBindApi::IBToString(inputBind, true),
				inputBind
			};
			this->GIBCategories.push_back(cat);
		}
		else
		{
			it->GameInputBinds[identifier] =
			{
				CGameBindsApi::ToString(identifier),
				CInputBindApi::IBToString(inputBind, true),
				inputBind
			};
		}
	}
}

void CBindsWindow::DrawBindSetterModal()
{
	if (this->OpenModalNextFrame == true)
	{
		ImGui::OpenPopup(this->ModalTitle.c_str());
		this->OpenModalNextFrame = false;
	}

	ImVec2 center(Renderer::Width * 0.5f, Renderer::Height * 0.5f);
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal(this->ModalTitle.c_str(), NULL, ModalFlags))
	{
		CContext* ctx = CContext::GetContext();
		CLocalization* langApi = ctx->GetLocalization();
		CInputBindApi* inputBindApi = ctx->GetInputBindApi();
		CGameBindsApi* gameBindsApi = ctx->GetGameBindsApi();

		inputBindApi->StartCapturing();

		InputBind currInputBind = inputBindApi->GetCapturedInputBind();
		std::string usedBy = inputBindApi->IsInUse(currInputBind);

		if (currInputBind == InputBind{})
		{
			ImGui::Text(this->Editing_BindText.c_str());
		}
		else
		{
			ImGui::Text(CInputBindApi::IBToString(currInputBind, true).c_str());
		}

		bool overwriting = false;

		if (usedBy != this->Editing_Identifier && !usedBy.empty())
		{
			ImGui::TextColored(ImVec4(255, 0, 0, 255), (langApi->Translate("((000063))") + usedBy + ".").c_str());
			overwriting = true;
		}

		bool close = false;

		if (ImGui::Button(langApi->Translate("((000064))")))
		{
			if (this->IsEditing == EBindEditType::Nexus)
			{
				inputBindApi->Set(this->Editing_Identifier, InputBind{});
			}
			else if (this->IsEditing == EBindEditType::Game)
			{
				gameBindsApi->Set(this->Editing_GameIdentifier, InputBind{});
			}

			this->Invalidate();

			close = true;
		}
		ImGui::SameLine();
		if (ImGui::Button(langApi->Translate("((000065))")))
		{
			if (this->IsEditing == EBindEditType::Nexus)
			{
				if (overwriting)
				{
					/* unset the bind that's currently using this wombo combo */
					inputBindApi->Set(usedBy, InputBind{});
				}

				inputBindApi->Set(this->Editing_Identifier, currInputBind);
			}
			else if (this->IsEditing == EBindEditType::Game)
			{
				gameBindsApi->Set(this->Editing_GameIdentifier, currInputBind);
			}

			this->Invalidate();

			close = true;
		}
		ImGui::SameLine();
		if (ImGui::Button(langApi->Translate("((000066))")))
		{
			close = true;
		}

		if (close)
		{
			/* unset all editing vars */
			this->Editing_Identifier = "";
			this->Editing_GameIdentifier = (EGameBinds)-1;
			this->Editing_BindText = "";
			this->IsEditing = EBindEditType::None;

			inputBindApi->EndCapturing();
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void CBindsWindow::DeleteStaleBind(const std::string& aIdentifier)
{
	if (aIdentifier.empty()) { return; }

	std::thread([this, aIdentifier]()
	{
		CContext* ctx = CContext::GetContext();
		CInputBindApi* inputBindApi = ctx->GetInputBindApi();

		inputBindApi->Delete(aIdentifier);
		this->Invalidate();
	}).detach();
}
