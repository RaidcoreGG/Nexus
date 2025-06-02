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
#include "Engine/Index/Index.h"
#include "Engine/Inputs/InputBinds/IbMapping.h"
#include "resource.h"
#include "GW2/Inputs/GameBinds/GbConst.h"
#include "Util/Time.h"

CBindsWindow::CBindsWindow()
{
	this->Name           = "Binds";
	this->DisplayName    = "((000060))";
	this->IconIdentifier = "ICON_BINDS";
	this->IconID         = RES_ICON_BINDS;
	this->IsHost         = true;

	this->Invalidate();
}

void CBindsWindow::RenderContent()
{
	CContext* ctx = CContext::GetContext();

	if (this->IsInvalid)
	{
		static CContext* ctx = CContext::GetContext();
		static CUiContext* uictx = ctx->GetUIContext();
		static CEscapeClosing* escclose = uictx->GetEscapeClosingService();

		escclose->Deregister(this->GetVisibleStatePtr());
		escclose->Register(this->GetNameID().c_str(), this->GetVisibleStatePtr());

		this->IBCategories = uictx->GetInputBinds();
		this->GIBCategories = uictx->GetGameBinds();

		this->IsInvalid = false;
	}

	ImGuiStyle& style = ImGui::GetStyle();

	float btnSz = ImGui::GetFontSize() * 1.5f;

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

	CLocalization* langApi = ctx->GetLocalization();

	if (ImGui::BeginTabBar("BindsTabBar", ImGuiTabBarFlags_None))
	{
		if (ImGui::BeginTabItem("Nexus"))
		{
			if (ImGui::BeginChild("Content", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f), false, ImGuiWindowFlags_NoBackground))
			{
				for (InputBindCategory_t& cat : this->IBCategories)
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
			ImGui::EndChild();

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Guild Wars 2"))
		{
			if (ImGui::BeginChild("Content", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f), false, ImGuiWindowFlags_NoBackground))
			{
				ImGui::Text("Info:");
				ImGui::TextWrapped("These binds are used by addons to emulate key presses for you in order to execute macros or similar.");
				ImGui::TextWrapped("The binds you set here should match the ones you use in-game.");

				static long long lastCheckedBinds = 0;
				static std::vector<std::filesystem::path> bindConfigs;

				if (ImGui::BeginCombo("##ImportGameBinds", "Import"))
				{
					long long now = Time::GetTimestamp();

					if (now - lastCheckedBinds > 10)
					{
						bindConfigs.clear();
						for (const std::filesystem::directory_entry entry : std::filesystem::directory_iterator(Index(EPath::DIR_GW2_INPUTBINDS)))
						{
							std::filesystem::path path = entry.path();

							/* sanity checks */
							if (std::filesystem::is_directory(path)) { continue; }
							if (std::filesystem::file_size(path) == 0) { continue; }
							if (path.extension() == ".xml")
							{
								bindConfigs.push_back(path);
							}
						}
						lastCheckedBinds = now;
					}

					for (std::filesystem::path& path : bindConfigs)
					{
						if (ImGui::Selectable(path.filename().string().c_str()))
						{
							/* Actually load the binds. */
							CGameBindsApi* gameBindsApi = ctx->GetGameBindsApi();
							gameBindsApi->Load(path);

							/* Trigger refresh for display binds. */
							CUiContext* uictx = ctx->GetUIContext();
							uictx->Invalidate();
						}
					}
					ImGui::EndCombo();
				}

				for (GameInputBindCategory_t& cat : this->GIBCategories)
				{
					if (ImGui::CollapsingHeader(langApi->Translate(cat.Name.c_str()), ImGuiTreeNodeFlags_DefaultOpen))
					{
						RenderGameInputBindsTable(cat.GameInputBinds);
					}
				}
			}
			ImGui::EndChild();

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}

void CBindsWindow::RenderSubWindows()
{
	if (this->BindSetterModal.Render() && this->BindSetterModal.GetResult() != EModalResult::None)
	{
		this->Invalidate();
	}
}

void CBindsWindow::RenderInputBindsTable(std::unordered_map<std::string, InputBindPacked_t>& aInputBinds)
{
	if (ImGui::BeginTable("table_inputbinds", 4, ImGuiTableFlags_BordersInnerH))
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
				this->BindSetterModal.SetTarget(identifier);
			}
			ImGui::PopID();

			ImGui::TableSetColumnIndex(2);
			if (ImGui::Checkbox(("##Passthrough_" + identifier).c_str(), &inputBind.Bind.Passthrough))
			{
				CInputBindApi* ibapi = ctx->GetInputBindApi();
				ibapi->SetPassthrough(identifier, inputBind.Bind.Passthrough);
			}
			ImGui::TooltipGeneric(langApi->Translate("((000115))"));

			ImGui::TableSetColumnIndex(3);
			/* FIXME: This accesses the DownAsync union field. */
			if (inputBind.Bind.Handler_DownOnlyAsync == nullptr)
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

void CBindsWindow::RenderGameInputBindsTable(std::unordered_map<EGameBinds, GameInputBindPacked_t>& aInputBinds)
{
	if (ImGui::BeginTable("table_gameinputbinds", 3, ImGuiTableFlags_BordersInnerH))
	{
		CContext* ctx = CContext::GetContext();
		CLocalization* langApi = ctx->GetLocalization();

		for (auto& [identifier, inputBind] : aInputBinds)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(langApi->Translate(NameFrom(identifier).c_str())); // get translation here

			ImGui::TableSetColumnIndex(1);
			ImGui::PushID((inputBind.Name +"##Primary").c_str());
			if (ImGui::Button(inputBind.KeysText.c_str(), ImVec2(ImGui::CalcTextSize("XXXXXXXXXXXXXXXXXXXXXXXX").x, 0.0f)))
			{
				this->BindSetterModal.SetTarget(identifier, true);
			}
			ImGui::PopID();

			ImGui::TableSetColumnIndex(2);
			ImGui::PushID((inputBind.Name + "##Secondary").c_str());
			if (ImGui::Button(inputBind.KeysText2.c_str(), ImVec2(ImGui::CalcTextSize("XXXXXXXXXXXXXXXXXXXXXXXX").x, 0.0f)))
			{
				this->BindSetterModal.SetTarget(identifier, false);
			}
			ImGui::PopID();
		}

		ImGui::EndTable();
	}
}

void CBindsWindow::DeleteStaleBind(const std::string& aIdentifier)
{
	if (aIdentifier.empty()) { return; }

	CContext*      ctx          = CContext::GetContext();
	CInputBindApi* inputBindApi = ctx->GetInputBindApi();

	inputBindApi->Delete(aIdentifier);
}
