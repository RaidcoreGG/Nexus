///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Inputs.cpp
/// Description  :  Contains a variety of utility for WndProc/Inputs.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Inputs.h"

unsigned short KeystrokeMessageFlags::GetScanCode()
{
	unsigned short ret = ScanCode;

	if (ExtendedFlag)
	{
		ret |= 0xE000;
	}

	return ret;
}

KeystrokeMessageFlags::KeystrokeMessageFlags(LPARAM aLParam)
{
	/* direct assignment is not possible, so we cast and then copy */
	KeystrokeMessageFlags tmp = *(KeystrokeMessageFlags*)&aLParam;

	RepeatCount			= tmp.RepeatCount;
	ScanCode			= tmp.ScanCode;
	ExtendedFlag		= tmp.ExtendedFlag;
	Reserved			= tmp.Reserved;
	ContextCode			= tmp.ContextCode;
	PreviousKeyState	= tmp.PreviousKeyState;
	TransitionState		= tmp.TransitionState;
}

KeystrokeMessageFlags& LParamToKMF(LPARAM& aLParam)
{
	return *(KeystrokeMessageFlags*)&aLParam;
}

LPARAM& KMFToLParam(KeystrokeMessageFlags& aKmf)
{
	return *(LPARAM*)&aKmf;
}

LPARAM GetKeyMessageLPARAM(unsigned aVKey, bool aIsDown, bool aIsSystem)
{
	KeyLParam lp;

	UINT sc = MapVirtualKeyA(aVKey, MAPVK_VK_TO_VSC_EX);

	lp.RepeatCount = 1;
	lp.ScanCode = LOBYTE(sc);
	lp.ExtendedFlag = (HIBYTE(sc) == 0xE0 || HIBYTE(sc) == 0xE1) ? 1 : 0;
	lp.Reserved = 0;
	lp.ContextCode = 0;//aIsSystem ? 1 : 0;
	lp.PreviousKeyState = aIsDown ? 0 : 1;
	lp.TransitionState = aIsDown ? 0 : 1;

	return KMFToLParam(lp);
}

LPARAM GetKeyMessageLPARAM_ScanCode(unsigned short aScanCode, bool aIsDown, bool aIsSystem)
{
	KeyLParam lp = GetKeyMessageLPARAM(0, aIsDown, aIsSystem);
	lp.ScanCode = aScanCode;
	lp.ExtendedFlag = (HIBYTE(aScanCode) == 0xE0 || HIBYTE(aScanCode) == 0xE1) ? 1 : 0;
	return KMFToLParam(lp);
}

WPARAM GetMouseMessageWPARAM(EMouseButtons aMouseButton, bool aIsCtrl, bool aIsShift, bool aIsDown)
{
	int lo = 0;
	int ho = 0;

	switch (aMouseButton)
	{
		case EMouseButtons::LMB:
			lo = aIsDown ? MK_LBUTTON : 0;
			break;
		case EMouseButtons::RMB:
			lo = aIsDown ? MK_RBUTTON : 0;
			break;
		case EMouseButtons::MMB:
			lo = aIsDown ? MK_MBUTTON : 0;
			break;
		case EMouseButtons::M4:
			lo = aIsDown ? MK_XBUTTON1 : 0;
			ho = XBUTTON1;
			break;
		case EMouseButtons::M5:
			lo = aIsDown ? MK_XBUTTON2 : 0;
			ho = XBUTTON2;
			break;
	}

	return MAKEWPARAM(
		(
			(aIsCtrl ? MK_CONTROL : 0) |
			(aIsShift ? MK_SHIFT : 0) |
			(lo)
		),
		(
			(ho)
		)
	);
}

namespace Inputs
{
	bool IsCursorHidden()
	{
		CURSORINFO curInfo{};
		curInfo.cbSize = sizeof(CURSORINFO);
		GetCursorInfo(&curInfo);
		return !(curInfo.flags & CURSOR_SHOWING);
	}
}
