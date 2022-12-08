#include <iostream>
#include <Shlwapi.h>
#include <string>
#include <d3d11.h>
#include <PathCch.h>
#include <cassert>

bool g_IsChainloading = false;
HMODULE g_Self;
HMODULE g_D3D11;
HMODULE g_Sys11;
ID3D11Device* g_D3DDev;
int bounce = 0;

static BOOL FindFunction(HMODULE mod, LPVOID func, LPCSTR name)
{
	FARPROC* fp = (FARPROC*)func;
	*fp = mod ? GetProcAddress(mod, name) : 0;
	return (fp != 0);
}

BOOL DxLoad()
{
	static bool attemped;

	if (!attemped)
	{
		attemped = true;

		/* attempt to chainload */
		WCHAR chainload[MAX_PATH];
		GetModuleFileNameW(g_Self, chainload, sizeof(chainload));
		PathCchRemoveFileSpec(chainload, sizeof(chainload));
		PathCchAppend(chainload, sizeof(chainload), L"d3d11_chainload.dll");
		g_D3D11 = LoadLibraryW(chainload);
		if (g_D3D11 == g_Self) { g_D3D11 = NULL; } /* don't load yourself lol */

		/* ifn chainload, load system dll */
		if (g_D3D11 == FALSE)
		{
			WCHAR system[MAX_PATH];
			GetSystemDirectoryW(system, ARRAYSIZE(system));
			PathCchAppend(system, sizeof(system), L"d3d11.dll");
			g_D3D11 = LoadLibraryW(system);

			assert(g_D3D11 && "Could neither chainload, nor load system DLL.");

			std::cout << "Loaded System DLL." << std::endl;

			return true;
		}
		else
		{
			g_IsChainloading = true;
			std::cout << "Chainloading: ";
			std::wcout << chainload << std::endl;
		}
	}

	return (g_D3D11 != NULL);
}

#pragma region dx11
extern "C" HRESULT WINAPI D3D11CoreCreateDevice(IDXGIFactory * pFactory,
										IDXGIAdapter* pAdapter,
										UINT Flags,
										const D3D_FEATURE_LEVEL* pFeatureLevels,
										UINT FeatureLevels,
										ID3D11Device &ppDevice	)
{
	static decltype(&D3D11CoreCreateDevice) func;

	std::cout << "D3D11CoreCreateDevice" << std::endl;

	if (DxLoad() == false) { return 0; }

	if (func == 0)
	{
		if (FindFunction(g_D3D11, &func, "D3D11CoreCreateDevice") == false)
		{
			return 0;
		}
	}

	return func(pFactory, pAdapter, Flags, pFeatureLevels, FeatureLevels, ppDevice);
}
extern "C" HRESULT WINAPI D3D11CreateDevice(	IDXGIAdapter* pAdapter,
												D3D_DRIVER_TYPE DriverType,
												HMODULE Software,
												UINT Flags,
												const D3D_FEATURE_LEVEL* pFeatureLevels,
												UINT FeatureLevels,
												UINT SDKVersion,
												ID3D11Device** ppDevice,
												D3D_FEATURE_LEVEL* pFeatureLevel,
												ID3D11DeviceContext** ppImmediateContext	)
{
	static decltype(&D3D11CreateDevice) func;

	std::cout << "D3D11CreateDevice" << std::endl;

	bounce++;
	static bool bounced = false;

	if (bounce >= 3)
	{
		WCHAR system[MAX_PATH];
		GetSystemDirectoryW(system, ARRAYSIZE(system));
		PathCchAppend(system, sizeof(system), L"d3d11.dll");
		g_Sys11 = LoadLibraryW(system);

		if (FindFunction(g_Sys11, &func, "D3D11CreateDevice") == false)
		{
			return 0;
		}

		bounced = true;
	}

	if (DxLoad() == false)
	{
		return 0;
	}

	if (func == 0)
	{
		if (FindFunction(g_D3D11, &func, "D3D11CreateDevice") == false)
		{
			return 0;
		}
	}

	return func(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
}
extern "C" HRESULT WINAPI D3D11CreateDeviceAndSwapChain(	IDXGIAdapter* pAdapter,
															D3D_DRIVER_TYPE DriverType,
															HMODULE Software,
															UINT Flags,
															const D3D_FEATURE_LEVEL* pFeatureLevels,
															UINT FeatureLevels,
															UINT SDKVersion,
															const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
															IDXGISwapChain** ppSwapChain,
															ID3D11Device** ppDevice,
															D3D_FEATURE_LEVEL* pFeatureLevel,
															ID3D11DeviceContext** ppImmediateContext	)
{
	static decltype(&D3D11CreateDeviceAndSwapChain) func;

	std::cout << "D3D11CreateDeviceAndSwapChain" << std::endl;

	if (DxLoad() == false) { return 0; }

	if (func == 0)
	{
		if (FindFunction(g_D3D11, &func, "D3D11CreateDeviceAndSwapChain") == false)
		{
			return 0;
		}
	}

	return func(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);
}
#pragma endregion

FILE* iobuf;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	g_Self = hModule;

	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			AllocConsole();
			freopen_s(&iobuf, "CONIN$", "r", stdin);
			freopen_s(&iobuf, "CONOUT$", "w", stderr);
			freopen_s(&iobuf, "CONOUT$", "w", stdout);
			static char path[MAX_PATH];
			GetModuleFileName(hModule, path, sizeof(path));
			std::cout << "[ATTACH] " << path << std::endl;
			break;
		case DLL_PROCESS_DETACH:
			std::cout << "[DETACH] " << path << std::endl;
			FreeConsole();
			if (g_D3D11) { FreeLibrary(g_D3D11); }
			break;
	}
	return true;
}
