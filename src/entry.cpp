#include <d3d11.h>
#include <cassert>
#include <thread>
#include <algorithm>
#include <shellapi.h>

#include "minhook/mh_hook.h"

#include "core.h"
#include "Paths.h"
#include "State.h"
#include "Hooks.h"
#include "Renderer.h"
#include "Shared.h"

#include "Logging/LogHandler.h"
#include "Logging/ConsoleLogger.h"
#include "Logging/FileLogger.h"

#include "Mumble/Mumble.h"
#include "Keybinds/KeybindHandler.h"
#include "Events/EventHandler.h"
#include "GUI/GUI.h"
#include "Loader/Loader.h"

#define IMPL_CHAINLOAD			/* enable chainloading */

#define UPDATE_INTERVAL	30;
unsigned Frames = 30;

/* handles */
HMODULE	hGW2		= nullptr;
HMODULE	hAddonHost	= nullptr;
HMODULE	hD3D11		= nullptr;
HMODULE	hSys11		= nullptr;

/* init/shutdown */
void InitializeState()
{
	/* arg list */
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	
	bool customMumble = false;

	/* skip first, that's the file path */
	for (int i = 1; i < argc; ++i)
	{
		std::wstring str;
		std::wstring cmp = argv[i];
		std::transform(cmp.begin(), cmp.end(), cmp.begin(), ::tolower);

		/* peek at the next argument, if it starts with - */
		if (i + 1 < argc && argv[i + 1][0] != L'-')
		{
			/* next argument belongs to this one */
			if (wcscmp(cmp.c_str(), L"-mumble") == 0)
			{
				bool customMumble = true;
				MumbleLink = Mumble::Create(argv[i + 1]);
			}

			str.append(argv[i]);
			str.append(L" ");
			str.append(argv[i + 1]);
			i++;
		}
		else
		{
			/* single argument */
			str.append(argv[i]);

#ifdef _DEBUG
			State::IsDeveloperMode = true;
#else
			State::IsDeveloperMode = wcscmp(cmp.c_str(), L"-ggdev") == 0;
			State::IsConsoleEnabled = wcscmp(cmp.c_str(), L"-ggconsole") == 0;
#endif
			State::IsVanilla = wcscmp(cmp.c_str(), L"-ggvanilla") == 0;
		}

		Parameters.push_back(str);
	}
	if (!customMumble) { MumbleLink = Mumble::Create(L"MumbleLink"); }
}
void InitializePaths()
{
	GetModuleFileNameW(hAddonHost, Path::F_HOST_DLL, MAX_PATH);											/* get self dll path */

	/* directories */
	PathGetDirectoryName(Path::F_HOST_DLL, Path::D_GW2);												/* get current directory */
	PathCopyAndAppend(Path::D_GW2, Path::D_GW2_ADDONS, L"addons");										/* get addons path */
	PathCopyAndAppend(Path::D_GW2_ADDONS, Path::D_GW2_ADDONS_RAIDCORE, L"Raidcore");					/* get addons Raidcore path */
	PathCopyAndAppend(Path::D_GW2_ADDONS_RAIDCORE, Path::D_GW2_ADDONS_RAIDCORE_FONTS, L"Fonts");		/* get addons Raidcore path */
	PathCopyAndAppend(Path::D_GW2_ADDONS_RAIDCORE, Path::D_GW2_ADDONS_RAIDCORE_LOCALES, L"Locales");	/* get addons Raidcore path */

	/* ensure folder tree*/
	CreateDirectoryW(Path::D_GW2_ADDONS_RAIDCORE, nullptr);												/* ensure Raidcore dir */
	CreateDirectoryW(Path::D_GW2_ADDONS_RAIDCORE_FONTS, nullptr);										/* ensure Raidcore/Fonts dir */
	CreateDirectoryW(Path::D_GW2_ADDONS_RAIDCORE_LOCALES, nullptr);										/* ensure Raidcore/Locales dir */

	/* ensure files */
	PathCopyAndAppend(Path::D_GW2_ADDONS_RAIDCORE, Path::F_LOG, L"AddonHost.log");						/* get log path */
	PathCopyAndAppend(Path::D_GW2_ADDONS_RAIDCORE, Path::F_KEYBINDS_JSON, L"keybinds.json");			/* get keybinds path */
	
	/* static paths */
	PathCopyAndAppend(Path::D_GW2, Path::F_TEMP_DLL, L"d3d11.tmp");										/* get temp dll path */
	PathCopyAndAppend(Path::D_GW2, Path::F_CHAINLOAD_DLL, L"d3d11_chainload.dll");						/* get chainload dll path */
	PathSystemAppend(Path::F_SYSTEM_DLL, L"d3d11.dll");													/* get system dll path */
}
void InitializeLogging()
{
	if (State::IsConsoleEnabled)
	{
		ConsoleLogger* cLog = new ConsoleLogger();
		cLog->SetLogLevel(ELogLevel::ALL);
		Logger->Register(cLog);
	}

	FileLogger* fLog = new FileLogger(Path::F_LOG);
	fLog->SetLogLevel(State::IsDeveloperMode ? ELogLevel::ALL : ELogLevel::INFO);
	Logger->Register(fLog);
}
void Initialize()
{
	State::AddonHost = ggState::LOAD;

	InitializeState();
	InitializePaths();
	InitializeLogging();

	MH_Initialize();

	MinhookTable.CreateHook		= MH_CreateHook;
	MinhookTable.RemoveHook		= MH_RemoveHook;
	MinhookTable.EnableHook		= MH_EnableHook;
	MinhookTable.DisableHook	= MH_DisableHook;

	Logger->LogInfo(GetCommandLineW());

	//std::thread([]() { KeybindHandler::LoadKeybinds(); }).detach();
}

void Shutdown()
{
	if (State::AddonHost < ggState::SHUTDOWN)
	{
		State::AddonHost = ggState::SHUTDOWN;

		GUI::Shutdown();

		Mumble::Destroy();

		MH_Uninitialize();

		if (hD3D11) { FreeLibrary(hD3D11); }
	}
}

/* hk */
LRESULT __fastcall hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//static const char* func_name = "hkWndProc";
	//Logger->Log(func_name);

	//if (uMsg == WM_PAINT && State::AddonHost <= ggState::ADDONS_LOAD) { Loader::Initialize(); }

	// don't pass to game if keybind
	if (KeybindHandler::WndProc(hWnd, uMsg, wParam, lParam)) { return 0; }

	// don't pass to game if gui
	if (GUI::WndProc(hWnd, uMsg, wParam, lParam)) { return 0; }

	if (uMsg == WM_DESTROY)
	{
		Logger->LogCritical(L"::Destroy()");
		Shutdown();
	}

	// TODO: Consider subclassing instead of CallWindowProcA
	// subclassing thingy: https://devblogs.microsoft.com/oldnewthing/?p=41883
	// CallWindowProcA is necessary btw, else a memory access violation will occur if directly called
	return CallWindowProcA(Hooks::GW2_WndProc, hWnd, uMsg, wParam, lParam);
}
HRESULT __stdcall hkDXGIPresent(IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags)
{
	//static const char* func_name = "hkDXGIPresent";
	//Logger->Log(func_name);

	/* every 30 frames, update identity*/
	if (Frames == 30) { Mumble::UpdateIdentity(); Loader::Update(); }

	if (Renderer::SwapChain != pChain)
	{
		Renderer::SwapChain = pChain;

		if (Renderer::Device)
		{
			Renderer::DeviceContext->Release();
			Renderer::DeviceContext = 0;
			Renderer::Device->Release();
			Renderer::Device = 0;
		}

		Renderer::SwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&Renderer::Device);
		Renderer::Device->GetImmediateContext(&Renderer::DeviceContext);

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		Renderer::SwapChain->GetDesc(&swapChainDesc);

		Renderer::WindowHandle = swapChainDesc.OutputWindow;
		Hooks::GW2_WndProc = (WNDPROC)SetWindowLongPtr(Renderer::WindowHandle, GWLP_WNDPROC, (LONG_PTR)hkWndProc);
	}

	GUI::Render();

	Frames++;

	/* always reset frames back to 0 above 30*/
	if (Frames > 30) { Frames = 0; }

	return Hooks::DXGI_Present(pChain, SyncInterval, Flags);
}
HRESULT __stdcall hkDXGIResizeBuffers(IDXGISwapChain* pChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	static const char* func_name = "hkDXGIResizeBuffers";
	Logger->Log(func_name);

	GUI::Shutdown();

	/* Cache window dimensions */
	Renderer::Width = Width;
	Renderer::Height = Height;
	
	return Hooks::DXGI_ResizeBuffers(pChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}

/* dx */
bool DxLoad()
{
	if (State::Directx < DxState::DIRECTX_READY)
	{
		State::Directx = DxState::DIRECTX_LOAD;

#ifdef IMPL_CHAINLOAD
		/* attempt to chainload */
		/* sanity check that the current dll isn't the chainload */
		if (Path::F_HOST_DLL != Path::F_CHAINLOAD_DLL)
		{
			Logger->LogInfo(L"Attempting to chainload.");

			State::IsChainloading = true;

			hD3D11 = LoadLibraryW(Path::F_CHAINLOAD_DLL);
		}
#endif

		/* ifn chainload, load system dll */
		if (!hD3D11)
		{
#ifdef IMPL_CHAINLOAD
			if (State::IsChainloading)
			{
				Logger->LogWarning(L"No chainload found or failed to load.");
			}
#endif
			State::IsChainloading = false;

			hD3D11 = LoadLibraryW(Path::F_SYSTEM_DLL);

			assert(hD3D11 && "Could not load system d3d11.dll");

			Logger->LogInfo(L"Loaded System DLL: %s", Path::F_SYSTEM_DLL);
		}

		State::Directx = DxState::DIRECTX_READY;
	}

	return (hD3D11 != NULL);
}

HRESULT __stdcall D3D11CoreCreateDevice(IDXGIFactory* pFactory, IDXGIAdapter* pAdapter, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, ID3D11Device& ppDevice)
{
	static decltype(&D3D11CoreCreateDevice) func;
	static const char* func_name = "D3D11CoreCreateDevice";
	Logger->Log(func_name);

#ifdef IMPL_CHAINLOAD
	if (State::Directx >= DxState::DIRECTX_READY)
	{
		Logger->LogWarning(L"DirectX Create already called. Chainload bounced back. Loading System d3d11.dll");

		hSys11 = LoadLibraryW(Path::F_SYSTEM_DLL);

		if (FindFunction(hSys11, &func, func_name) == false)
		{
			return 0;
		}
	}
#endif

	if (DxLoad() == false) { return 0; }

	if (func == 0)
	{
		if (FindFunction(hD3D11, &func, func_name) == false)
		{
			return 0;
		}
	}

	return func(pFactory, pAdapter, Flags, pFeatureLevels, FeatureLevels, ppDevice);
}
HRESULT __stdcall D3D11CreateDevice(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext)
{
	static decltype(&D3D11CreateDevice) func;
	static const char* func_name = "D3D11CreateDevice";
	Logger->Log(func_name);

#ifdef IMPL_CHAINLOAD
	if (State::Directx >= DxState::DIRECTX_READY)
	{
		Logger->LogWarning(L"DirectX Create already called. Chainload bounced back. Loading System d3d11.dll");

		hSys11 = LoadLibraryW(Path::F_SYSTEM_DLL);

		if (FindFunction(hSys11, &func, func_name) == false)
		{
			return 0;
		}
	}
#endif

	if (DxLoad() == false) { return 0; }

	if (func == 0)
	{
		if (FindFunction(hD3D11, &func, func_name) == false)
		{
			return 0;
		}
	}

	if (State::Directx < DxState::DIRECTX_HOOKED)
	{
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

				MH_CreateHook(vtbl[8], (LPVOID)&hkDXGIPresent, (LPVOID*)&Hooks::DXGI_Present);
				MH_CreateHook(vtbl[13], (LPVOID)&hkDXGIResizeBuffers, (LPVOID*)&Hooks::DXGI_ResizeBuffers);
				MH_EnableHook(MH_ALL_HOOKS);

				context->Release();
				device->Release();
				swap->Release();
			}

			DestroyWindow(wnd);
		}

		UnregisterClassW(wc.lpszClassName, wc.hInstance);

		State::AddonHost	= ggState::READY;
		State::Directx		= DxState::DIRECTX_HOOKED;
	}

	return func(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
}
HRESULT __stdcall D3D11CreateDeviceAndSwapChain(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, IDXGISwapChain** ppSwapChain, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext)
{
	static decltype(&D3D11CreateDeviceAndSwapChain) func;
	static const char* func_name = "D3D11CreateDeviceAndSwapChain";
	Logger->Log(func_name);

#ifdef IMPL_CHAINLOAD
	if (State::Directx >= DxState::DIRECTX_READY)
	{
		Logger->LogWarning(L"DirectX Create already called. Chainload bounced back. Loading System d3d11.dll");

		hSys11 = LoadLibraryW(Path::F_SYSTEM_DLL);

		if (FindFunction(hSys11, &func, func_name) == false)
		{
			return 0;
		}
	}
#endif

	if (DxLoad() == false) { return 0; }

	if (func == 0)
	{
		if (FindFunction(hD3D11, &func, func_name) == false)
		{
			return 0;
		}
	}

	return func(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);
}

/* entry */
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hAddonHost = hModule;
		hGW2 = GetModuleHandle(NULL);

		Initialize();

		Logger->LogInfo(L"Version: %s", Version);
		break;
	case DLL_PROCESS_DETACH:

		break;
	}
	return true;
}