///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  CtlSubWindow.cpp
/// Description  :  Contains the functionality for a sub window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "CtlSubWindow.h"

#include "imgui_extensions.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "Context.h"
#include "resource.h"

void ISubWindow::Render()
{
	bool anchored = this->IsAnchored;

	if (anchored)
	{
		this->IsPoppedOut = false;
	}

	bool popout = this->IsPopOut();

	if (popout)
	{
		ImGui::Begin(this->Name.c_str(), &this->IsPoppedOut, ImGuiWindowFlags_NoCollapse);
	}

	float width = ImGui::GetWindowWidth();
	float height = ImGui::GetWindowHeight();

	ImGuiStyle& style = ImGui::GetStyle();

	float btnSz = ImGui::GetFontSize() * 1.5f;

	ImGuiWindow* wnd = ImGui::GetCurrentWindow();

	ImVec2 btnPos = ImVec2(width - (btnSz + (this->IsPoppedOut ? style.WindowPadding.x : 0)) - (wnd->ScrollbarY ? wnd->ScrollbarSizes.x : 0), wnd->TitleBarHeight() + (this->IsPoppedOut ? style.WindowPadding.y : 0));

	this->RenderContent();

	if (anchored)
	{
		return;
	}

	if (!popout && this->Tex_PopoutIcon)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		ImGui::SetCursorPos(btnPos);
		if (ImGui::ImageButton(this->Tex_PopoutIcon->Resource, ImVec2(btnSz, btnSz)))
		{
			this->IsPoppedOut = !this->IsPoppedOut;
		}
		ImGui::PopStyleVar();
		ImGui::TooltipGeneric("Click to pop-out into separate window.");
	}
	else
	{
		CContext* ctx = CContext::GetContext();
		this->Tex_PopoutIcon = ctx->GetTextureService()->GetOrCreate("ICON_POPOUT", RES_ICON_POPOUT, ctx->GetModule());
	}

	if (popout)
	{
		ImGui::End();
	}
}

void ISubWindow::RenderSubWindows()
{
	/* nop */
}

const Texture* ISubWindow::GetIcon()
{
	if (!this->Tex_Icon && !this->IconIdentifier.empty() && this->IconID)
	{
		CContext* ctx = CContext::GetContext();
		this->Tex_Icon = ctx->GetTextureService()->GetOrCreate(this->IconIdentifier.c_str(), this->IconID, ctx->GetModule());
	}

	return this->Tex_Icon;
}

bool ISubWindow::IsPopOut()
{
	return this->IsPoppedOut && !this->IsAnchored;
}

void ISubWindow::PopIn()
{
	this->IsPoppedOut = false;
}

bool ISubWindow::HasSubWindows()
{
	return this->IsHost;
}

bool* ISubWindow::GetVisibleStatePtr()
{
	return &this->IsPoppedOut;
}