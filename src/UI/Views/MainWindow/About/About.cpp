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

#include "Runtime/Runtime.h"
using namespace Raidcore::Nexus;

#include "res/ResConst.h"
#include "Util/DLL.h"

namespace Raidcore::Nexus::GUI
{
	CAboutBox::CAboutBox()
	{
		this->Name = "About";
		this->DisplayName = "((000008))";
		this->IconIdentifier = "ICON_ABOUT";
		this->IconID = RES_ICON_ABOUT;
		this->IsAnchored = true;
		this->IsHost = true;
	}

	void CAboutBox::RenderContent()
	{
		if (this->IsInvalid)
		{
			/* nop */

			this->IsInvalid = false;
		}

		Runtime& ctx = Runtime::Get();
		Graphics::TextureLoader& texapi = ctx.TextureLoader();
		Network::Updater& selfupdater = ctx.Network().Updater();

		if (ImGui::CollapsingHeader("About", ImGuiTreeNodeFlags_DefaultOpen))
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
				this->Tex_BannerDiscord = texapi.GetOrCreate("BANNER_DISCORD", RES_BANNER_DISCORD, GetCurrentModule());
			}

			ImGui::SameLine();

			ImGui::TextWrapped("The discord server is the perfect place for feedback, discussions or getting help with Nexus. Or you can simply hang out!");

			ImGui::EndGroup();
		}

		if (ImGui::CollapsingHeader("Changelog", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::TextWrapped(selfupdater.GetChangelog().c_str());
		}
	}
}
