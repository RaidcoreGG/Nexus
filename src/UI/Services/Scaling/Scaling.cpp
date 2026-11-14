///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  UiScaling.cpp
/// Description  :  Contains the implementation for UI scaling.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Scaling.h"

#include "imgui/imgui.h"

#include "Host/GW2/Mumble/MblConst.h"

CScaling::CScaling(RenderContext_t* aRenderCtx, CDataLinkApi* aDataLink)
{
	this->RenderContext = aRenderCtx;
	this->DataLink      = aDataLink;

	this->MumbleIdentity = static_cast<Mumble::Identity*>(this->DataLink->GetResource(DL_MUMBLE_LINK_IDENTITY));
	this->NexusLink      = static_cast<NexusLinkData_t*>(this->DataLink->GetResource(DL_NEXUS_LINK));

	this->DpiScalingFactor           = 1.0f;
	this->EffectiveDpiScalingFactor  = 1.0f;
	this->MinResolutionScalingFactor = 1.0f;

	this->INotifyHandler<IMumbleIdentityNotifyHandler>::Attach(MUMBLEIDENTITY_PRIO_SCALING);
	this->INotifyHandler<IDxgiResizeNotifyHandler>::Attach(DXGIRESIZE_PRIO_SCALING);
}

CScaling::~CScaling()
{
	this->INotifyHandler<IDxgiResizeNotifyHandler>::Detach();
	this->INotifyHandler<IMumbleIdentityNotifyHandler>::Detach();
}

void CScaling::OnMumbleIdentityNotify(const Mumble::Identity& aIdentity)
{
	this->UpdateGameUIScale();
}

void CScaling::OnDxgiResizeNotify(IDXGISwapChain* aSwapChain, UINT aWidth, UINT aHeight)
{
	this->UpdateResolution(aWidth, aHeight);
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

	if (this->DpiScaling.Get())
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

	if (this->LastUIScale.Get() != currScaling)
	{
		this->LastUIScale.Set(currScaling);
		this->UpdateScaling();
	}
}

void CScaling::UpdateResolution(UINT aWidth, UINT aHeight)
{
	/// Takes the window width or 1024, whichever is smaller.
	/// Divides it by 1024, to get a relative scaling factor to that.
	/// Repeats the same with the window height and 768.
	/// Lastly, it takes whichever of these two scaling factors is smaller.
	/// If the window size is >= 1024x768, the scaling factor is 1.
	this->MinResolutionScalingFactor =
		min(
			min(static_cast<float>(aWidth), 1024.0f) / 1024.0f,
			min(static_cast<float>(aHeight), 768.0f) / 768.0f
		);

	this->UpdateScaling();
}

void CScaling::UpdateScaling()
{
	this->NexusLink->Scaling
		= this->LastUIScale.Get()
		* this->EffectiveDpiScalingFactor
		* this->MinResolutionScalingFactor;
}
