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
#include "imgui/imgui_extensions.h"

#include "Host/GW2/Context.h"
#include "Host/GW2/Index/Index.h"
#include "Engine/Inputs/InputBinds/IbMapping.h"
#include "Host/GW2/Inputs/GameBinds/GbConst.h"
#include "res/Resources.h"
#include "Util/Time.h"

namespace Raidcore::UI
{
	CBindsWindow::CBindsWindow()
	{
		this->Name           = "Binds";
		this->DisplayName    = Lang::Binds::Keybinds;
		this->IconIdentifier = "ICON_BINDS";
		this->IconEntry      = &Resources::IconBinds;
		this->IsHost         = true;

		this->Invalidate();
	}

	void CBindsWindow::RenderContent()
	{
		GW2::CContext*       ctx      = GW2::CContext::GetContext();
		UI::CUiContext*     uictx    = ctx->GetUIContext();
		UI::CEscapeClosing* escclose = uictx->GetEscapeClosingService();
		UI::CLocalization*  langApi  = uictx->GetLocalization();

		if (this->IsInvalid)
		{
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

		if (ImGui::BeginTabBar("BindsTabBar", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem("Nexus"))
			{
				if (ImGui::BeginChild("Content", ImVec2(ImGui::GetWindowContentRegionWidth(), 0.0f), false, ImGuiWindowFlags_NoBackground))
				{
					for (UI::InputBindCategory_t& cat : this->IBCategories)
					{
						if (ImGui::CollapsingHeader(
							cat.Name != "(null)"
							? cat.Name.c_str()
							: Lang::Binds::Inactive.Text(),
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
					ImGui::TextUnformatted(Lang::Binds::GameBindsInfo.Text());
					ImGui::TextWrapped("%s", Lang::Binds::GameBindsUsage.Text());
					ImGui::TextWrapped("%s", Lang::Binds::GameBindsMatch.Text());

					static long long lastCheckedBinds = 0;
					static std::vector<std::filesystem::path> bindConfigs;

					if (ImGui::BeginCombo("##ImportGameBinds", Lang::Binds::Import.Text()))
					{
						long long now = Time::GetTimestamp();

						if (now - lastCheckedBinds > 10)
						{
							/* Create binds directory. */
							if (!std::filesystem::exists(GW2::Index(GW2::EPath::DIR_GW2_INPUTBINDS)))
							{
								std::filesystem::create_directories(GW2::Index(GW2::EPath::DIR_GW2_INPUTBINDS));
							}

							bindConfigs.clear();

							for (const std::filesystem::directory_entry entry : std::filesystem::directory_iterator(GW2::Index(GW2::EPath::DIR_GW2_INPUTBINDS)))
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
								GW2::CGameBindsApi* gameBindsApi = ctx->GetGameBindsApi();
								gameBindsApi->Load(path);

								/* Trigger refresh for display binds. */
								UI::CUiContext* uictx = ctx->GetUIContext();
								uictx->Invalidate();
							}
						}
						ImGui::EndCombo();
					}

					for (UI::GameInputBindCategory_t& cat : this->GIBCategories)
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
		if (this->BindSetterModal.Render() && this->BindSetterModal.GetResult() != UI::EModalResult::None)
		{
			this->Invalidate();
		}
	}

	void CBindsWindow::RenderInputBindsTable(std::unordered_map<std::string, UI::InputBindPacked_t>& aInputBinds)
	{
		if (ImGui::BeginTable("table_inputbinds", 4, ImGuiTableFlags_BordersInnerH))
		{
			GW2::CContext*      ctx     = GW2::CContext::GetContext();
			UI::CUiContext*    uictx   = ctx->GetUIContext();
			UI::CLocalization* langApi = uictx->GetLocalization();

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
					Engine::Input::CInputBindApi* ibapi = ctx->GetInputBindApi();
					ibapi->SetPassthrough(identifier, inputBind.Bind.Passthrough);
				}
				ImGui::TooltipGeneric(Lang::Binds::PassThrough.Text());

				ImGui::TableSetColumnIndex(3);
				/* FIXME: This accesses the DownAsync union field. */
				if (inputBind.Bind.Handler_DownOnlyAsync == nullptr)
				{
					if (ImGui::SmallButton(("X##" + identifier).c_str()))
					{
						DeleteStaleBind(identifier);
					}
					ImGui::SameLine();
					ImGui::TextDisabled(Lang::Binds::NotInUse.Text());
				}
			}

			ImGui::EndTable();
		}
	}

	void CBindsWindow::RenderGameInputBindsTable(std::unordered_map<GW2::EGameBinds, UI::GameInputBindPacked_t>& aInputBinds)
	{
		if (ImGui::BeginTable("table_gameinputbinds", 3, ImGuiTableFlags_BordersInnerH))
		{
			GW2::CContext*      ctx     = GW2::CContext::GetContext();
			UI::CUiContext*    uictx   = ctx->GetUIContext();
			UI::CLocalization* langApi = uictx->GetLocalization();

			for (auto& [identifier, inputBind] : aInputBinds)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(langApi->Translate(GW2::NameFrom(identifier).c_str())); // get translation here

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

		GW2::CContext*             ctx          = GW2::CContext::GetContext();
		Engine::Input::CInputBindApi* inputBindApi = ctx->GetInputBindApi();

		inputBindApi->Delete(aIdentifier);
	}
}
