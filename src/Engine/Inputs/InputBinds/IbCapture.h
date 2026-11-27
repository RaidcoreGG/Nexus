///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IbCapture.h
/// Description  :  Provides functions for capturing InputBinds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "IbBind.h"

#include <windows.h>

namespace Engine::Input
{
	///----------------------------------------------------------------------------------------------------
	/// CInputBindCapture Class
	///----------------------------------------------------------------------------------------------------
	class CInputBindCapture
	{
		public:
		///----------------------------------------------------------------------------------------------------
		/// StartCapturing:
		/// 	Starts capturing the held Bind_t.
		/// 	The bind is continuously captured.
		/// 	Calling this function merely signals to stop further processing the inputs.
		///----------------------------------------------------------------------------------------------------
		void StartCapturing();

		///----------------------------------------------------------------------------------------------------
		/// EndCapturing:
		/// 	Ends capturing the held Bind_t.
		///----------------------------------------------------------------------------------------------------
		void EndCapturing();

		///----------------------------------------------------------------------------------------------------
		/// GetCapture:
		/// 	Gets which Bind_t is currently held.
		///----------------------------------------------------------------------------------------------------
		Bind_t GetCapture() const;

		protected:
		///----------------------------------------------------------------------------------------------------
		/// ProcessInput:
		/// 	Returns true, if the input should not be further processed.
		///----------------------------------------------------------------------------------------------------
		bool ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam);

		///----------------------------------------------------------------------------------------------------
		/// GetCaptureRef:
		/// 	Gets which Bind_t is currently held.
		/// 	Not threadsafe. Only use during WndProc.
		///----------------------------------------------------------------------------------------------------
		const Bind_t& GetCaptureRef() const;

		private:
		bool       IsCapturing{ false };
		Bind_t     Capture{};

		EModifiers ModifiersHeld{ EModifiers::None };
	};
}
