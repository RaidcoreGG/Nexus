///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  About.cpp
/// Description  :  Contains the content of the about window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "About.h"

#include <shellapi.h>

#include "imgui.h"
#include "ImAnimate/ImAnimate.h"

#include "Core/Context.h"
#include "resource.h"

CAboutBox::CAboutBox()
{
	this->Name           = "About";
	this->DisplayName    = "((000008))";
	this->IconIdentifier = "ICON_ABOUT";
	this->IconID         = RES_ICON_ABOUT;
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

	CContext*       ctx     = CContext::GetContext();
	CTextureLoader* texapi  = ctx->GetTextureService();
	CUpdater*       updater = ctx->GetUpdater();

	if (ImGui::CollapsingHeader("About", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGuiStyle& style = ImGui::GetStyle();

		/* banner is 640x152 size relative ot that */
		float btnHeight = ImGui::GetFontSize() * 3.5f;
		float btnWidth = btnHeight / 152.f * 640.f;

		ImVec2 regionSz = ImGui::GetContentRegionAvail();
		ImVec2 btnSectionSz = ImVec2((regionSz.x - style.ItemSpacing.x) / 2, ImGui::GetTextLineHeight() * 5);
		ImVec2 infoSectionSz = ImVec2((regionSz.x - style.ItemSpacing.x) / 2, ImGui::GetTextLineHeight() * 6);
		ImVec2 infoSectionInnerSz = ImVec2(btnWidth, infoSectionSz.y);

		if (ImGui::BeginChild("DiscordButton", btnSectionSz, false, ImGuiWindowFlags_NoBackground))
		{
			if (this->Tex_BannerDiscord)
			{
				static float discordTint = 0.9f;

				ImGui::SetCursorPosX((btnSectionSz.x - btnWidth) / 2);
				ImGui::SetCursorPosY((btnSectionSz.y - btnHeight) / 2);
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
				this->Tex_BannerDiscord = texapi->GetOrCreate("BANNER_DISCORD", RES_BANNER_DISCORD, ctx->GetModule());
			}
		}
		ImGui::EndChild();

		ImGui::SameLine();

		if (ImGui::BeginChild("PatreonButton", btnSectionSz, false, ImGuiWindowFlags_NoBackground))
		{
			if (this->Tex_BannerPatreon)
			{
				static float patreonTint = 0.9f;

				ImGui::SetCursorPosX((btnSectionSz.x - btnWidth) / 2);
				ImGui::SetCursorPosY((btnSectionSz.y - btnHeight) / 2);
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
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
				ImGui::PopStyleVar();

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
				this->Tex_BannerPatreon = texapi->GetOrCreate("BANNER_PATREON", RES_BANNER_PATREON, ctx->GetModule());
			}
		}
		ImGui::EndChild();

		if (ImGui::BeginChild("DiscordInfo", infoSectionSz, false, ImGuiWindowFlags_NoBackground))
		{
			ImGui::SetCursorPosX((infoSectionSz.x - infoSectionInnerSz.x) / 2);
			if (ImGui::BeginChild("Inner", infoSectionInnerSz, false, ImGuiWindowFlags_NoBackground))
			{
				ImGui::TextWrapped("The discord server is the perfect place for feedback, discussions or getting help with Nexus. Or you can simply hang out!");
			}
			ImGui::EndChild();
		}
		ImGui::EndChild();

		ImGui::SameLine();

		if (ImGui::BeginChild("PatreonInfo", infoSectionSz, false, ImGuiWindowFlags_NoBackground))
		{
			ImGui::SetCursorPosX((infoSectionSz.x - infoSectionInnerSz.x) / 2);
			if (ImGui::BeginChild("Inner", infoSectionInnerSz, false, ImGuiWindowFlags_NoBackground))
			{
				ImGui::TextWrapped("If you would like to support Raidcore financially, you can do so via Patreon. This is absolutely optional and not required.");
			}
			ImGui::EndChild();
		}
		ImGui::EndChild();
	}

	if (ImGui::CollapsingHeader("Changelog", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextWrapped(updater->GetChangelog().c_str());
	}
}
