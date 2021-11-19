#include <iostream>
#include <Shlwapi.h>
#include <string>
#include <d3d9.h>
#include <d3d11.h>

HMODULE d3d9;
HMODULE d3d11;
int dxlevel;
void* d3ddev; // d3ddev is IDirect3D9* if dxlevel==9, or ID3D11Device* if dxlevel==11

static BOOL find_func(HMODULE mod, LPVOID func, LPCSTR name) {
	FARPROC* fp = (FARPROC*)func;
	*fp = mod ? GetProcAddress(mod, name) : 0;
	return (fp != 0);
}

BOOL dx_load(int dxlvl) {
	switch (dxlvl)
	{
	case 9:
		if (d3d9 == FALSE) {
			WCHAR path[MAX_PATH];
			GetSystemDirectoryW(path, ARRAYSIZE(path));
			PathAppendW(path, L"d3d9.dll");
			d3d9 = LoadLibraryW(path);
		}
		return TRUE;
	case 11:
		if (d3d11 == FALSE) {
			WCHAR path[MAX_PATH];
			GetSystemDirectoryW(path, ARRAYSIZE(path));
			PathAppendW(path, L"d3d11.dll");
			d3d11 = LoadLibraryW(path);
		}
		return TRUE;
	}

	return FALSE;
}

#pragma region dx9
IDirect3D9* WINAPI Direct3DCreate9(UINT SDKVersion) {
	static decltype(&Direct3DCreate9) func;

	std::cout << "Direct3DCreate9" << std::endl;

	if (dx_load(9) == FALSE) {
		return 0;
	}

	if (func == 0) {
		if (find_func(d3d9, &func, "Direct3DCreate9") == FALSE) {
			return 0;
		}
	}

	dxlevel = 9;
	std::cout << "dxlevel = 9" << std::endl;

	return func(SDKVersion);
}
HRESULT WINAPI Direct3DCreate9Ex(UINT SDKVersion, IDirect3D9Ex** ppD3D) {
	static decltype(&Direct3DCreate9Ex) func;

	std::cout << "Direct3DCreate9Ex" << std::endl;

	if (dx_load(9) == FALSE) {
		return D3DERR_NOTAVAILABLE;
	}

	if (func == 0) {
		if (find_func(d3d9, &func, "Direct3DCreate9Ex") == FALSE) {
			return D3DERR_NOTAVAILABLE;
		}
	}

	dxlevel = 9;
	std::cout << "dxlevel = 9" << std::endl;

	return func(SDKVersion, ppD3D);
}
int WINAPI D3DPERF_BeginEvent(D3DCOLOR col, LPCWSTR wszName) {
	static decltype(&D3DPERF_BeginEvent) func;

	std::cout << "D3DPERF_BeginEvent" << std::endl;

	if (dx_load(9) == FALSE) {
		return D3DERR_NOTAVAILABLE;
	}

	if (func == 0) {
		if (find_func(d3d9, &func, "D3DPERF_BeginEvent") == FALSE) {
			return D3DERR_NOTAVAILABLE;
		}
	}

	return func(col, wszName);
}
int WINAPI D3DPERF_EndEvent(void) {
	static decltype(&D3DPERF_EndEvent) func;

	std::cout << "D3DPERF_EndEvent" << std::endl;

	if (dx_load(9) == FALSE) {
		return D3DERR_NOTAVAILABLE;
	}

	if (func == 0) {
		if (find_func(d3d9, &func, "D3DPERF_EndEvent") == FALSE) {
			return D3DERR_NOTAVAILABLE;
		}
	}

	return func();
}
void WINAPI D3DPERF_SetMarker(D3DCOLOR col, LPCWSTR wszName) {
	static decltype(&D3DPERF_SetMarker) func;

	std::cout << "D3DPERF_SetMarker" << std::endl;

	if (dx_load(9) == FALSE) {
		return;
	}

	if (func == 0) {
		if (find_func(d3d9, &func, "D3DPERF_SetMarker") == FALSE) {
			return;
		}
	}

	func(col, wszName);
}
void WINAPI D3DPERF_SetRegion(D3DCOLOR col, LPCWSTR wszName) {
	static decltype(&D3DPERF_SetRegion) func;

	std::cout << "D3DPERF_SetRegion" << std::endl;

	if (dx_load(9) == FALSE) {
		return;
	}

	if (func == 0) {
		if (find_func(d3d9, &func, "D3DPERF_SetRegion") == FALSE) {
			return;
		}
	}
}
BOOL WINAPI D3DPERF_QueryRepeatFrame(void) {
	static decltype(&D3DPERF_EndEvent) func;

	std::cout << "D3DPERF_QueryRepeatFrame" << std::endl;

	if (dx_load(9) == FALSE) {
		return FALSE;
	}

	if (func == 0) {
		if (find_func(d3d9, &func, "D3DPERF_QueryRepeatFrame") == FALSE) {
			return FALSE;
		}
	}

	return func();
}
void WINAPI D3DPERF_SetOptions(DWORD dwOptions) {
	static decltype(&D3DPERF_SetOptions) func;

	std::cout << "D3DPERF_SetOptions" << std::endl;

	if (dx_load(9) == FALSE) {
		return;
	}

	if (func == 0) {
		if (find_func(d3d9, &func, "D3DPERF_SetOptions") == FALSE) {
			return;
		}
	}

	func(dwOptions);
}
DWORD WINAPI D3DPERF_GetStatus(void) {
	static decltype(&D3DPERF_GetStatus) func;

	std::cout << "D3DPERF_GetStatus" << std::endl;

	if (dx_load(9) == FALSE) {
		return 0;
	}

	if (func == 0) {
		if (find_func(d3d9, &func, "D3DPERF_GetStatus") == FALSE) {
			return 0;
		}
	}

	return func();
}
#pragma endregion

#pragma region dx11
HRESULT WINAPI D3D11CoreCreateDevice(IDXGIFactory* pFactory, IDXGIAdapter* pAdapter, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, ID3D11Device &ppDevice) {
	static decltype(&D3D11CoreCreateDevice) func;

	std::cout << "D3D11CoreCreateDevice" << std::endl;

	if (dx_load(11) == FALSE) {
		return 0;
	}

	if (func == 0) {
		if (find_func(d3d11, &func, "D3D11CoreCreateDevice") == FALSE) {
			return 0;
		}
	}

	dxlevel = 11;
	std::cout << "dxlevel = 11" << std::endl;

	return func(pFactory, pAdapter, Flags, pFeatureLevels, FeatureLevels, ppDevice);
}
HRESULT WINAPI D3D11CreateDevice(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext) {
	static decltype(&D3D11CreateDevice) func;

	std::cout << "D3D11CreateDevice" << std::endl;

	if (dx_load(11) == FALSE) {
		return 0;
	}

	if (func == 0) {
		if (find_func(d3d11, &func, "D3D11CreateDevice") == FALSE) {
			return 0;
		}
	}

	dxlevel = 11;
	std::cout << "dxlevel = 11" << std::endl;

	return func(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
}
HRESULT WINAPI D3D11CreateDeviceAndSwapChain(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, IDXGISwapChain** ppSwapChain, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext) {
	static decltype(&D3D11CreateDeviceAndSwapChain) func;

	std::cout << "D3D11CreateDeviceAndSwapChain" << std::endl;

	if (dx_load(11) == FALSE) {
		return 0;
	}

	if (func == 0) {
		if (find_func(d3d11, &func, "D3D11CreateDeviceAndSwapChain") == FALSE) {
			return 0;
		}
	}

	dxlevel = 11;
	std::cout << "dxlevel = 11" << std::endl;

	return func(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);
}
#pragma endregion

FILE* iobuf;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
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
		if (d3d9) { FreeLibrary(d3d9); }
		if (d3d11) { FreeLibrary(d3d11); }
		break;
	}
	return TRUE;
}
