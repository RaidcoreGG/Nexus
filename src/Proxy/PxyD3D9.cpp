///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  PxyD3D9.cpp
/// Description  :  Implementation of proxy functions for D3D9.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "PxyD3D9.h"

#include <d3d9.h>
#include <d3d9types.h>
#include <windows.h>

#include "Proxy.h"
#include "PxyEnum.h"
#include "Runtime/Runtime.h"

static ProxyModule_t s_ProxyModule{};

#define PROXY_MODULE_NAME "d3d9.dll"

PROXY IDirect3D9* WINAPI Direct3DCreate9(
	UINT SDKVersion
)
{
	s_ProxyModule.Init(PROXY_MODULE_NAME);
	Runtime::Get().Initialize(EProxyFunction::D3D9_DIRECT3DCREATE9);

	static thread_local bool s_InProxyCall = false;

	auto fn = s_ProxyModule.GetFunc<decltype(&Direct3DCreate9)>("Direct3DCreate9", s_InProxyCall);

	PROTECT_RECURSE(s_InProxyCall);

	return fn(SDKVersion);
}

PROXY HRESULT WINAPI Direct3DCreate9Ex(
	UINT           SDKVersion,
	IDirect3D9Ex** ppD3D
)
{
	s_ProxyModule.Init(PROXY_MODULE_NAME);
	Runtime::Get().Initialize(EProxyFunction::D3D9_DIRECT3DCREATE9EX);

	static thread_local bool s_InProxyCall = false;

	auto fn = s_ProxyModule.GetFunc<decltype(&Direct3DCreate9Ex)>("Direct3DCreate9Ex", s_InProxyCall);

	PROTECT_RECURSE(s_InProxyCall);

	return fn(
		SDKVersion,
		ppD3D
	);
}

PROXY int WINAPI D3DPERF_BeginEvent(
	D3DCOLOR col,
	LPCWSTR  wszName
)
{
	s_ProxyModule.Init(PROXY_MODULE_NAME);
	Runtime::Get().Initialize(EProxyFunction::D3D9_D3DPERF_BEGINEVENT);

	static thread_local bool s_InProxyCall = false;

	auto fn = s_ProxyModule.GetFunc<decltype(&D3DPERF_BeginEvent)>("D3DPERF_BeginEvent", s_InProxyCall);

	PROTECT_RECURSE(s_InProxyCall);

	return fn(col, wszName);
}

PROXY int WINAPI D3DPERF_EndEvent()
{
	s_ProxyModule.Init(PROXY_MODULE_NAME);
	Runtime::Get().Initialize(EProxyFunction::D3D9_D3DPERF_ENDEVENT);

	static thread_local bool s_InProxyCall = false;

	auto fn = s_ProxyModule.GetFunc<decltype(&D3DPERF_EndEvent)>("D3DPERF_EndEvent", s_InProxyCall);

	PROTECT_RECURSE(s_InProxyCall);

	return fn();
}

PROXY void WINAPI D3DPERF_SetMarker(
	D3DCOLOR col,
	LPCWSTR  wszName
)
{
	s_ProxyModule.Init(PROXY_MODULE_NAME);
	Runtime::Get().Initialize(EProxyFunction::D3D9_D3DPERF_SETMARKER);

	static thread_local bool s_InProxyCall = false;

	auto fn = s_ProxyModule.GetFunc<decltype(&D3DPERF_SetMarker)>("D3DPERF_SetMarker", s_InProxyCall);

	PROTECT_RECURSE(s_InProxyCall);

	fn(col, wszName);
}

PROXY void WINAPI D3DPERF_SetRegion(
	D3DCOLOR col,
	LPCWSTR  wszName
)
{
	s_ProxyModule.Init(PROXY_MODULE_NAME);
	Runtime::Get().Initialize(EProxyFunction::D3D9_D3DPERF_SETREGION);

	static thread_local bool s_InProxyCall = false;

	auto fn = s_ProxyModule.GetFunc<decltype(&D3DPERF_SetRegion)>("D3DPERF_SetRegion", s_InProxyCall);

	PROTECT_RECURSE(s_InProxyCall);

	fn(col, wszName);
}

PROXY BOOL WINAPI D3DPERF_QueryRepeatFrame()
{
	s_ProxyModule.Init(PROXY_MODULE_NAME);
	Runtime::Get().Initialize(EProxyFunction::D3D9_D3DPERF_QUERYREPEATFRAME);

	static thread_local bool s_InProxyCall = false;

	auto fn = s_ProxyModule.GetFunc<decltype(&D3DPERF_QueryRepeatFrame)>("D3DPERF_QueryRepeatFrame", s_InProxyCall);

	PROTECT_RECURSE(s_InProxyCall);

	return fn();
}

PROXY void WINAPI D3DPERF_SetOptions(
	DWORD dwOptions
)
{
	s_ProxyModule.Init(PROXY_MODULE_NAME);
	Runtime::Get().Initialize(EProxyFunction::D3D9_D3DPERF_SETOPTIONS);

	static thread_local bool s_InProxyCall = false;

	auto fn = s_ProxyModule.GetFunc<decltype(&D3DPERF_SetOptions)>("D3DPERF_SetOptions", s_InProxyCall);

	PROTECT_RECURSE(s_InProxyCall);

	fn(dwOptions);
}

PROXY DWORD WINAPI D3DPERF_GetStatus()
{
	s_ProxyModule.Init(PROXY_MODULE_NAME);
	Runtime::Get().Initialize(EProxyFunction::D3D9_D3DPERF_GETSTATUS);

	static thread_local bool s_InProxyCall = false;

	auto fn = s_ProxyModule.GetFunc<decltype(&D3DPERF_GetStatus)>("D3DPERF_GetStatus", s_InProxyCall);

	PROTECT_RECURSE(s_InProxyCall);

	return fn();
}
