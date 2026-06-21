///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  PxyFuncDefs.h
/// Description  :  Function definitions for proxy functions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <windows.h>
#include <dxgi.h>
#include <d3d11.h>

// CreateDevice defined in d3d11.h
// CreateDeviceAndSwapChain defined in d3d11.h
typedef HRESULT(WINAPI* PFN_D3D11_CORE_CREATE_DEVICE)          (IDXGIFactory*, IDXGIAdapter*, UINT, const D3D_FEATURE_LEVEL*, UINT, ID3D11Device**);
typedef HRESULT(WINAPI* PFN_D3D11_CORE_CREATE_LAYERED_DEVICE)  (const void*, DWORD, const void*, REFIID, void**);
typedef HRESULT(WINAPI* PFN_D3D11_CORE_GET_LAYERED_DEVICE_SIZE)(const void*, DWORD);
typedef HRESULT(WINAPI* PFN_D3D11_CORE_REGISTER_LAYERS)        (const void*, DWORD);
