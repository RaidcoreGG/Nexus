#include "CAddonsWindow.h"

#include <Windows.h>
#include <shellapi.h>

#include "Consts.h"
#include "Shared.h"
#include "Paths.h"
#include "State.h"
#include "Renderer.h"

#include "Loader/Loader.h"
#include "AddonItem.h"
#include "Textures/TextureLoader.h"

#include "imgui.h"
#include "imgui_extensions.h"

#include "resource.h"

namespace GUI
{
	float windowWidth = 620.0f;
	float windowHeight = 480.0f;
	float contentWidth = 540.0f;
	float contentHeight = 410.0f;

	bool showInstalled = false;
	bool refreshHovered = false;

	CAddonsWindow::CAddonsWindow(std::string aName)
	{
		Name = aName;

		Background = nullptr;
		TitleBar = nullptr;
		TitleBarHover = nullptr;
		TitleBarEnd = nullptr;
		TitleBarEndHover = nullptr;
		BtnClose = nullptr;
		BtnCloseHover = nullptr;
		TabBtn = nullptr;
		TabBtnHover = nullptr;
		BtnRefresh = nullptr;
		BtnRefreshHover = nullptr;

		TitleBarControlled = false;
		CloseHovered = false;

		TabIndex = 0;
		Tab1Hovered = false;
		Tab2Hovered = false;
	}

	void CAddonsWindow::Render()
	{
		if (!Visible) { return; }

		if (!Background) { Background = TextureLoader::GetOrCreate("TEX_ADDONS_BACKGROUND", RES_TEX_ADDONS_BACKGROUND, NexusHandle); }
		if (!TitleBar) { TitleBar = TextureLoader::GetOrCreate("TEX_ADDONS_TITLEBAR", RES_TEX_ADDONS_TITLEBAR, NexusHandle); }
		if (!TitleBarHover) { TitleBarHover = TextureLoader::GetOrCreate("TEX_ADDONS_TITLEBAR_HOVER", RES_TEX_ADDONS_TITLEBAR_HOVER, NexusHandle); }
		if (!TitleBarEnd) { TitleBarEnd = TextureLoader::GetOrCreate("TEX_TITLEBAREND", RES_TEX_TITLEBAREND, NexusHandle); }
		if (!TitleBarEndHover) { TitleBarEndHover = TextureLoader::GetOrCreate("TEX_TITLEBAREND_HOVER", RES_TEX_TITLEBAREND_HOVER, NexusHandle); }
		if (!BtnClose) { BtnClose = TextureLoader::GetOrCreate("TEX_BTNCLOSE", RES_TEX_BTNCLOSE, NexusHandle); }
		if (!BtnCloseHover) { BtnCloseHover = TextureLoader::GetOrCreate("TEX_BTNCLOSE_HOVER", RES_TEX_BTNCLOSE_HOVER, NexusHandle); }
		if (!TabBtn) { TabBtn = TextureLoader::GetOrCreate("TEX_TABBTN", RES_TEX_TABBTN, NexusHandle); }
		if (!TabBtnHover) { TabBtnHover = TextureLoader::GetOrCreate("TEX_TABBTN_HOVER", RES_TEX_TABBTN_HOVER, NexusHandle); }
		if (!BtnRefresh) { BtnRefresh = TextureLoader::GetOrCreate("TEX_BTNREFRESH", RES_TEX_BTNREFRESH, NexusHandle); }
		if (!BtnRefreshHover) { BtnRefreshHover = TextureLoader::GetOrCreate("TEX_BTNREFRESH_HOVER", RES_TEX_BTNREFRESH_HOVER, NexusHandle); }

		if (!(
			Background &&
			TitleBar && TitleBarHover &&
			TitleBarEnd && TitleBarEndHover &&
			BtnClose && BtnCloseHover &&
			TabBtn && TabBtnHover &&
			BtnRefresh && BtnRefreshHover
			))
		{
			return;
		}

		ImGui::PushFont(FontUI);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 4));
		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));

		ImGui::SetNextWindowSize(ImVec2(windowWidth * Renderer::Scaling, windowHeight * Renderer::Scaling));
		if (ImGui::Begin(Name.c_str(), &Visible, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | 
			(TitleBarControlled ? 0 : ImGuiWindowFlags_NoMove)))
		{
			float btnHeight = 22.0f * Renderer::Scaling;

			ImGui::SetCursorPos(ImVec2(0, 0));
			ImGui::Image(Background->Resource, ImVec2(windowWidth * Renderer::Scaling, windowHeight * Renderer::Scaling));

			ImGui::SetCursorPos(ImVec2(28.0f, 8.0f + (64.0f * Renderer::Scaling)));

			ImVec2 text1sz = ImGui::CalcTextSize("Installed");
			ImVec2 text2sz = ImGui::CalcTextSize("Library");
			ImVec2 text1offset = ImVec2(((92.0f * Renderer::Scaling) - text1sz.x) / 2, ((24.0f * Renderer::Scaling) - text1sz.y) / 2);
			ImVec2 text2offset = ImVec2(((92.0f * Renderer::Scaling) - text2sz.x) / 2, ((24.0f * Renderer::Scaling) - text2sz.y) / 2);
			
			ImVec2 tab1origin = ImGui::GetCursorPos(); // 28.0f, 28.0f

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

			if (ImGui::ImageButton(!Tab1Hovered ? TabBtn->Resource : TabBtnHover->Resource, ImVec2(92.0f * Renderer::Scaling, 24.0f * Renderer::Scaling)))
			{
				TabIndex = 0;
			}
			Tab1Hovered = ImGui::IsItemHovered();

			ImGui::SameLine();

			ImVec2 tab2origin = ImGui::GetCursorPos();

			if (ImGui::ImageButton(!Tab2Hovered ? TabBtn->Resource : TabBtnHover->Resource, ImVec2(92.0f * Renderer::Scaling, 24.0f * Renderer::Scaling)))
			{
				TabIndex = 1;
			}
			Tab2Hovered = ImGui::IsItemHovered();

			ImGui::PopStyleColor(3);
			ImGui::PopStyleVar();

			ImGui::SetCursorPos(ImVec2(tab1origin.x + text1offset.x, tab1origin.y + text1offset.y));
			ImGui::TextColored(TabIndex == 0 ? ImVec4(1, 1, 1, 1) : ImVec4(0.666f, 0.666f, 0.666f, 1.0f), "Installed");

			ImGui::SetCursorPos(ImVec2(tab2origin.x + text2offset.x, tab2origin.y + text2offset.y));
			ImGui::TextColored(TabIndex == 1 ? ImVec4(1, 1, 1, 1) : ImVec4(0.666f, 0.666f, 0.666f, 1.0f), "Library");

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 0.f }); // smol checkbox
			ImGui::SetCursorPos(ImVec2(contentWidth * Renderer::Scaling - Renderer::Scaling * 24.0f, tab1origin.y));
			if (ImGui::ImageButton(!refreshHovered ? BtnRefresh->Resource : BtnRefreshHover->Resource, ImVec2(Renderer::Scaling * 24.0f, Renderer::Scaling * 24.0f)))
			{
				Loader::NotifyChanges();
			}
			refreshHovered = ImGui::IsItemHovered();
			ImGui::PopStyleVar();
			ImGui::PopStyleColor(3);

			ImGui::SetCursorPos(ImVec2(28.0f, 32.0f + (64.0f * Renderer::Scaling)));
			{
				ImGui::BeginChild("##AddonsWindowContent", ImVec2(contentWidth * Renderer::Scaling, (contentHeight * Renderer::Scaling) - (64.0f * Renderer::Scaling)));

				if (TabIndex == 0)
				{
					{
						ImGui::BeginChild("##AddonTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), (btnHeight * 1.5f) * -1));

						if (Loader::Addons.size() == 0)
						{
							ImVec2 windowSize = ImGui::GetWindowSize();
							ImVec2 textSize = ImGui::CalcTextSize("No addons installed.");
							ImVec2 position = ImGui::GetCursorPos();
							ImGui::SetCursorPos(ImVec2((position.x + (windowSize.x - textSize.x)) / 2, (position.y + (windowSize.y - textSize.y)) / 2));
							ImGui::TextColoredOutlined(ImVec4(0.666f, 0.666f, 0.666f, 1.0f), "No addons installed.");
						}
						else
						{
							const std::lock_guard<std::mutex> lock(Loader::Mutex);
							{
								for (auto& [path, addon] : Loader::Addons)
								{
									if (path.filename() == "arcdps_integration64.dll") { continue; }
									AddonItem(addon);
								}
							}
						}

						ImGui::EndChild();
					}

					if (ImGui::GW2::Button("Open Addons Folder", ImVec2(ImGui::CalcTextSize("Open Addons Folder").x + 16.0f, btnHeight)))
					{
						std::string strAddons = Path::D_GW2_ADDONS.string();
						ShellExecuteA(NULL, "explore", strAddons.c_str(), NULL, NULL, SW_SHOW);
					}
					ImGui::SameLine();
					if (ImGui::GW2::Button("Check for Updates", ImVec2(ImGui::CalcTextSize("Check for Updates").x + 16.0f, btnHeight)))
					{
						const std::lock_guard<std::mutex> lock(Loader::Mutex);
						{
							for (auto& [path, addon] : Loader::Addons)
							{
								if (nullptr == addon->Definitions) { continue; }

								std::filesystem::path tmpPath = path.string();
								signed int tmpSig = addon->Definitions->Signature;
								std::string tmpName = addon->Definitions->Name;
								AddonVersion tmpVers = addon->Definitions->Version;
								EUpdateProvider tmpProv = addon->Definitions->Provider;
								std::string tmpLink = addon->Definitions->UpdateLink != nullptr ? addon->Definitions->UpdateLink : "";

								std::thread([tmpPath, tmpSig, tmpName, tmpVers, tmpProv, tmpLink]()
									{
										if (Loader::UpdateAddon(tmpPath, tmpSig, tmpName, tmpVers, tmpProv, tmpLink))
										{
											Loader::QueueAddon(ELoaderAction::Reload, tmpPath);
										}
									})
									.detach();
							}
						}
					}
					ImGui::TooltipGeneric("Checks each addon and updates it, if available.\nSome addons require a restart to apply the update and won't take effect immediately.");
				}
				else if (TabIndex == 1)
				{
					{
						ImGui::BeginChild("##AddonTabScroll", ImVec2(ImGui::GetWindowContentRegionWidth(), (btnHeight * 1.5f) * -1));

						int downloadable = 0;
						const std::lock_guard<std::mutex> lockLoader(Loader::Mutex);
						if (Loader::AddonLibrary.size() != 0)
						{
							for (auto& libAddon : Loader::AddonLibrary)
							{
								bool exists = false;
								{
									for (auto& [path, addon] : Loader::Addons)
									{
										if (addon->Definitions != nullptr && addon->Definitions->Signature == libAddon->Signature)
										{
											exists = true;
											break;
										}
									}
								}
								if (false == exists || true == showInstalled)
								{
									AddonItem(libAddon, exists);
									downloadable++;
								}
							}
						}
						
						if (Loader::AddonLibrary.size() == 0 || downloadable == 0)
						{
							ImVec2 windowSize = ImGui::GetWindowSize();
							ImVec2 textSize = ImGui::CalcTextSize("There's nothing here.\nMaybe ask your favourite developer to support Nexus!");
							ImVec2 position = ImGui::GetCursorPos();
							ImGui::SetCursorPos(ImVec2((position.x + (windowSize.x - textSize.x)) / 2, (position.y + (windowSize.y - textSize.y)) / 2));
							ImGui::TextColoredOutlined(ImVec4(0.666f, 0.666f, 0.666f, 1.0f), "There's nothing here.\nMaybe ask your favourite developer to support Nexus!");
						}

						ImGui::EndChild();
					}

					ImGui::Checkbox("Show already installed", &showInstalled);
				}

				ImGui::EndChild();
			}

			ImGui::SetCursorPos(ImVec2(0, 0));

			ImGui::Image(TitleBarControlled ? TitleBarHover->Resource : TitleBar->Resource, ImVec2(Renderer::Scaling * 600.0f, Renderer::Scaling * 64.0f));
			bool barHov = ImGui::IsItemHovered();

			ImGui::SetCursorPos(ImVec2((windowWidth * Renderer::Scaling) - (Renderer::Scaling * 128.0f), 0));
			ImGui::Image(TitleBarControlled ? TitleBarEndHover->Resource : TitleBarEnd->Resource, ImVec2(Renderer::Scaling * 128.0f, Renderer::Scaling * 64.0f));
			bool endHov = ImGui::IsItemHovered();

			TitleBarControlled = barHov || endHov;

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

			ImGui::SetCursorPos(ImVec2(((windowWidth - 20.0f) * Renderer::Scaling) - (Renderer::Scaling * 32.0f), 15.0f * Renderer::Scaling));
			if (ImGui::ImageButton(CloseHovered ? BtnCloseHover->Resource : BtnClose->Resource, ImVec2(Renderer::Scaling * 32.0f, Renderer::Scaling * 32.0f)))
			{
				Visible = false;
			}
			CloseHovered = ImGui::IsItemHovered();

			ImGui::PopStyleColor(3);
			ImGui::PopStyleVar();

			ImGui::PushFont(FontBig);
			ImGui::SetCursorPos(ImVec2(28.0f, 20.0f * Renderer::Scaling));
			ImGui::TextColored(ImVec4(1.0f, .933f, .733f, 1.0f), Name.c_str());
			ImGui::PopFont();
		}
		ImGui::End();

		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar(6);
		ImGui::PopFont();
	}
}