///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Scaling.h
/// Description  :  Contains the implementation for UI scaling.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "Core/NexusLink.h"
#include "Core/Preferences/PrefContext.h"
#include "Engine/_Concepts/IWndProc.h"
#include "Engine/DataLink/DlApi.h"
#include "Engine/Events/EvtApi.h"
#include "GW2/Mumble/MblReader.h"
#include "UI/Renderer/RdrContext.h"

///----------------------------------------------------------------------------------------------------
/// CScaling Class
///----------------------------------------------------------------------------------------------------
class CScaling : public virtual IWndProc
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// OnMumbleIdentityChanged:
	/// 	Update game base scale.
	///----------------------------------------------------------------------------------------------------
	static void OnMumbleIdentityChanged(void* aEventArgs);

	///----------------------------------------------------------------------------------------------------
	/// OnWindowResized:
	/// 	Update game base scale.
	///----------------------------------------------------------------------------------------------------
	static void OnWindowResized(void* aEventArgs);

	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CScaling(
		RenderContext_t* aRenderCtx,
		CDataLinkApi*    aDataLink,
		CEventApi*       aEventApi,
		CSettings*       aSettings
	);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CScaling();

	///----------------------------------------------------------------------------------------------------
	/// WndProc:
	/// 	Returns 0 if message was processed.
	///----------------------------------------------------------------------------------------------------
	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

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
	void UpdateResolution();

	private:
	RenderContext_t*  RenderContext;
	CDataLinkApi*     DataLink;
	CEventApi*        EventApi;
	CSettings*        Settings;
	Mumble::Identity* MumbleIdentity;
	NexusLinkData_t*  NexusLink;

	bool              DpiScalingEnabled;
	float             DpiScalingFactor;
	float             EffectiveDpiScalingFactor;
	float             GameScalingFactor;
	float             MinResolutionScalingFactor;

	///----------------------------------------------------------------------------------------------------
	/// UpdateScaling:
	/// 	Updates the scaling factor of the UI.
	///----------------------------------------------------------------------------------------------------
	void UpdateScaling();
};
