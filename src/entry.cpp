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

#include "loader_hook.h"

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

/* meme */
static HRESULT(*dxgi_present)(IDXGISwapChain* This, UINT SyncInterval, UINT Flags);

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool initialized = false;
bool delay = false;
ID3D11Device* g_pDevice = nullptr;
ID3D11DeviceContext* g_pDeviceContext = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
ID3D11RenderTargetView* g_mainRenderTargetView;

WNDPROC origWndProc = nullptr;
HWND window;

HRESULT __stdcall dxgi_present_hook(IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags);
/* meme */

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

			MH_CreateHook(vtbl[8], (LPVOID)&dxgi_present_hook, (LPVOID *)&dxgi_present);
			MH_EnableHook(MH_ALL_HOOKS);

			context->Release();
			device->Release();
			swap->Release();
		}

		DestroyWindow(wnd);
	}

	UnregisterClassW(wc.lpszClassName, wc.hInstance);

	std::thread([]() { Sleep(10000); delay = true; }).detach();

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

void GetPaths()
{
	/* get self dll path */
	GetModuleFileNameW(g_Self, g_Path_HostDll, sizeof(g_Path_HostDll));

	/* get current directory */
	memcpy(g_Path_HostDirectory, g_Path_HostDll, sizeof(g_Path_HostDirectory));
	PathCchRemoveFileSpec(g_Path_HostDirectory, sizeof(g_Path_HostDirectory));

	/* get temp dll path */
	memcpy(g_Path_TempDll, g_Path_HostDirectory, sizeof(g_Path_TempDll));
	PathCchAppend(g_Path_TempDll, sizeof(g_Path_TempDll), L"d3d11.tmp");

	/* get chainload dll path */
	memcpy(g_Path_ChainloadDll, g_Path_HostDirectory, sizeof(g_Path_ChainloadDll));
	PathCchAppend(g_Path_ChainloadDll, sizeof(g_Path_ChainloadDll), L"d3d11_chainload.dll");
}

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
		
		GetPaths();

		if (MH_Initialize() != MH_OK) {
			return FALSE;
		}

		std::wcout << L"[ATTACH] " << g_Path_HostDll << std::endl;
		break;
	case DLL_PROCESS_DETACH:
		std::wcout << L"[DETACH] " << g_Path_HostDll << std::endl;

		MH_Uninitialize();

		FreeConsole();

		if (g_D3D11) { FreeLibrary(g_D3D11); }
		break;
	}
	return true;
}


LRESULT CALLBACK hWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	/*ImGuiIO& io = ImGui::GetIO();
	POINT mousePosition;
	GetCursorPos(&mousePosition);
	ScreenToClient(window, &mousePosition);
	io.MousePos.x = mousePosition.x;
	io.MousePos.y = mousePosition.y;*/

	//ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

	return CallWindowProc(origWndProc, hWnd, uMsg, wParam, lParam);
}

HRESULT __stdcall dxgi_present_hook(IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags)
{
	std::cout << "present" << std::endl;
	if (delay)
	{
		if (!initialized)
		{
			pChain->GetDevice(__uuidof(ID3D11Device), (void**)&g_pDevice);
			g_pDevice->GetImmediateContext(&g_pDeviceContext);
			g_pSwapChain = pChain;

			DXGI_SWAP_CHAIN_DESC swapChainDesc;
			pChain->GetDesc(&swapChainDesc);

			// create imgui context
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			io.Fonts->AddFontDefault();
			io.Fonts->Build();

			window = swapChainDesc.OutputWindow;

			origWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)hWndProc);

			// Init imgui
			ImGui_ImplWin32_Init(window);
			ImGui_ImplDX11_Init(g_pDevice, g_pDeviceContext);
			ImGui::GetIO().ImeWindowHandle = window;

			// create buffers
			ID3D11Texture2D* pBackBuffer;
			pChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
			g_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
			pBackBuffer->Release();

			initialized = true;
		}

		// imgui define new frame
		ImGui_ImplWin32_NewFrame();
		ImGui_ImplDX11_NewFrame();
		ImGui::NewFrame();

		// Draw ImGui here!
		ImGui::ShowDemoWindow();

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