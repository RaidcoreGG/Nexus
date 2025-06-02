///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  RdrWindow.h
/// Description  :  Definition for the renderer window.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef RDRWINDOW_H
#define RDRWINDOW_H

#include <windows.h>
#include <cstdint>

///----------------------------------------------------------------------------------------------------
/// RenderWindow_t Struct
///----------------------------------------------------------------------------------------------------
struct RenderWindow_t
{
	HWND     Handle = nullptr;
	uint32_t Width  = 0;
	uint32_t Height = 0;
};

#endif
