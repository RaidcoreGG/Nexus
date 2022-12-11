#include <iostream>
#include <Shlwapi.h>
#include <string>
#include <d3d11.h>
#include <PathCch.h>
#include <cassert>
#include <thread>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

#define CHAINLOAD_HACK

FILE* iobuf;

static WCHAR g_Path_HostDirectory[512];
static WCHAR g_Path_HostDll[512];
static WCHAR g_Path_TempDll[512];
static WCHAR g_Path_ChainloadDll[512];

static bool g_IsChainloading = false;
static bool g_IsLoaded = false;
static bool g_IsCreated = false;

static HMODULE g_GW2;
static HMODULE g_Self;
static HMODULE g_D3D11;
static HMODULE g_Sys11;

static BOOL FindFunction(HMODULE mod, LPVOID func, LPCSTR name)
{
	FARPROC* fp = (FARPROC*)func;
	*fp = mod ? GetProcAddress(mod, name) : 0;
	return (fp != 0);
}

BOOL DxLoad()
{
	if (!g_IsLoaded)
	{
		g_IsLoaded = true;

#ifdef CHAINLOAD_HACK
		/* attempt to chainload */
		/* sanity check that the current dll isn't the chainload */
		if (g_Path_HostDll != g_Path_ChainloadDll)
		{
			g_IsChainloading = true;

			g_D3D11 = LoadLibraryW(g_Path_ChainloadDll);

			std::cout << "Attempting to chainload: ";
			std::wcout << g_Path_ChainloadDll << std::endl;
		}
#endif

		/* ifn chainload, load system dll */
		if (!g_D3D11)
		{
			g_IsChainloading = false;

			std::cout << "No chainload found or invalid." << std::endl;

			WCHAR path[512];
			GetSystemDirectoryW(path, sizeof(path));
			PathCchAppend(path, sizeof(path), L"d3d11.dll");
			g_D3D11 = LoadLibraryW(path);

			assert(g_D3D11 && "Could neither chainload, nor load system DLL.");

			std::cout << "Loaded System DLL." << std::endl;
		}
	}

	return (g_D3D11 != NULL);
}

#pragma region dx11
extern "C" HRESULT WINAPI D3D11CoreCreateDevice(IDXGIFactory * pFactory, IDXGIAdapter * pAdapter, UINT Flags, const D3D_FEATURE_LEVEL * pFeatureLevels, UINT FeatureLevels, ID3D11Device & ppDevice)
{
	static decltype(&D3D11CoreCreateDevice) func;
	static const char* func_name = "D3D11CoreCreateDevice";

	std::cout << func_name << std::endl;

#ifdef CHAINLOAD_HACK
    if (g_IsCreated)
    {
        WCHAR system[MAX_PATH];
        GetSystemDirectoryW(system, ARRAYSIZE(system));
        PathCchAppend(system, sizeof(system), L"d3d11.dll");
        g_Sys11 = LoadLibraryW(system);

        if (FindFunction(g_Sys11, &func, func_name) == false)
        {
            return 0;
        }
    }

    g_IsCreated = true;
#endif

	if (DxLoad() == false) { return 0; }

	if (func == 0)
	{
		if (FindFunction(g_D3D11, &func, func_name) == false)
		{
			return 0;
		}
	}

	return func(pFactory, pAdapter, Flags, pFeatureLevels, FeatureLevels, ppDevice);
}
extern "C" HRESULT WINAPI D3D11CreateDevice(IDXGIAdapter * pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL * pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, ID3D11Device * *ppDevice, D3D_FEATURE_LEVEL * pFeatureLevel, ID3D11DeviceContext * *ppImmediateContext)
{
	static decltype(&D3D11CreateDevice) func;
	static const char* func_name = "D3D11CreateDevice";

	std::cout << func_name << std::endl;

#ifdef CHAINLOAD_HACK
	if (g_IsCreated)
	{
		WCHAR system[MAX_PATH];
		GetSystemDirectoryW(system, ARRAYSIZE(system));
		PathCchAppend(system, sizeof(system), L"d3d11.dll");
		g_Sys11 = LoadLibraryW(system);

		if (FindFunction(g_Sys11, &func, func_name) == false)
		{
			return 0;
		}
	}

	g_IsCreated = true;
#endif

	if (DxLoad() == false) { return 0; }

	if (func == 0)
	{
		if (FindFunction(g_D3D11, &func, func_name) == false)
		{
			return 0;
		}
	}

	return func(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
}
extern "C" HRESULT WINAPI D3D11CreateDeviceAndSwapChain(IDXGIAdapter * pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL * pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, const DXGI_SWAP_CHAIN_DESC * pSwapChainDesc, IDXGISwapChain * *ppSwapChain, ID3D11Device * *ppDevice, D3D_FEATURE_LEVEL * pFeatureLevel, ID3D11DeviceContext * *ppImmediateContext)
{
	static decltype(&D3D11CreateDeviceAndSwapChain) func;
	static const char* func_name = "D3D11CreateDeviceAndSwapChain";

	std::cout << func_name << std::endl;

#ifdef CHAINLOAD_HACK
    if (g_IsCreated)
    {
        WCHAR system[MAX_PATH];
        GetSystemDirectoryW(system, ARRAYSIZE(system));
        PathCchAppend(system, sizeof(system), L"d3d11.dll");
        g_Sys11 = LoadLibraryW(system);

        if (FindFunction(g_Sys11, &func, func_name) == false)
        {
            return 0;
        }
    }

    g_IsCreated = true;
#endif

	if (DxLoad() == false) { return 0; }

	if (func == 0)
	{
		if (FindFunction(g_D3D11, &func, func_name) == false)
		{
			return 0;
		}
	}

	return func(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);
}
#pragma endregion

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_Self = hModule;
        g_GW2 = GetModuleHandle(NULL);

		AllocConsole();
		freopen_s(&iobuf, "CONIN$", "r", stdin);
		freopen_s(&iobuf, "CONOUT$", "w", stderr);
		freopen_s(&iobuf, "CONOUT$", "w", stdout);

		/* get self dll path */
		GetModuleFileNameW(hModule, g_Path_HostDll, sizeof(g_Path_HostDll));

		/* get current directory */
		memcpy(g_Path_HostDirectory, g_Path_HostDll, sizeof(g_Path_HostDirectory));
		PathCchRemoveFileSpec(g_Path_HostDirectory, sizeof(g_Path_HostDirectory));

		/* get temp dll path */
		memcpy(g_Path_TempDll, g_Path_HostDirectory, sizeof(g_Path_TempDll));
		PathCchAppend(g_Path_TempDll, sizeof(g_Path_TempDll), L"d3d11.tmp");

		/* get chainload dll path */
		memcpy(g_Path_ChainloadDll, g_Path_HostDirectory, sizeof(g_Path_ChainloadDll));
		PathCchAppend(g_Path_ChainloadDll, sizeof(g_Path_ChainloadDll), L"d3d11_chainload.dll");

		std::wcout << L"[ATTACH] " << g_Path_HostDll << std::endl;
		break;
	case DLL_PROCESS_DETACH:
		std::wcout << L"[DETACH] " << g_Path_HostDll << std::endl;

		FreeConsole();

		if (g_D3D11) { FreeLibrary(g_D3D11); }
		break;
	}
	return true;
}
