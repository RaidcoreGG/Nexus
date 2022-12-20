#include <iostream>
#include <Shlwapi.h>
#include <string>
#include <d3d11.h>
#include <PathCch.h>
#include <cassert>
#include <thread>

#include "minhook/mh_hook.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

#include "Logging/LogHandler.h"
#include "Logging/ConsoleLogger.h"
#include "Logging/FileLogger.h"

#define IMPL_CHAINLOAD

static WCHAR g_Path_HostDirectory[MAX_PATH];
static WCHAR g_Path_HostDll[MAX_PATH];
static WCHAR g_Path_TempDll[MAX_PATH];
static WCHAR g_Path_ChainloadDll[MAX_PATH];
static WCHAR g_Path_SystemDll[MAX_PATH];

static bool g_IsDxLoaded = false;
static bool g_IsChainloading = false;
static bool g_IsDxCreated = false;
static bool g_IsImGuiInitialized = false;

static HMODULE g_GW2;
static HMODULE g_Self;
static HMODULE g_D3D11;
static HMODULE g_Sys11;

LogHandler* logger = LogHandler::GetInstance();;

/* meme */
void InitializeImGui();
static WNDPROC g_GW2_WndProc = nullptr;
static HWND g_GW2_HWND = nullptr;
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

static HRESULT(*dxgi_present)(IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags);
static HRESULT(*dxgi_resize_buffers)(IDXGISwapChain* pChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);

HRESULT __stdcall hkDXGIPresent(IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags);
HRESULT __stdcall hkDXGIResizeBuffers(IDXGISwapChain* pChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
/* meme */

static BOOL FindFunction(HMODULE mod, LPVOID func, LPCSTR name)
{
	FARPROC* fp = (FARPROC*)func;
	*fp = mod ? GetProcAddress(mod, name) : 0;
	return (fp != 0);
}

BOOL DxLoad()
{
	if (!g_IsDxLoaded)
	{
		g_IsDxLoaded = true;

#ifdef IMPL_CHAINLOAD
		/* attempt to chainload */
		/* sanity check that the current dll isn't the chainload */
		if (g_Path_HostDll != g_Path_ChainloadDll)
		{
			g_IsChainloading = true;

			g_D3D11 = LoadLibraryW(g_Path_ChainloadDll);

			logger->Log(L"Attempting to chainload.");
			//std::wcout << g_Path_ChainloadDll << std::endl;
		}
#endif

		/* ifn chainload, load system dll */
		if (!g_D3D11)
		{
#ifdef IMPL_CHAINLOAD
			if (g_IsChainloading)
			{
				logger->Log(L"No chainload found or failed to load.");
			}
#endif
			g_IsChainloading = false;

			WCHAR path[512];
			GetSystemDirectoryW(path, sizeof(path));
			PathCchAppend(path, sizeof(path), L"d3d11.dll");
			g_D3D11 = LoadLibraryW(path);

			assert(g_D3D11 && "Could not load system d3d11.dll");

			logger->Log(L"Loaded System DLL.");
		}
	}

	return (g_D3D11 != NULL);
}
#pragma region dx11
HRESULT __stdcall D3D11CoreCreateDevice(IDXGIFactory * pFactory, IDXGIAdapter * pAdapter, UINT Flags, const D3D_FEATURE_LEVEL * pFeatureLevels, UINT FeatureLevels, ID3D11Device & ppDevice)
{
	static decltype(&D3D11CoreCreateDevice) func;
	static const char* func_name = "D3D11CoreCreateDevice";

	logger->Log(func_name);

#ifdef IMPL_CHAINLOAD
	if (g_IsDxCreated)
	{
		logger->LogWarning(L"DirectX Create already called. Chainload bounced back. Loading System d3d11.dll");

		g_Sys11 = LoadLibraryW(g_Path_SystemDll);

		if (FindFunction(g_Sys11, &func, func_name) == false)
		{
			return 0;
		}
	}

	g_IsDxCreated = true;
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
HRESULT __stdcall D3D11CreateDevice(IDXGIAdapter * pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL * pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, ID3D11Device * *ppDevice, D3D_FEATURE_LEVEL * pFeatureLevel, ID3D11DeviceContext * *ppImmediateContext)
{
	static decltype(&D3D11CreateDevice) func;
	static const char* func_name = "D3D11CreateDevice";

	logger->Log(func_name);

#ifdef IMPL_CHAINLOAD
	if (g_IsDxCreated)
	{
		logger->LogWarning(L"DirectX Create already called. Chainload bounced back. Loading System d3d11.dll");

		g_Sys11 = LoadLibraryW(g_Path_SystemDll);

		if (FindFunction(g_Sys11, &func, func_name) == false)
		{
			return 0;
		}
	}

	g_IsDxCreated = true;
#endif

	if (DxLoad() == false) { return 0; }

	if (func == 0)
	{
		if (FindFunction(g_D3D11, &func, func_name) == false)
		{
			return 0;
		}
	}

	WNDCLASSEXW wc;
	memset(&wc, 0, sizeof(wc));

	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = DefWindowProcW;
	wc.hInstance = GetModuleHandleW(0);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wc.lpszClassName = L"TempDxWndClass";
	RegisterClassExW(&wc);

	HWND wnd = CreateWindowExW(0, wc.lpszClassName, 0, WS_OVERLAPPEDWINDOW, 0, 0, 128, 128, 0, 0, wc.hInstance, 0);
	if (wnd) {
		DXGI_SWAP_CHAIN_DESC swap_desc = {};
		swap_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swap_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swap_desc.BufferCount = 1;
		swap_desc.SampleDesc.Count = 1;
		swap_desc.OutputWindow = wnd;
		swap_desc.Windowed = TRUE;

		ID3D11Device* device;
		ID3D11DeviceContext* context;
		IDXGISwapChain* swap;

		if (SUCCEEDED(D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, 0, 0, 0, D3D11_SDK_VERSION, &swap_desc, &swap, &device, 0, &context))) {
			LPVOID* vtbl;

			vtbl = *(LPVOID**)swap;
			//dxgi_release = hook_vtbl_fn(vtbl, 2, dxgi_release_hook);
			//dxgi_present = hook_vtbl_fn(vtbl, 8, dxgi_present_hook);
			//dxgi_resize_buffers = hook_vtbl_fn(vtbl, 13, dxgi_resize_buffers_hook);

			MH_CreateHook(vtbl[8], (LPVOID)&hkDXGIPresent, (LPVOID*)&dxgi_present);
			MH_CreateHook(vtbl[13], (LPVOID)&hkDXGIResizeBuffers, (LPVOID*)&dxgi_resize_buffers);
			MH_EnableHook(MH_ALL_HOOKS);

			context->Release();
			device->Release();
			swap->Release();
		}

		DestroyWindow(wnd);
	}

	UnregisterClassW(wc.lpszClassName, wc.hInstance);

	std::thread([]() { Sleep(10000); InitializeImGui(); }).detach();

	return func(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
}
HRESULT __stdcall D3D11CreateDeviceAndSwapChain(IDXGIAdapter * pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL * pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, const DXGI_SWAP_CHAIN_DESC * pSwapChainDesc, IDXGISwapChain * *ppSwapChain, ID3D11Device * *ppDevice, D3D_FEATURE_LEVEL * pFeatureLevel, ID3D11DeviceContext * *ppImmediateContext)
{
	static decltype(&D3D11CreateDeviceAndSwapChain) func;
	static const char* func_name = "D3D11CreateDeviceAndSwapChain";

	logger->Log(func_name);

#ifdef IMPL_CHAINLOAD
	if (g_IsDxCreated)
	{
		logger->LogWarning(L"DirectX Create already called. Chainload bounced back. Loading System d3d11.dll");

		g_Sys11 = LoadLibraryW(g_Path_SystemDll);

		if (FindFunction(g_Sys11, &func, func_name) == false)
		{
			return 0;
		}
	}

	g_IsDxCreated = true;
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

void InitializePaths()
{
	/* get self dll path */
	GetModuleFileNameW(g_Self, g_Path_HostDll, MAX_PATH);

	/* get current directory */
	memcpy(g_Path_HostDirectory, g_Path_HostDll, MAX_PATH);
	PathCchRemoveFileSpec(g_Path_HostDirectory, MAX_PATH);

	/* get temp dll path */
	memcpy(g_Path_TempDll, g_Path_HostDirectory, MAX_PATH);
	PathCchAppend(g_Path_TempDll, MAX_PATH, L"d3d11.tmp");

	/* get chainload dll path */
	memcpy(g_Path_ChainloadDll, g_Path_HostDirectory, MAX_PATH);
	PathCchAppend(g_Path_ChainloadDll, MAX_PATH, L"d3d11_chainload.dll");

	/* get system dll path */
	GetSystemDirectoryW(g_Path_SystemDll, MAX_PATH);
	PathCchAppend(g_Path_SystemDll, MAX_PATH, L"d3d11.dll");
}
void InitializeLogging()
{
	ConsoleLogger* cLog = new ConsoleLogger();
	cLog->SetLogLevel(LogLevel::ALL);
	logger->Register(cLog);

	FileLogger* fLog = new FileLogger("rcAddonHost.log");
	fLog->SetLogLevel(LogLevel::ALL);
	logger->Register(fLog);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_Self = hModule;
		g_GW2 = GetModuleHandle(NULL);

		InitializePaths();
		InitializeLogging();

		if (MH_Initialize() != MH_OK) {
			return FALSE;
		}

		logger->LogDebug(L"%s %s", L"ATTACH", g_Path_HostDll);
		break;
	case DLL_PROCESS_DETACH:
		logger->LogDebug(L"%s %s", L"DETACH", g_Path_HostDll);

		MH_Uninitialize();

		if (g_D3D11) { FreeLibrary(g_D3D11); }
		break;
	}
	return true;
}

LRESULT CALLBACK hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//ImGuiIO& io = ImGui::GetIO();

	//ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

	if (uMsg == WM_DESTROY)
	{
		logger->LogCritical(L"::Destroy()");
	}

	return CallWindowProc(g_GW2_WndProc, hWnd, uMsg, wParam, lParam);
}

void InitializeImGui()
{
	g_pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&g_pDevice);
	g_pDevice->GetImmediateContext(&g_pDeviceContext);

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	g_pSwapChain->GetDesc(&swapChainDesc);

	// create imgui context
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.Fonts->AddFontDefault();
	io.Fonts->Build();

	g_GW2_HWND = swapChainDesc.OutputWindow;
	g_GW2_WndProc = (WNDPROC)SetWindowLongPtr(g_GW2_HWND, GWLP_WNDPROC, (LONG_PTR)hkWndProc);

	// Init imgui
	ImGui_ImplWin32_Init(g_GW2_HWND);
	ImGui_ImplDX11_Init(g_pDevice, g_pDeviceContext);
	ImGui::GetIO().ImeWindowHandle = g_GW2_HWND;

	// create buffers
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
	g_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
	pBackBuffer->Release();

	g_IsImGuiInitialized = true;
}

HRESULT __stdcall hkDXGIPresent(IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags)
{
	if (g_pSwapChain != pChain)
	{
		g_pSwapChain = pChain;

		if (g_pDevice)
		{
			g_pDevice->Release();
			g_pDevice = 0;
		}

		g_pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&g_pDevice);
	}

	if (g_IsImGuiInitialized)
	{
		// imgui define new frame
		ImGui_ImplWin32_NewFrame();
		ImGui_ImplDX11_NewFrame();
		ImGui::NewFrame();

		// Draw ImGui here!
		//ImGui::ShowDemoWindow();

		// imgui end frame
		ImGui::EndFrame();

		// render
		ImGui::Render();

		// finish drawing
		g_pDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	return dxgi_present(pChain, SyncInterval, Flags);
}

HRESULT __stdcall hkDXGIResizeBuffers(IDXGISwapChain* pChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	// TODO:
	// the game cannot handle resizing anymore for some reason
	// probably related to imgui
	// fuck imgui

	/*if (g_ImGui_Initialized)
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }

		g_ImGui_Initialized = false;

		InitializeImGui();
	}*/

	return dxgi_resize_buffers(pChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}