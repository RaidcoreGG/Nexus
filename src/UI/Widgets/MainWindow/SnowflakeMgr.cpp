///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  SnowflakeMgr.cpp
/// Description  :  Contains the logic for winter flair.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "SnowflakeMgr.h"

#include "imgui/imgui_extensions.h"
#include "imgui/imgui_internal.h"

#include "Core/Context.h"
#include "ImAnimate/ImAnimate.h"
#include "resource.h"
#include "Util/Time.h"
#include "Core/Preferences/PrefConst.h"

CSnowflakeMgr::CSnowflakeMgr()
{
	CContext* ctx = CContext::GetContext();
	CSettings* settingsctx = ctx->GetSettingsCtx();

	bool isPartyPooper = settingsctx->Get<bool>(OPT_DISABLEFESTIVEFLAIR, false);

	IsItChristmas = Time::GetMonth() == 12 && !isPartyPooper;
}

ImVec2 Rotate(const ImVec2& aPoint, float aCosA, float aSinA)
{
	return ImVec2(aPoint.x * aCosA - aPoint.y * aSinA, aPoint.x * aSinA + aPoint.y * aCosA);
}

void ImageRotated(ImTextureID aTextureIdentifier, ImVec2 aOrigin, ImVec2 aSize, float aAngle)
{
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	// convert degrees to radians
	aAngle *= 3.14159f / 180.0f;

	float cos_a = cosf(aAngle);
	float sin_a = sinf(aAngle);
	ImVec2 pos[4] =
	{
		aOrigin + Rotate(ImVec2(-aSize.x * 0.5f, -aSize.y * 0.5f), cos_a, sin_a),
		aOrigin + Rotate(ImVec2(+aSize.x * 0.5f, -aSize.y * 0.5f), cos_a, sin_a),
		aOrigin + Rotate(ImVec2(+aSize.x * 0.5f, +aSize.y * 0.5f), cos_a, sin_a),
		aOrigin + Rotate(ImVec2(-aSize.x * 0.5f, +aSize.y * 0.5f), cos_a, sin_a)
	};
	ImVec2 uvs[4] =
	{
		ImVec2(0.0f, 0.0f),
		ImVec2(1.0f, 0.0f),
		ImVec2(1.0f, 1.0f),
		ImVec2(0.0f, 1.0f)
	};

	draw_list->AddImageQuad(aTextureIdentifier, pos[0], pos[1], pos[2], pos[3], uvs[0], uvs[1], uvs[2], uvs[3], (ImColor)ImVec4(1.f, 1.f, 1.f, 0.7f));
}

void CSnowflakeMgr::Update()
{
	if (!IsItChristmas) { return; }

	static CContext* ctx = CContext::GetContext();
	static CTextureLoader* texapi = ctx->GetTextureService();

	ImGuiContext* imctx = ImGui::GetCurrentContext();

	if (!imctx->CurrentWindow) { return; }

	std::vector<Snowflake_t>& snowflakes = this->Snowflakes[imctx->CurrentWindow->ID];

	ImVec2 wndSize = imctx->CurrentWindow->Size;
	ImVec2 wndPos = imctx->CurrentWindow->Pos;

	/* density logic */
	int amtSnowflakes = wndSize.x / ImGui::GetFontSize() * 0.5f;

	if (snowflakes.size() != amtSnowflakes)
	{
		std::srand(std::time(nullptr));

		static Texture_t* texSnowflake = texapi->GetOrCreate("SNOWFLAKE", RES_ICON_SNOWFLAKE, CContext::GetContext()->GetModule());

		if (!texSnowflake)
		{
			texSnowflake = texapi->GetOrCreate("SNOWFLAKE", RES_ICON_SNOWFLAKE, CContext::GetContext()->GetModule());
			return;
		}

		snowflakes.clear();

		while (snowflakes.size() < amtSnowflakes)
		{
			snowflakes.push_back(Snowflake_t{
				texSnowflake, // texture
				(float)((ImGui::GetFontSize() / 2) + (std::rand() % (int)ImGui::GetFontSize())), // Size
				(float)(std::rand() % (int)wndSize.x), //X
				(float)(std::rand() % (int)wndSize.y), //Y

				(float)(10000 + (std::rand() % 10000)), // FallDuration

				std::rand() % 10 > 5, // swaydir
				(float)(2000 + (std::rand() % 1000)), //SwayDuration;
				0,
				(float)(-1 * ( 10 + (std::rand() % 90))), //SwayStart;
				(float)(1 * (10 + (std::rand() % 90))), //SwayEnd;

				std::rand() % 10 > 5, // tiltdir
				(float)(1000 + (std::rand() % 1000)), //TiltDuration;
				180, // Angle
				(float)(120 + (std::rand() % 30)), //AngleStart;
				(float)(210 + (std::rand() % 30)) //AngleEnd;
			});
		}
	}

	/* rendering logic */
	for (Snowflake_t& sf : snowflakes)
	{
		float angleFixed = sf.Angle - 180;

		if (angleFixed < 0) angleFixed += 360;

		ImageRotated(sf.Texture_t->Resource, ImVec2(wndPos.x + sf.X + sf.SwayOffset, wndPos.y + sf.Y), ImVec2(sf.Size, sf.Size), angleFixed);

		ImGui::Animate(-sf.Size, wndSize.y + sf.Size, sf.FallDuration, &sf.Y, ImAnimate::ECurve::Linear);

		if (sf.Tilt)
		{
			ImGui::Animate(sf.AngleStart, sf.AngleEnd, sf.TiltDuration, &sf.Angle, ImAnimate::ECurve::InOutQuad);

			if (sf.Angle == sf.AngleEnd) { sf.Tilt = false; }
		}
		else
		{
			ImGui::Animate(sf.AngleEnd, sf.AngleStart, sf.TiltDuration, &sf.Angle, ImAnimate::ECurve::InOutQuad);

			if (sf.Angle == sf.AngleStart) { sf.Tilt = true; }
		}

		if (sf.Sway)
		{
			ImGui::Animate(sf.SwayStart, sf.SwayEnd, sf.SwayDuration, &sf.SwayOffset, ImAnimate::ECurve::InOutQuad);

			if (sf.SwayOffset == sf.SwayEnd) { sf.Sway = false; }
		}
		else
		{
			ImGui::Animate(sf.SwayEnd, sf.SwayStart, sf.SwayDuration, &sf.SwayOffset, ImAnimate::ECurve::InOutQuad);

			if (sf.SwayOffset == sf.SwayStart) { sf.Sway = true; }
		}

		if (sf.Y == wndSize.y + sf.Size)
		{
			sf.Y = -sf.Size;
		}
	}
}

void CSnowflakeMgr::Clear()
{
	this->Snowflakes.clear();
}
