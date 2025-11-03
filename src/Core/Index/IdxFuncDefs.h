///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  IdxFuncDefs.h
/// Description  :  Function definitions for the Index.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

typedef const char* (*IDX_GETGAMEDIR)  ();
typedef const char* (*IDX_GETADDONDIR) (const char* aName);
typedef const char* (*IDX_GETCOMMONDIR)();
