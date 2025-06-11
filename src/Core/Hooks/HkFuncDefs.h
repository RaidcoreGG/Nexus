///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  HkFuncDefs.h
/// Description  :  Function definitions for hooked/detoured functions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef HKFUNCDEFS_H
#define HKFUNCDEFS_H

#include <dxgi.h>

typedef HRESULT (__stdcall* DXPRESENT)      (IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags);
typedef HRESULT (__stdcall* DXRESIZEBUFFERS)(IDXGISwapChain* pChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);

#endif
