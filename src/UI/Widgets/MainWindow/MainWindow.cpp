///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  MainWindow.cpp
/// Description  :  Contains the logic for the main Nexus UI window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "MainWindow.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "ImAnimate/ImAnimate.h"

#include "Context.h"
#include "resource.h"

constexpr ImGuiWindowFlags Flags = ImGuiWindowFlags_NoTitleBar  |
								   ImGuiWindowFlags_NoCollapse  |
								   ImGuiWindowFlags_NoScrollbar;

void CMainWindow::AddWindow(ISubWindow* aWindow)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);
	
	if (std::find(this->Windows.begin(), this->Windows.end(), aWindow) == this->Windows.end())
	{
		this->Windows.push_back(aWindow);
	}
}

void CMainWindow::RemoveWindow(ISubWindow* aWindow)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = std::find(this->Windows.begin(), this->Windows.end(), aWindow);

	if (it != this->Windows.end())
	{
		this->Windows.erase(it);
	}
}

void CMainWindow::Render()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	for (ISubWindow* window : this->Windows)
	{
		/* render popout windows */
		if (window->IsPopOut())
		{
			window->Render();
		}

		/* render indepentent children of windows*/
		if (window->HasSubWindows())
		{
			window->RenderSubWindows();
		}
	}

	/* don't continue if not visible */
	if (!this->IsVisible)
	{
		return;
	}

	/* restore active content */
	if (this->ActiveContent && this->ActiveContent->IsPopOut())
	{
		this->ActiveContent = nullptr;
	}

	ImGuiStyle& style = ImGui::GetStyle();
	ImVec2 padding = style.WindowPadding;

	float fontScaleFactor = ImGui::GetTextLineHeight() / 16.f;
	float headerHeight = ImGui::GetTextLineHeight() * 2;
	float footerHeight = ImGui::GetTextLineHeight() * 1.5f;
	float sidebarWidth_Collapsed = (padding.x * 2) + headerHeight;
	float sidebarWidth_Expanded = 300.f * fontScaleFactor;

	bool poppedPadding = false;

	float minWidth = 1000.f * fontScaleFactor;
	float minHeight = (600.f * fontScaleFactor) + headerHeight;

	/* render main window */
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(minWidth, minHeight), ImGuiCond_FirstUseEver);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(minWidth * .5f, minHeight * .5f));
	if (ImGui::Begin("Nexus", (bool*)0, Flags | (this->IsHandleHeld ? 0 : ImGuiWindowFlags_NoMove)))
	{
		float wndWidth = ImGui::GetWindowWidth();
		float contentHeight = ImGui::GetWindowHeight() - headerHeight;
		float contentWidth = wndWidth - sidebarWidth_Collapsed;

		ImGui::PopStyleVar(2);
		poppedPadding = true;

		static float renderedSidebarWidth = sidebarWidth_Collapsed;
		static float navTextAlpha = 0;
		if (this->IsSidebarActive)
		{
			ImGui::Animate(sidebarWidth_Collapsed, sidebarWidth_Expanded, 500, &renderedSidebarWidth, ImAnimate::ECurve::InOutCubic);
			
			if (renderedSidebarWidth > sidebarWidth_Expanded * .5f)
			{
				ImGui::Animate(0, 1, 250, &navTextAlpha, ImAnimate::ECurve::InOutCubic);
			}
		}
		else
		{
			ImGui::Animate(sidebarWidth_Expanded, sidebarWidth_Collapsed, 500, &renderedSidebarWidth, ImAnimate::ECurve::InOutCubic);
			ImGui::Animate(1, 0, 250, &navTextAlpha, ImAnimate::ECurve::InOutCubic);
		}

		ImVec4 navTextCol = style.Colors[ImGuiCol_Text];
		navTextCol.w = navTextAlpha;

		ImVec2 sidebarP1 = ImGui::GetWindowPos();
		sidebarP1.y += headerHeight;
		ImVec2 sidebarP2 = ImVec2(renderedSidebarWidth + sidebarP1.x, contentHeight + sidebarP1.y);
		ImVec2 contentP1 = ImVec2(sidebarP2.x, sidebarP1.y);
		ImVec2 contentP2 = ImVec2(sidebarP2.x + wndWidth - renderedSidebarWidth, sidebarP2.y);

		ImGui::PushClipRect(contentP1, contentP2, false);
		ImGui::SetCursorPos(ImVec2(sidebarWidth_Collapsed, headerHeight));
		if (ImGui::BeginChild("Nexus##mainwindow_content", ImVec2(contentWidth, contentHeight - footerHeight)))
		{
			ImGui::SetCursorPos(padding);
			if (ImGui::BeginChild("Nexus##mainwindow_contentinner", ImVec2(contentWidth - (padding.x * 2), contentHeight - footerHeight - (padding.y * 2))))
			{
				if (this->ActiveContent)
				{
					this->ActiveContent->Render();
				}
				else
				{
					this->RenderDashboard();
				}
			}
			ImGui::EndChild();
		}
		ImGui::EndChild();

		ImGui::SetCursorPos(ImVec2(sidebarWidth_Collapsed, headerHeight + contentHeight - footerHeight));
		if (ImGui::BeginChild("Nexus##mainwindow_footer", ImVec2(contentWidth, footerHeight), false, Flags))
		{
			ImGui::SetCursorPos(ImVec2(0, 0));
			ImGui::Separator();

			ImGui::SetCursorPos(ImVec2((contentWidth - ImGui::CalcTextSize(u8"© 2021 - 2024 Raidcore").x) / 2 - sidebarWidth_Collapsed, (footerHeight - ImGui::GetFontSize()) / 2));
			ImGui::Text(u8"© 2021 - 2024 Raidcore");
		}
		ImGui::EndChild();
		ImGui::PopClipRect();

		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_TitleBgCollapsed));
		ImGui::SetCursorPos(ImVec2(0, headerHeight));
		if (ImGui::BeginChild("Nexus##mainwindow_sidebar", ImVec2(renderedSidebarWidth, contentHeight)))
		{
			ImVec2 navItemSz = ImVec2(renderedSidebarWidth - (padding.x * 2), headerHeight);
			float navX = padding.x;
			float navY = padding.y;

			/* Dashboard Nav Item */
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
				ImGui::SetCursorPos(ImVec2(navX, navY));
				if (ImGui::Selectable("##Dashboard",
					this->ActiveContent == nullptr,
					0,
					navItemSz))
				{
					this->ActiveContent = nullptr;
				}
				ImGui::PopStyleVar();

				if (this->Tex_DashboardIcon)
				{
					ImGui::SetCursorPos(ImVec2(navX, navY));
					ImGui::Image(Tex_DashboardIcon->Resource, ImVec2(navItemSz.y, navItemSz.y));
				}
				else
				{
					CContext* ctx = CContext::GetContext();
					this->Tex_DashboardIcon = ctx->GetTextureService()->GetOrCreate("ICON_DASHBOARD", RES_ICON_DASHBOARD, ctx->GetModule());
				}

				ImGui::SetCursorPos(ImVec2(navX + navItemSz.y + padding.x, navY + ((navItemSz.y - ImGui::GetTextLineHeight()) / 2)));
				ImGui::TextColored(navTextCol, "Dashboard");

				navY += navItemSz.y + padding.y;
			}
			
			/* Dynamic Nav Items */
			for (ISubWindow* window : this->Windows)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
				ImGui::SetCursorPos(ImVec2(navX, navY));
				if (ImGui::Selectable(("##" + window->GetName()).c_str(),
					this->ActiveContent && this->ActiveContent->GetName() == window->GetName(),
					0,
					navItemSz))
				{
					if (!window->IsPopOut())
					{
						this->ActiveContent = window;
					}
					else
					{
						ImGui::SetWindowFocus(window->GetName().c_str());
					}
				}
				ImGui::PopStyleVar();

				const Texture* icon = window->GetIcon();
				if (icon)
				{
					ImGui::SetCursorPos(ImVec2(navX, navY));
					ImGui::Image(icon->Resource, ImVec2(navItemSz.y, navItemSz.y));
				}

				ImGui::SetCursorPos(ImVec2(navX + navItemSz.y + padding.x, navY + ((navItemSz.y - ImGui::GetTextLineHeight()) / 2)));
				ImGui::TextColored(navTextCol, window->GetName().c_str());

				navY += navItemSz.y + padding.y;
			}

			ImVec4 colDisabled = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
			colDisabled.w = navTextAlpha;
			float lineHeight = ImGui::GetTextLineHeightWithSpacing();
			CContext* ctx = CContext::GetContext();
			
			ImGui::SetCursorPos(ImVec2(navX, contentHeight - padding.y - (lineHeight * 2)));
			ImGui::TextColored(colDisabled, ctx->GetVersion().string().c_str());

			ImGui::SetCursorPos(ImVec2(navX, contentHeight - padding.y - lineHeight));
			ImGui::TextColored(colDisabled, ctx->GetBuild());

			/* vertical separator line */
			ImGuiWindow* window = ImGui::GetCurrentWindow();

			ImVec2 sepP1 = ImVec2(window->Pos.x + renderedSidebarWidth - 1.0f, window->Pos.y);
			ImVec2 sepP2 = ImVec2(window->Pos.x + renderedSidebarWidth - 1.0f, window->Pos.y + contentHeight);

			window->DrawList->AddLine(sepP1, sepP2, ImGui::GetColorU32(ImGuiCol_Separator));
		}
		this->IsSidebarActive = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
		ImGui::EndChild();
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_ChildBg, style.Colors[ImGuiCol_TitleBgActive]);
		ImGui::SetCursorPos(ImVec2(0, 0));
		if (ImGui::BeginChild("Nexus##mainwindow_header", ImVec2(wndWidth, headerHeight), false, Flags))
		{
			if (this->Tex_RaidcoreTag)
			{
				/* tag is 128x104 */
				float tagHeight = headerHeight * .6f;
				float tagWidth = (tagHeight / 104.f) * 128.f;
				float offset = headerHeight * .2f;
				ImGui::SetCursorPos(ImVec2(offset, offset));
				ImGui::Image(this->Tex_RaidcoreTag->Resource, ImVec2(tagWidth, tagHeight));

				ImVec2 titleSz = ImGui::CalcTextSize("Nexus");

				ImGui::SetCursorPos(ImVec2(offset + offset + tagWidth, (headerHeight - titleSz.y) / 2.0f));
				ImGui::Text("Nexus");
			}
			else
			{
				CContext* ctx = CContext::GetContext();
				this->Tex_RaidcoreTag = ctx->GetTextureService()->GetOrCreate("RAIDCORE_TAG", RES_ICON_RAIDCORE, ctx->GetModule());
			}

			if (this->Tex_CloseIcon)
			{
				float tagHeight = headerHeight * .6f;
				float offset = headerHeight * .2f;

				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
				ImGui::SetCursorPos(ImVec2(wndWidth - tagHeight - offset, offset));
				if (ImGui::ImageButton(this->Tex_CloseIcon->Resource, ImVec2(tagHeight, tagHeight)))
				{
					this->IsVisible = false;
				}
				ImGui::PopStyleVar();
			}
			else
			{
				CContext* ctx = CContext::GetContext();
				this->Tex_CloseIcon = ctx->GetTextureService()->GetOrCreate("ICON_CLOSE", RES_ICON_CLOSE, ctx->GetModule());
			}
		}
		this->IsHandleHeld = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
		ImGui::EndChild();
		ImGui::PopStyleColor();
	}
	ImGui::End();

	if (!poppedPadding)
	{
		ImGui::PopStyleVar(2);
	}
}

void CMainWindow::Invalidate()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	for (ISubWindow* window : this->Windows)
	{
		window->Invalidate();
	}
}

void CMainWindow::Activate(const std::string& aWindowName)
{
	if (aWindowName.empty())
	{
		this->IsVisible = !this->IsVisible;
	}
	else
	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		for (ISubWindow* window : this->Windows)
		{
			if (window->GetName() == aWindowName)
			{
				if (window->IsPopOut())
				{
					window->PopIn();
				}
				else if (this->ActiveContent == window)
				{
					this->IsVisible = !this->IsVisible;
				}
				else
				{
					this->ActiveContent = window;
					this->IsVisible = true;
				}
				break;
			}
		}
	}
}

void CMainWindow::RenderDashboard()
{
	ImGui::Text("Dashboard");
}
