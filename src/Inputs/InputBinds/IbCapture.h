///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IbCapture.h
/// Description  :  Provides functions for capturing InputBinds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "IbBindV2.h"

#include <windows.h>

///----------------------------------------------------------------------------------------------------
/// CInputBindCapture Class
///----------------------------------------------------------------------------------------------------
class CInputBindCapture
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// StartCapturing:
	/// 	Starts capturing the held InputBind_t.
	/// 	The bind is continuously captured.
	/// 	Calling this function merely signals to stop further processing the inputs.
	///----------------------------------------------------------------------------------------------------
	void StartCapturing();

	///----------------------------------------------------------------------------------------------------
	/// EndCapturing:
	/// 	Ends capturing the held InputBind_t.
	///----------------------------------------------------------------------------------------------------
	void EndCapturing();

	///----------------------------------------------------------------------------------------------------
	/// GetCapture:
	/// 	Gets which InputBind_t is currently held.
	///----------------------------------------------------------------------------------------------------
	InputBind_t GetCapture() const;

	protected:
	///----------------------------------------------------------------------------------------------------
	/// ProcessInput:
	/// 	Returns true, if the input should not be further processed.
	///----------------------------------------------------------------------------------------------------
	bool ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam);

	///----------------------------------------------------------------------------------------------------
	/// GetCaptureRef:
	/// 	Gets which InputBind_t is currently held.
	/// 	Not threadsafe. Only use during WndProc.
	///----------------------------------------------------------------------------------------------------
	const InputBind_t& GetCaptureRef() const;

	private:
	bool      IsCapturing = false;
	InputBind_t Capture     = {};

	bool      IsAltHeld   = false;
	bool      IsCtrlHeld  = false;
	bool      IsShiftHeld = false;
};
