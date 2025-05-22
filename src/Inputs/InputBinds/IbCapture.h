///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IbCapture.h
/// Description  :  Provides functions for capturing InputBinds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef IBCAPTURE_H
#define IBCAPTURE_H

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
	/// 	Starts capturing the held InputBind.
	/// 	The bind is continuously captured.
	/// 	Calling this function merely signals to stop further processing the inputs.
	///----------------------------------------------------------------------------------------------------
	void StartCapturing();

	///----------------------------------------------------------------------------------------------------
	/// EndCapturing:
	/// 	Ends capturing the held InputBind.
	///----------------------------------------------------------------------------------------------------
	void EndCapturing();

	///----------------------------------------------------------------------------------------------------
	/// GetCapture:
	/// 	Gets which InputBind is currently held.
	///----------------------------------------------------------------------------------------------------
	InputBind GetCapture() const;

	protected:
	///----------------------------------------------------------------------------------------------------
	/// ProcessInput:
	/// 	Returns true, if the input should not be further processed.
	///----------------------------------------------------------------------------------------------------
	bool ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam);

	///----------------------------------------------------------------------------------------------------
	/// GetCaptureRef:
	/// 	Gets which InputBind is currently held.
	/// 	Not threadsafe. Only use during WndProc.
	///----------------------------------------------------------------------------------------------------
	const InputBind& GetCaptureRef() const;

	private:
	bool      IsCapturing = false;
	InputBind Capture     = {};

	bool      IsAltHeld   = false;
	bool      IsCtrlHeld  = false;
	bool      IsShiftHeld = false;
};

#endif
