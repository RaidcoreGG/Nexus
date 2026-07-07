///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  PxyDXGI.h
/// Description  :  Implementation of proxy functions for DXGI.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <windows.h>

#include "Proxy.h"

///----------------------------------------------------------------------------------------------------
/// (PROXY) CreateDXGIFactory
///----------------------------------------------------------------------------------------------------
PROXY HRESULT WINAPI CreateDXGIFactory(
	REFIID riid,
	void** ppFactory
);

///----------------------------------------------------------------------------------------------------
/// (PROXY) CreateDXGIFactory1
///----------------------------------------------------------------------------------------------------
PROXY HRESULT WINAPI CreateDXGIFactory1(
	REFIID riid,
	void** ppFactory
);

///----------------------------------------------------------------------------------------------------
/// (PROXY) CreateDXGIFactory2
///----------------------------------------------------------------------------------------------------
PROXY HRESULT WINAPI CreateDXGIFactory2(
	UINT   Flags,
	REFIID riid,
	void** ppFactory
);

///----------------------------------------------------------------------------------------------------
/// (PROXY) DXGIGetDebugInterface1
///----------------------------------------------------------------------------------------------------
PROXY HRESULT WINAPI DXGIGetDebugInterface1(
	UINT   Flags,
	REFIID riid,
	void** ppDebug
);

///----------------------------------------------------------------------------------------------------
/// (PROXY) DXGIDeclareAdapterRemovalSupport
///----------------------------------------------------------------------------------------------------
PROXY HRESULT WINAPI DXGIDeclareAdapterRemovalSupport();
