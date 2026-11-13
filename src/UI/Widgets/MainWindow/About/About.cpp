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

#include "Host/GW2/Context.h"
#include "res/Resources.h"

CAboutBox::CAboutBox()
{
	this->Name           = "About";
	this->DisplayName    = Lang::Nav::About;
	this->IconIdentifier = "ICON_ABOUT";
	this->IconEntry      = &Resources::IconAbout;
	this->IsAnchored     = true;
	this->IsHost         = true;
}

void CAboutBox::RenderContent()
{
	if (this->IsInvalid)
	{
		/* nop */

		this->IsInvalid = false;
	}

	CContext*       ctx         = CContext::GetContext();
	CTextureLoader* texapi      = ctx->GetTextureService();
	CSelfUpdater*   selfupdater = ctx->GetSelfUpdater();

	if (ImGui::CollapsingHeader(Lang::Nav::About.Text(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		/* banner is 640x152 size relative ot that */
		float btnHeight = ImGui::GetFontSize() * 3.5f;
		float btnWidth = btnHeight / 152.f * 640.f;

		ImGui::BeginGroup();

		if (this->Tex_BannerDiscord)
		{
			static float discordTint = 0.9f;

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
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
			ImGui::PopStyleVar();

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
			this->Tex_BannerDiscord = texapi->GetOrCreate("BANNER_DISCORD", Resources::BannerDiscord);
		}

		ImGui::SameLine();

		ImGui::TextWrapped("%s", Lang::About::DiscordHint.Text());

		ImGui::EndGroup();
	}

	if (ImGui::CollapsingHeader(Lang::Nav::Changelog.Text(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextWrapped(selfupdater->GetChangelog().c_str());
	}
}
