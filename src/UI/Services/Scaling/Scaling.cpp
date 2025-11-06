///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  UiScaling.cpp
/// Description  :  Contains the implementation for UI scaling.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Scaling.h"

#include "imgui/imgui.h"

#include "Core/Hooks/HkConst.h"
#include "Core/Preferences/PrefConst.h"
#include "GW2/Mumble/MblConst.h"

static CScaling* s_Scaling{};

/*static*/ void CScaling::OnMumbleIdentityChanged(void* aEventArgs)
{
	assert(s_Scaling);
	s_Scaling->UpdateGameUIScale();
}

/*static*/ void CScaling::OnWindowResized(void* aEventArgs)
{
	assert(s_Scaling);
	s_Scaling->UpdateResolution();
}

CScaling::CScaling(RenderContext_t* aRenderCtx, CDataLinkApi* aDataLink, CEventApi* aEventApi, CSettings* aSettings)
{
	this->RenderContext = aRenderCtx;
	this->DataLink      = aDataLink;
	this->EventApi      = aEventApi;
	this->Settings      = aSettings;

	this->MumbleIdentity = static_cast<Mumble::Identity*>(this->DataLink->GetResource(DL_MUMBLE_LINK_IDENTITY));
	this->NexusLink      = static_cast<NexusLinkData_t*>(this->DataLink->GetResource(DL_NEXUS_LINK));

	this->DpiScalingEnabled          = this->Settings->Get<bool>(OPT_DPISCALING, true);
	this->DpiScalingFactor           = 1.0f;
	this->EffectiveDpiScalingFactor  = 1.0f;
	this->GameScalingFactor          = this->Settings->Get<float>(OPT_LASTUISCALE, SC_NORMAL);
	this->MinResolutionScalingFactor = 1.0f;

	this->Settings->Subscribe<bool>(OPT_DPISCALING, [&](bool aEnabled)
	{
		this->DpiScalingEnabled = aEnabled;
		this->UpdateDPI();
	});

	this->EventApi->Subscribe(EV_MUMBLE_IDENTITY_UPDATED, CScaling::OnMumbleIdentityChanged);
	this->EventApi->Subscribe(EV_WINDOW_RESIZED, CScaling::OnWindowResized);

	if (s_Scaling)
	{
		throw "An instance of UI Scaling already exists.";
	}

	s_Scaling = this;
}

CScaling::~CScaling()
{
	this->EventApi->Unsubscribe(EV_MUMBLE_IDENTITY_UPDATED, CScaling::OnMumbleIdentityChanged);
	this->EventApi->Unsubscribe(EV_WINDOW_RESIZED, CScaling::OnWindowResized);
}

UINT CScaling::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_DPICHANGED)
	{
		this->UpdateDPI();
	}

	return uMsg;
}

void CScaling::UpdateDPI()
{
	UINT dpi = GetDpiForWindow(this->RenderContext->Window.Handle);

	if (dpi > 0)
	{
		this->DpiScalingFactor = static_cast<float>(dpi) / 96.0f;
	}
	else
	{
		this->DpiScalingFactor = 1.0f;
	}

	if (this->DpiScalingEnabled)
	{
		this->EffectiveDpiScalingFactor = this->DpiScalingFactor;
	}
	else
	{
		this->EffectiveDpiScalingFactor = 1.0f;
	}

	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = this->EffectiveDpiScalingFactor;

	/* DPI was updated, update the cumulative scaling factor. */
	this->UpdateScaling();
}

void CScaling::UpdateGameUIScale()
{
	float currScaling = Mumble::GetScalingFactor(this->MumbleIdentity->UISize);

	if (this->GameScalingFactor != currScaling)
	{
		this->Settings->Set(OPT_LASTUISCALE, currScaling);
		this->GameScalingFactor = currScaling;
		this->UpdateScaling();
	}
}

void CScaling::UpdateResolution()
{
	/// Takes the window width or 1024, whichever is smaller.
	/// Divides it by 1024, to get a relative scaling factor to that.
	/// Repeats the same with the window height and 768.
	/// Lastly, it takes whichever of these two scaling factors is smaller.
	/// If the window size is >= 1024x768, the scaling factor is 1.
	this->MinResolutionScalingFactor =
		min(
			min(this->RenderContext->Window.Width, 1024.0f) / 1024.0f,
			min(this->RenderContext->Window.Height, 768.0f) / 768.0f
		);

	this->UpdateScaling();
}

void CScaling::UpdateScaling()
{
	this->NexusLink->Scaling
		= this->GameScalingFactor
		* this->EffectiveDpiScalingFactor
		* this->MinResolutionScalingFactor;
}
