#include "AddonsWindow.h"

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

namespace GUI
{
	float windowWidth = 620.0f;
	float windowHeight = 480.0f;
	float contentWidth = 540.0f;
	float contentHeight = 410.0f;

	AddonsWindow::AddonsWindow(std::string aName)
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

		TitleBarControlled = false;
		CloseHovered = false;

		TabIndex = 0;
		Tab1Hovered = false;
		Tab2Hovered = false;
	}

	void AddonsWindow::Render()
	{
		if (!Visible) { return; }

		if (!Background) { Background = TextureLoader::Get("TEX_ADDONS_BACKGROUND"); }
		if (!TitleBar) { TitleBar = TextureLoader::Get("TEX_ADDONS_TITLEBAR"); }
		if (!TitleBarHover) { TitleBarHover = TextureLoader::Get("TEX_ADDONS_TITLEBAR_HOVER"); }
		if (!TitleBarEnd) { TitleBarEnd = TextureLoader::Get("TEX_TITLEBAREND"); }
		if (!TitleBarEndHover) { TitleBarEndHover = TextureLoader::Get("TEX_TITLEBAREND_HOVER"); }
		if (!BtnClose) { BtnClose = TextureLoader::Get("TEX_BTNCLOSE"); }
		if (!BtnCloseHover) { BtnCloseHover = TextureLoader::Get("TEX_BTNCLOSE_HOVER"); }
		if (!TabBtn) { TabBtn = TextureLoader::Get("TEX_TABBTN"); }
		if (!TabBtnHover) { TabBtnHover = TextureLoader::Get("TEX_TABBTN_HOVER"); }

		if (!(
			Background &&
			TitleBar && TitleBarHover &&
			TitleBarEnd && TitleBarEndHover &&
			BtnClose && BtnCloseHover &&
			TabBtn && TabBtnHover
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

		ImGui::SetNextWindowSize(ImVec2(windowWidth * Renderer::Scaling, windowHeight * Renderer::Scaling));
		if (ImGui::Begin(Name.c_str(), &Visible, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | 
			(TitleBarControlled ? 0 : ImGuiWindowFlags_NoMove)))
		{
			float btnHeight = 22.0f * Renderer::Scaling;

			if (Background)
			{
				ImGui::SetCursorPos(ImVec2(0, 0));
				ImGui::Image(Background->Resource, ImVec2(windowWidth * Renderer::Scaling, windowHeight * Renderer::Scaling));
			}
			else
			{
				Background = TextureLoader::Get("TEX_ADDONS_BACKGROUND");
			}

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
			ImGui::TextColored(ImVec4(0.666f, 0.666f, 0.666f, 1.0f), "Installed");

			ImGui::SetCursorPos(ImVec2(tab2origin.x + text2offset.x, tab2origin.y + text2offset.y));
			ImGui::TextColored(ImVec4(0.666f, 0.666f, 0.666f, 1.0f), "Library");

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
							Loader::Mutex.lock();
							{
								for (auto& [path, addon] : Loader::Addons)
								{
									AddonItem(addon);
								}
							}
							Loader::Mutex.unlock();
						}

						ImGui::EndChild();
					}

					if (ImGui::GW2Button("Open Addons Folder", ImVec2(ImGui::CalcTextSize("Open Addons Folder").x + 16.0f, btnHeight)))
					{
						std::string strAddons = Path::D_GW2_ADDONS.string();
						ShellExecuteA(NULL, "explore", strAddons.c_str(), NULL, NULL, SW_SHOW);
					}
				}
				else if (TabIndex == 1)
				{
					ImVec2 windowSize = ImGui::GetWindowSize();
					ImVec2 textSize = ImGui::CalcTextSize("Unable to fetch addons.");
					ImVec2 position = ImGui::GetCursorPos();
					ImGui::SetCursorPos(ImVec2((position.x + (windowSize.x - textSize.x)) / 2, (position.y + (windowSize.y - textSize.y)) / 2));
					ImGui::TextColoredOutlined(ImVec4(0.666f, 0.666f, 0.666f, 1.0f), "Unable to fetch addons.");
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

		ImGui::PopStyleVar(6);
		ImGui::PopFont();
	}
}