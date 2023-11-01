#ifndef KEYSTROKEMESSAGEFLAGS_H
#define KEYSTROKEMESSAGEFLAGS_H

#include <Windows.h>

typedef struct KeystrokeMessageFlags
{
	unsigned RepeatCount : 16;
	unsigned ScanCode : 8;
	unsigned ExtendedFlag : 1;
	unsigned Reserved : 4;
	unsigned ContextCode : 1;
	unsigned PreviousKeyState : 1;
	unsigned TransitionState : 1;

	unsigned short GetScanCode()
	{
		unsigned short ret = ScanCode;

		if (ExtendedFlag)
		{
			ret |= 0xE000;
		}

		return ret;
	}
} KeyLParam;

static KeystrokeMessageFlags& LParamToKMF(LPARAM& lp)
{
	return *(KeystrokeMessageFlags*)&lp;
}

static LPARAM& KMFToLParam(KeystrokeMessageFlags& kmf)
{
	return *(LPARAM*)&kmf;
}
#endif