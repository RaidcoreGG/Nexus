///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  MainWindow.cpp
/// Description  :  Contains the logic for the main Nexus UI window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "MainWindow.h"

#include "ImAnimate/ImAnimate.h"
#include "imgui/imgui.h"
#include "imgui/imgui_extensions.h"
#include "imgui/imgui_internal.h"

#include "Core/Context.h"
#include "Resources/ResConst.h"
#include "UI/Widgets/MainWindow/About/About.h"
#include "UI/Widgets/MainWindow/Addons/Addons.h"
#include "UI/Widgets/MainWindow/Binds/Binds.h"
#include "UI/Widgets/MainWindow/Debug/Debug.h"
#include "UI/Widgets/MainWindow/Log/Log.h"
#include "UI/Widgets/MainWindow/Options/Options.h"
#include "Util/Time.h"

#include "SnowflakeMgr.h"

constexpr ImGuiWindowFlags Flags
	= ImGuiWindowFlags_NoTitleBar
	| ImGuiWindowFlags_NoCollapse
	| ImGuiWindowFlags_NoScrollbar;

static CMainWindow* s_MainWindow{};

/*static*/ void CMainWindow::OnInputBind(std::string aIdentifier)
{
	if (!s_MainWindow) { return; }

	if (aIdentifier == KB_MENU)
	{
		s_MainWindow->Activate();
	}
	else if (aIdentifier == KB_ADDONS)
	{
		s_MainWindow->Activate("Addons");
	}
	else if (aIdentifier == KB_DEBUG)
	{
		s_MainWindow->Activate("Debug");
	}
	else if (aIdentifier == KB_LOG)
	{
		s_MainWindow->Activate("Log");
	}
	else if (aIdentifier == KB_OPTIONS)
	{
		s_MainWindow->Activate("Options");
	}
}

CMainWindow::CMainWindow()
{
	CContext*      ctx    = CContext::GetContext();
	CLogApi*       logger = ctx->GetLogger();
	CInputBindApi* ibapi  = ctx->GetInputBindApi();

	CAddonsWindow*  addonsWnd  = new CAddonsWindow();
	COptionsWindow* optionsWnd = new COptionsWindow();
	CBindsWindow*   bindsWNd   = new CBindsWindow();
	CLogWindow*     logWnd     = new CLogWindow();
	CDebugWindow*   debugWnd   = new CDebugWindow();
	CAboutBox*      aboutWnd   = new CAboutBox();

	logger->Register(logWnd);

	this->AddWindow(addonsWnd);
	this->AddWindow(optionsWnd);
	this->AddWindow(bindsWNd);
	this->AddWindow(logWnd);
	this->AddWindow(debugWnd);
	this->AddWindow(aboutWnd);

	/* register InputBinds */
	ibapi->Register(KB_MENU,    EIbHandlerType::DownAsync, CMainWindow::OnInputBind, "CTRL+O");
	ibapi->Register(KB_ADDONS,  EIbHandlerType::DownAsync, CMainWindow::OnInputBind, "(null)");
	ibapi->Register(KB_OPTIONS, EIbHandlerType::DownAsync, CMainWindow::OnInputBind, "(null)");
	ibapi->Register(KB_LOG,     EIbHandlerType::DownAsync, CMainWindow::OnInputBind, "(null)");
	ibapi->Register(KB_DEBUG,   EIbHandlerType::DownAsync, CMainWindow::OnInputBind, "(null)");

	if (s_MainWindow)
	{
		throw "CMainWindow already registered.";
	}

	s_MainWindow = this;
}

CMainWindow::~CMainWindow()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	for (ISubWindow* window : this->Windows)
	{
		delete window;
	}
}

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

	static CSnowflakeMgr s_SnowflakeMgr = {}; // winter flair

	/* don't continue if not visible */
	if (!this->IsVisible)
	{
		s_SnowflakeMgr.Clear();
		return;
	}

	/* restore active content */
	if (this->ActiveContent && this->ActiveContent->IsPopOut())
	{
		this->ActiveContent = nullptr;
	}

	if (!this->ActiveContent)
	{
		for (ISubWindow* window : this->Windows)
		{
			if (!window->IsPopOut())
			{
				this->ActiveContent = window;
				break;
			}
		}
	}

	ImGuiStyle& style = ImGui::GetStyle();
	ImVec2 padding = style.WindowPadding;

	float footerHeight = ImGui::GetFrameHeightWithSpacing();
	float navItemWidth = (ImGui::GetFrameHeightWithSpacing() * 1.2f);
	float sidebarWidth = (padding.x * 2) + navItemWidth;

	/* tag is 128x110 */
	float tagWidth = navItemWidth;
	float tagHeight = 55.f * (navItemWidth / 64.f);
	float headerHeight = (padding.x * 2) + tagHeight;

	bool poppedPadding = false;

	float minWidth = 1000.f;
	float minHeight = 600.f + headerHeight;

	/* render main window */
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(minWidth, minHeight), ImGuiCond_FirstUseEver);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(minWidth * .5f, minHeight * .5f));
	if (ImGui::Begin("Nexus", (bool*)0, Flags | (this->IsHandleHeld ? 0 : ImGuiWindowFlags_NoMove)))
	{
		static CContext* ctx = CContext::GetContext();

		float wndWidth = ImGui::GetWindowWidth();
		float contentHeight = ImGui::GetWindowHeight() - headerHeight;
		float contentWidth = wndWidth - sidebarWidth;

		ImGui::PopStyleVar(2);
		poppedPadding = true;

		ImGui::SetCursorPos(ImVec2(sidebarWidth + padding.x, headerHeight + padding.y));
		if (ImGui::BeginChild("Page", ImVec2(contentWidth - (padding.x * 2), contentHeight - footerHeight - (padding.y * 2)), false, ImGuiWindowFlags_NoBackground))
		{
			if (this->ActiveContent)
			{
				this->ActiveContent->Render();
			}
			else
			{
				//	this->RenderDashboard();
			}
		}
		ImGui::EndChild();

		ImGui::SetCursorPos(ImVec2(sidebarWidth, headerHeight + contentHeight - footerHeight));
		if (ImGui::BeginChild("Footer", ImVec2(contentWidth, footerHeight), false, ImGuiWindowFlags_NoBackground))
		{
			ImGui::SetCursorPos(ImVec2(0, 0));
			ImGui::Separator();

			float copyrightNoticeWidth = ImGui::CalcTextSize(reinterpret_cast<const char*>(u8"© 2021 - 2025 Raidcore")).x;

			ImGui::SetCursorPos(ImVec2(((contentWidth - copyrightNoticeWidth) / 2) - sidebarWidth, (footerHeight - ImGui::GetFontSize()) / 2));
			ImGui::Text(reinterpret_cast<const char*>(u8"© 2021 - 2025 Raidcore"));
		}
		ImGui::EndChild();

		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_TitleBgCollapsed));
		ImGui::SetCursorPos(ImVec2(0, headerHeight));
		if (ImGui::BeginChild("Sidebar", ImVec2(sidebarWidth, contentHeight), false, ImGuiWindowFlags_NoBackground))
		{
			float navX = padding.x;
			float navY = padding.y;

			/* Dashboard Nav Item */
			{
				//ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
				//ImGui::SetCursorPos(ImVec2(navX, navY));
				//if (ImGui::Selectable("##Dashboard",
				//	this->ActiveContent == nullptr,
				//	0,
				//	navItemSz))
				//{
				//	this->ActiveContent = nullptr;
				//}
				//ImGui::PopStyleVar();

				//if (this->Tex_DashboardIcon)
				//{
				//	ImGui::SetCursorPos(ImVec2(navX, navY));
				//	ImGui::Image(Tex_DashboardIcon->Resource, ImVec2(navItemSz.y, navItemSz.y));
				//}
				//else
				//{
				//	CContext* ctx = CContext::GetContext();
				//	this->Tex_DashboardIcon = ctx->GetTextureService()->GetOrCreate("ICON_DASHBOARD", RES_ICON_DASHBOARD, ctx->GetModule());
				//}

				//ImGui::SetCursorPos(ImVec2(navX + navItemSz.y + padding.x, navY + ((navItemSz.y - ImGui::GetTextLineHeight()) / 2)));
				//ImGui::TextColored(navTextCol, "Dashboard");

				//navY += navItemSz.y + padding.y;
			}
			
			static CUiContext* uictx = ctx->GetUIContext();
			static CLocalization* langApi = uictx->GetLocalization();

			/* Dynamic Nav Items */
			for (ISubWindow* window : this->Windows)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
				ImGui::SetCursorPos(ImVec2(navX, navY));
				if (ImGui::Selectable(("##" + window->GetName()).c_str(),
					this->ActiveContent && this->ActiveContent->GetName() == window->GetName(),
					0,
					ImVec2(navItemWidth, navItemWidth)))
				{
					if (!window->IsPopOut())
					{
						this->ActiveContent = window;
					}
					else
					{
						std::string wndName = langApi->Translate(window->GetDisplayName().c_str());
						ImGui::SetWindowFocus((wndName + "##" + window->GetName()).c_str());
					}
				}
				ImGui::PopStyleVar();

				ImGui::TooltipGeneric(langApi->Translate(window->GetDisplayName().c_str()));

				const Texture_t* icon = window->GetIcon();
				if (icon)
				{
					ImGui::SetCursorPos(ImVec2(navX, navY));
					ImGui::Image(icon->Resource, ImVec2(navItemWidth, navItemWidth));
				}

				navY += navItemWidth + padding.y;
			}

			/* vertical separator line */
			ImGuiWindow* window = ImGui::GetCurrentWindow();

			ImVec2 sepP1 = ImVec2(window->Pos.x + sidebarWidth - 1.0f, window->Pos.y);
			ImVec2 sepP2 = ImVec2(window->Pos.x + sidebarWidth - 1.0f, window->Pos.y + contentHeight);

			window->DrawList->AddLine(sepP1, sepP2, ImGui::GetColorU32(ImGuiCol_Separator));
		}

		ImGui::EndChild();
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_ChildBg, style.Colors[ImGuiCol_TitleBgActive]);
		ImGui::SetCursorPos(ImVec2(0, 0));
		if (ImGui::BeginChild("Header", ImVec2(wndWidth, headerHeight), false, ImGuiWindowFlags_NoBackground))
		{
			if (this->Tex_RaidcoreTag)
			{
				float offset = style.WindowPadding.x;
				ImGui::SetCursorPos(ImVec2(offset, offset));
				ImGui::Image(this->Tex_RaidcoreTag->Resource, ImVec2(tagWidth, tagHeight));

				ImVec2 titleSz = ImGui::CalcTextSize("Nexus");

				ImGui::SetCursorPos(ImVec2(offset + offset + tagWidth, (headerHeight - titleSz.y) / 2.0f));
				ImGui::Text("Nexus");

				std::string v = ctx->GetVersion().string();

				ImVec2 versionSz = ImGui::CalcTextSize(v.c_str());

				//ImGui::SetCursorPos(ImVec2(navX, contentHeight - padding.y - (lineHeight * 2)));
				ImGui::SetCursorPos(ImVec2(offset + offset + offset + tagWidth + titleSz.x, (headerHeight - versionSz.y) / 2.0f));
				ImGui::TextDisabled(v.c_str());

				//ImGui::SetCursorPos(ImVec2(navX, contentHeight - padding.y - lineHeight));
				//ImGui::TextColored(colDisabled, ctx->GetBuild());

			}
			else
			{
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
				this->Tex_CloseIcon = ctx->GetTextureService()->GetOrCreate("ICON_CLOSE", RES_ICON_CLOSE, ctx->GetModule());
			}
		}
		this->IsHandleHeld = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
		ImGui::EndChild();
		ImGui::PopStyleColor();

		s_SnowflakeMgr.Update();
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
