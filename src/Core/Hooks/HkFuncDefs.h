///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  HkFuncDefs.h
/// Description  :  Function definitions for hooked/detoured functions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_4.h>
#include <dxgiformat.h>
#include <windows.h>

typedef HRESULT (__stdcall* DXPRESENT)       (IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags);
typedef HRESULT (__stdcall* DXPRESENT1)      (IDXGISwapChain1* pChain, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters);
typedef HRESULT (__stdcall* DXRESIZEBUFFERS) (IDXGISwapChain* pChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
typedef HRESULT (__stdcall* DXRESIZEBUFFERS1)(IDXGISwapChain3* pChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT Format, UINT SwapChainFlags, const UINT* pCreationNodeMask, IUnknown* const* ppPresentQueue);
