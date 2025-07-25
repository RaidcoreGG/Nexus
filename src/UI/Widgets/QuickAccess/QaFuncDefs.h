///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  QaFuncDefs.h
/// Description  :  Function definitions for Quick Access.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef QAFUNCDEFS_H
#define QAFUNCDEFS_H

#include "UI/FuncDefs.h"

typedef void (*QUICKACCESS_ADDSHORTCUT) (const char* aIdentifier, const char* aTextureIdentifier, const char* aTextureHoverIdentifier, const char* aInputBindIdentifier, const char* aTooltipText);
typedef void (*QUICKACCESS_ADDSIMPLE)   (const char* aIdentifier, GUI_RENDER aShortcutRenderCallback);
typedef void (*QUICKACCESS_ADDSIMPLE2)  (const char* aIdentifier, const char* aTargetShortcutIdentifier, GUI_RENDER aShortcutRenderCallback);
typedef void (*QUICKACCESS_GENERIC)     (const char* aIdentifier);

#endif
