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
#include "Core/DataLink/DlApi.h"
#include "Host/Events/EvtApi.h"
#include "GW2/Mumble/MblReader.h"
#include "Graphics/GrWindow.h"

using namespace Raidcore::Nexus;

///----------------------------------------------------------------------------------------------------
/// Raidcore::Nexus::GUI Namespace
///----------------------------------------------------------------------------------------------------
namespace Raidcore::Nexus::GUI
{
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
			HWND                aGameWindow,
			Graphics::Window_t& aGrWindow,
			CDataLinkApi* aDataLink,
			Host::EventApi& aEventApi,
			CSettings* aSettings
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
		HWND                GameWindow{ nullptr };
		Graphics::Window_t& GrWindow;
		CDataLinkApi* DataLink;
		Host::EventApi& EventApi;
		CSettings* Settings;
		Mumble::Identity* MumbleIdentity;
		NexusLinkData_t* NexusLink;

		bool                DpiScalingEnabled;
		float               DpiScalingFactor;
		float               EffectiveDpiScalingFactor;
		float               GameScalingFactor;
		float               MinResolutionScalingFactor;

		///----------------------------------------------------------------------------------------------------
		/// UpdateScaling:
		/// 	Updates the scaling factor of the UI.
		///----------------------------------------------------------------------------------------------------
		void UpdateScaling();
	};
}
