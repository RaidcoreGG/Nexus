///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Alerts.cpp
/// Description  :  Contains the logic for the Alerts HUD element.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Alerts.h"

#include "imgui/imgui.h"
#include "imgui/imgui_extensions.h"
#include "ImAnimate/ImAnimate.h"

#include "Renderer.h"

constexpr ImGuiWindowFlags Flags
	= ImGuiWindowFlags_NoDecoration
	| ImGuiWindowFlags_NoInputs
	| ImGuiWindowFlags_NoNav
	| ImGuiWindowFlags_AlwaysAutoResize
	| ImGuiWindowFlags_NoResize
	| ImGuiWindowFlags_NoSavedSettings
	| ImGuiWindowFlags_NoBackground;

CAlerts::CAlerts(CDataLinkApi* aDataLink)
{
	this->NexusLink = (NexusLinkData*)aDataLink->GetResource("DL_NEXUS_LINK");
}

void CAlerts::Render()
{
	if (this->Queue.size() > 0)
	{
		AlertMessage& alert = this->Queue.front();

		ImGui::PushFont((ImFont*)this->NexusLink->FontBig);
		float width = ImGui::CalcTextSize(alert.Message.c_str()).x;

		/* center horizontally */
		ImGui::SetNextWindowPos(ImVec2((Renderer::Width - width) / 2.0f, 230.0f * Renderer::Scaling));
		if (ImGui::Begin("##Alerts", (bool*)0, Flags))
		{
			ImColor msgCol;
			switch (alert.Type)
			{
				default:
				case EAlertType::Info:
					msgCol = ImVec4(1.f, 1.f, 0.f, this->Opacity);
					break;
				case EAlertType::Error:
					msgCol = ImVec4(1.f, 0.f, 0.f, this->Opacity);
					break;
			}
			ImGui::TextColoredOutlined(msgCol, "%s", alert.Message.c_str());
		}
		ImGui::End();

		ImGui::PopFont();

		if (alert.StartTime == 0)
		{
			alert.StartTime = ImGui::GetCurrentContext()->Time * 1000;
		}
		else if ((ImGui::GetCurrentContext()->Time * 1000 - alert.StartTime) > 5000)
		{
			ImGui::Animate(1, 0, 2500, &this->Opacity, ImAnimate::ECurve::InCubic);
		}
	}

	if (this->Opacity == 0)
	{
		this->Queue.erase(this->Queue.begin());
		this->Opacity = 1;
	}
}

void CAlerts::Notify(EAlertType aType, const char* aMessage)
{
	if (aType == EAlertType::None) { return; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	/* reset fade out if it's the same message */
	if (this->Queue.size() > 0 && this->Queue.front().Message == aMessage)
	{
		this->Opacity = 1.0f;
	}
	else /* otherwise add it to the queue */
	{
		this->Queue.push_back(AlertMessage{aType, aMessage });
	}
}
