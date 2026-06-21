///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  UiFuncDefs.h
/// Description  :  Function definitions for the UI.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "UiEnum.h"

typedef void (*GUI_RENDER)   ();
typedef void (*GUI_ADDRENDER)(ERenderType aRenderType, GUI_RENDER aRenderCallback);
typedef void (*GUI_REMRENDER)(GUI_RENDER aRenderCallback);
