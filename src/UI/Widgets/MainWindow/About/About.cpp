///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  About.cpp
/// Description  :  Contains the content of the about window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "About.h"

#include <shellapi.h>

#include "imgui/imgui.h"
#include "ImAnimate/ImAnimate.h"

#include "Context.h"
#include "resource.h"

CAboutBox::CAboutBox()
{
	this->Name           = "About";
	this->IconIdentifier = "ICON_ABOUT";
	this->IconID         = RES_ICON_ABOUT;
}

void CAboutBox::RenderContent()
{
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

	/* banner is 640x152 size relative ot that */
	float btnHeight = btnWidth / 640.0f * 152.0f;

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	if (this->Tex_BannerDiscord)
	{
		static float discordTint = 0.9f;

		if (ImGui::ImageButton(this->Tex_BannerDiscord->Resource,
			ImVec2(btnWidth, btnHeight),
			ImVec2(0, 0),
			ImVec2(1, 1),
			-1,
			ImVec4(0, 0, 0, 0),
			ImVec4(discordTint, discordTint, discordTint, 1)))
		{
			ShellExecuteA(0, 0, "https://discord.gg/raidcore", 0, 0, SW_SHOW);
		}

		if (ImGui::IsItemHovered())
		{
			ImGui::Animate(0.9f, 1.0f, 100, &discordTint, ImAnimate::ECurve::InCubic);
		}
		else
		{
			ImGui::Animate(1.0f, 0.9f, 100, &discordTint, ImAnimate::ECurve::InCubic);
		}
	}
	else
	{
		CContext* ctx = CContext::GetContext();
		this->Tex_BannerDiscord = ctx->GetTextureService()->GetOrCreate("BANNER_DISCORD", RES_BANNER_DISCORD, ctx->GetModule());
	}

	ImGui::SameLine();

	if (this->Tex_BannerPatreon)
	{
		static float patreonTint = 0.9f;

		if (ImGui::ImageButton(this->Tex_BannerPatreon->Resource,
			ImVec2(btnWidth, btnHeight),
			ImVec2(0, 0),
			ImVec2(1, 1),
			-1,
			ImVec4(0, 0, 0, 0),
			ImVec4(patreonTint, patreonTint, patreonTint, 1)))
		{
			ShellExecuteA(0, 0, "https://www.patreon.com/bePatron?u=46163080", 0, 0, SW_SHOW);
		}

		if (ImGui::IsItemHovered())
		{
			ImGui::Animate(0.9f, 1.0f, 100, &patreonTint, ImAnimate::ECurve::InCubic);
		}
		else
		{
			ImGui::Animate(1.0f, 0.9f, 100, &patreonTint, ImAnimate::ECurve::InCubic);
		}
	}
	else
	{
		CContext* ctx = CContext::GetContext();
		this->Tex_BannerPatreon = ctx->GetTextureService()->GetOrCreate("BANNER_PATREON", RES_BANNER_PATREON, ctx->GetModule());
	}
	ImGui::PopStyleVar();

	ImGui::TextWrapped("Do not forget to actually put some text here.");
}
