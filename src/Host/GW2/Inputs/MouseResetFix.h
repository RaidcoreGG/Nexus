///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  MouseResetFix.h
/// Description  :  Fix for the cursor reseting to (0,0). Fix for the cursor moving while hidden.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <windows.h>

#include "Core/Preferences/PrefSetting.h"
#include "Engine/Inputs/RawInput/WndProcNotify.h"

namespace Raidcore::GW2
{
	///----------------------------------------------------------------------------------------------------
	/// CMouseResetFix Class
	///----------------------------------------------------------------------------------------------------
	class CMouseResetFix : public Engine::Input::IWndProcNotifyHandler
	{
		public:
		///----------------------------------------------------------------------------------------------------
		/// ctor
		///----------------------------------------------------------------------------------------------------
		CMouseResetFix();

		///----------------------------------------------------------------------------------------------------
		/// dtor
		///----------------------------------------------------------------------------------------------------
		~CMouseResetFix();

		///----------------------------------------------------------------------------------------------------
		/// OnWndProcNotify:
		/// 	Tracks the camera drag. Never consumes.
		///----------------------------------------------------------------------------------------------------
		UINT OnWndProcNotify(HWND& hWnd, UINT& uMsg, WPARAM& wParam, LPARAM& lParam) override;

		Core::CSetting<bool> LockCursor { "CameraControl_LockCursor", false };
		Core::CSetting<bool> ResetCursor{ "CameraControl_ResetCursor", false };

		private:
		bool  IsConfining = false;
		POINT LastPos     = {};

		///----------------------------------------------------------------------------------------------------
		/// Release:
		/// 	Ends the confinement and restores the cursor to where the drag started.
		///----------------------------------------------------------------------------------------------------
		void Release();
	};
}
