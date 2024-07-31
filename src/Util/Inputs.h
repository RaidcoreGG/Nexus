///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Inputs.h
/// Description  :  Contains a variety of utility for WndProc/Inputs.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef INPUTS_H
#define INPUTS_H

#include <Windows.h>

enum class EMouseButtons
{
	None,
	LMB,
	RMB,
	MMB,
	M4,
	M5
};

///----------------------------------------------------------------------------------------------------
/// KeystrokeMessageFlags/KeyLParam struct
///----------------------------------------------------------------------------------------------------
typedef struct KeystrokeMessageFlags
{
	unsigned RepeatCount		: 16;
	unsigned ScanCode			: 8;
	unsigned ExtendedFlag		: 1;
	unsigned Reserved			: 4;
	unsigned ContextCode		: 1;
	unsigned PreviousKeyState	: 1;
	unsigned TransitionState	: 1;

	///----------------------------------------------------------------------------------------------------
	/// GetScanCode:
	/// 	Returns the scancode (including extended key flag) of the LParam.
	///----------------------------------------------------------------------------------------------------
	unsigned short GetScanCode();

	KeystrokeMessageFlags() = default;
	KeystrokeMessageFlags(LPARAM aLParam);
} KeyLParam;

///----------------------------------------------------------------------------------------------------
/// LParamToKMF:
/// 	Casts an LPARAM to a KeystrokeMessageFlags.
///----------------------------------------------------------------------------------------------------
KeystrokeMessageFlags& LParamToKMF(LPARAM& aLParam);

///----------------------------------------------------------------------------------------------------
/// KMFToLParam:
/// 	Casts an KeystrokeMessageFlags to a LPARAM.
///----------------------------------------------------------------------------------------------------
LPARAM& KMFToLParam(KeystrokeMessageFlags& aKmf);

///----------------------------------------------------------------------------------------------------
/// GetKeyMessageLPARAM:
/// 	Creates an LPARAM for key messages.
///----------------------------------------------------------------------------------------------------
LPARAM GetKeyMessageLPARAM(unsigned aVKey, bool aIsDown, bool aIsSystem);

#endif
