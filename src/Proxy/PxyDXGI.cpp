///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  PxyDXGI.cpp
/// Description  :  Implementation of proxy functions for DXGI.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "PxyDXGI.h"

#include <windows.h>

#include "Proxy.h"
#include "PxyEnum.h"
#include "Runtime/Runtime.h"

static ProxyModule_t s_ProxyModule{};

#define PROXY_MODULE_NAME "dxgi.dll"

PROXY HRESULT __stdcall CreateDXGIFactory(REFIID riid, void** ppFactory)
{
	s_ProxyModule.Init(PROXY_MODULE_NAME);
	Runtime::Get().Initialize(EProxyFunction::DXGI_CreateDXGIFactory);

	static thread_local bool s_InProxyCall = false;

	auto fn = s_ProxyModule.GetFunc<decltype(&CreateDXGIFactory)>("CreateDXGIFactory", s_InProxyCall);

	PROTECT_RECURSE(s_InProxyCall);

	return fn(
		riid,
		ppFactory
	);
}

PROXY HRESULT __stdcall CreateDXGIFactory1(REFIID riid, void** ppFactory)
{
	s_ProxyModule.Init(PROXY_MODULE_NAME);
	Runtime::Get().Initialize(EProxyFunction::DXGI_CreateDXGIFactory1);

	static thread_local bool s_InProxyCall = false;

	auto fn = s_ProxyModule.GetFunc<decltype(&CreateDXGIFactory1)>("CreateDXGIFactory1", s_InProxyCall);

	PROTECT_RECURSE(s_InProxyCall);

	return fn(
		riid,
		ppFactory
	);
}

PROXY HRESULT __stdcall CreateDXGIFactory2(UINT Flags, REFIID riid, void** ppFactory)
{
	s_ProxyModule.Init(PROXY_MODULE_NAME);
	Runtime::Get().Initialize(EProxyFunction::DXGI_CreateDXGIFactory2);

	static thread_local bool s_InProxyCall = false;

	auto fn = s_ProxyModule.GetFunc<decltype(&CreateDXGIFactory2)>("CreateDXGIFactory2", s_InProxyCall);

	PROTECT_RECURSE(s_InProxyCall);

	return fn(
		Flags,
		riid,
		ppFactory
	);
}

PROXY HRESULT __stdcall DXGIGetDebugInterface1(UINT Flags, REFIID riid, void** ppDebug)
{
	s_ProxyModule.Init(PROXY_MODULE_NAME);
	Runtime::Get().Initialize(EProxyFunction::DXGI_DXGIGetDebugInterface1);

	static thread_local bool s_InProxyCall = false;

	auto fn = s_ProxyModule.GetFunc<decltype(&DXGIGetDebugInterface1)>("DXGIGetDebugInterface1", s_InProxyCall);

	PROTECT_RECURSE(s_InProxyCall);

	return fn(
		Flags,
		riid,
		ppDebug
	);
}

PROXY HRESULT __stdcall DXGIDeclareAdapterRemovalSupport()
{
	s_ProxyModule.Init(PROXY_MODULE_NAME);
	Runtime::Get().Initialize(EProxyFunction::DXGI_DXGIDeclareAdapterRemovalSupport);

	static thread_local bool s_InProxyCall = false;

	auto fn = s_ProxyModule.GetFunc<decltype(&DXGIDeclareAdapterRemovalSupport)>("DXGIDeclareAdapterRemovalSupport", s_InProxyCall);

	PROTECT_RECURSE(s_InProxyCall);

	return fn();
}
