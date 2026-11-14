///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Scaling.h
/// Description  :  Contains the implementation for UI scaling.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "Host/GW2/Addons/API/NexusLink.h"
#include "Core/Preferences/PrefSetting.h"
#include <windows.h>
#include "Engine/DataLink/DlApi.h"
#include "Host/GW2/Hooks/HkNotify.h"
#include "Host/GW2/Mumble/MblConst.h"
#include "Host/GW2/Mumble/MblNotify.h"
#include "Host/GW2/Mumble/MblReader.h"
#include "UI/Renderer/RdrContext.h"

///----------------------------------------------------------------------------------------------------
/// CScaling Class
///----------------------------------------------------------------------------------------------------
class CScaling
	: public IMumbleIdentityNotifyHandler
	, public IDxgiResizeNotifyHandler
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CScaling(
		RenderContext_t* aRenderCtx,
		CDataLinkApi*    aDataLink
	);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CScaling();

	///----------------------------------------------------------------------------------------------------
	/// OnMumbleIdentityNotify:
	/// 	Updates the game UI scale.
	///----------------------------------------------------------------------------------------------------
	void OnMumbleIdentityNotify(const Mumble::Identity& aIdentity) override;

	///----------------------------------------------------------------------------------------------------
	/// OnDxgiResizeNotify:
	/// 	Updates the minimum resolution scaling before the game window raises EV_WINDOW_RESIZED.
	///----------------------------------------------------------------------------------------------------
	void OnDxgiResizeNotify(IDXGISwapChain* aSwapChain, UINT aWidth, UINT aHeight) override;

	///----------------------------------------------------------------------------------------------------
	/// WndProc:
	/// 	Returns 0 if message was processed.
	///----------------------------------------------------------------------------------------------------
	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	///----------------------------------------------------------------------------------------------------
	/// UpdateDPI:
	/// 	Updates the DPI factor of the UI.
	///----------------------------------------------------------------------------------------------------
	void UpdateDPI();

	///----------------------------------------------------------------------------------------------------
	/// UpdateGameUIScale:
	/// 	Updates the game dependant UI scaling.
	///----------------------------------------------------------------------------------------------------
	void UpdateGameUIScale();

	///----------------------------------------------------------------------------------------------------
	/// UpdateResolution:
	/// 	Updates the minimum resolution scaling.
	///----------------------------------------------------------------------------------------------------
	void UpdateResolution(UINT aWidth, UINT aHeight);

	private:
	RenderContext_t*  RenderContext;
	CDataLinkApi*     DataLink;
	Mumble::Identity* MumbleIdentity;
	NexusLinkData_t*  NexusLink;

	float             DpiScalingFactor;
	float             EffectiveDpiScalingFactor;
	float             MinResolutionScalingFactor;

	///----------------------------------------------------------------------------------------------------
	/// UpdateScaling:
	/// 	Updates the scaling factor of the UI.
	///----------------------------------------------------------------------------------------------------
	void UpdateScaling();
	public:
	CSetting<bool>    DpiScaling { "DPIScaling", true, [this](const bool&) { this->UpdateDPI(); } };
	CSetting<float>   LastUIScale{ "LastUIScale", SC_NORMAL };
};
