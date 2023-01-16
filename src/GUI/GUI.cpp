#include "GUI.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui_impl_dx11.h"

#include "../State.h"
#include "../Renderer.h"
#include "../Paths.h"
#include "../Shared.h"

#include "../Keybinds/KeybindHandler.h"

namespace GUI
{
	void SetupWindowsAndKeybinds(); // forward declare

	bool		IsMenuVisible = true;
	bool		IsSetup = false;

	std::mutex WindowsMutex;
	std::vector<IWindow*> Windows;

	void Initialize()
	{
		// create imgui context
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();

		// Load Fonts
		// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
		// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
		// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
		// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
		// - Read 'docs/FONTS.md' for more instructions and details.
		// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
		//io.Fonts->AddFontDefault();
		//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
		//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
		//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
		//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
		//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
		//IM_ASSERT(font != NULL);

		//io.Fonts->AddFontDefault();
		wchar_t font[MAX_PATH];
		PathCopyAndAppend(Path::D_GW2_ADDONS_RAIDCORE, font, L"raidcore_font.ttf");

		std::wstring wstr = font;
		std::string str = WStrToStr(wstr);
		const char* cp = str.c_str();

		io.Fonts->AddFontFromFileTTF(cp, 16.0f);
		io.Fonts->Build();

		// Init imgui
		ImGui_ImplWin32_Init(Renderer::WindowHandle);
		ImGui_ImplDX11_Init(Renderer::Device, Renderer::DeviceContext);
		ImGui::GetIO().ImeWindowHandle = Renderer::WindowHandle;

		// create buffers
		ID3D11Texture2D* pBackBuffer;
		Renderer::SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
		Renderer::Device->CreateRenderTargetView(pBackBuffer, NULL, &Renderer::RenderTargetView);
		pBackBuffer->Release();

		if (!IsSetup) { SetupWindowsAndKeybinds(); }

		State::IsImGuiInitialized = true;
	}

	void Shutdown()
	{
		if (State::IsImGuiInitialized)
		{
			State::IsImGuiInitialized = false;

			ImGui_ImplDX11_Shutdown();
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
			if (Renderer::RenderTargetView) { Renderer::RenderTargetView->Release(); Renderer::RenderTargetView = NULL; }
		}
	}

	bool WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (State::IsImGuiInitialized)
		{
			ImGuiIO& io = ImGui::GetIO();
			if (io.WantCaptureMouse)
			{
				switch (uMsg)
				{
				case WM_LBUTTONDBLCLK:
				case WM_LBUTTONDOWN:	if (!GetAsyncKeyState(VK_RBUTTON)) { io.MouseDown[0] = true; }					return true;
				case WM_RBUTTONDBLCLK:
				case WM_RBUTTONDOWN:	io.MouseDown[1] = true;															return true;

				case WM_LBUTTONUP:		io.MouseDown[0] = false;														break;
				case WM_RBUTTONUP:		io.MouseDown[1] = false;														break;

				case WM_MOUSEWHEEL:		io.MouseWheel += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;	return true;
				case WM_MOUSEHWHEEL:	io.MouseWheelH += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;	return true;
				}
			}

			if (io.WantTextInput)
			{
				switch (uMsg)
				{
				case WM_KEYDOWN:
				case WM_SYSKEYDOWN:
					if (wParam < 256)
						io.KeysDown[wParam] = 1;
					return true;
				case WM_KEYUP:
				case WM_SYSKEYUP:
					if (wParam < 256)
						io.KeysDown[wParam] = 0;
					return true;
				case WM_CHAR:
					// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
					if (wParam > 0 && wParam < 0x10000)
						io.AddInputCharacterUTF16((unsigned short)wParam);
					return true;
				}
			}
		}

		return false;
	}

	void SetupWindowsAndKeybinds()
	{
		LogWindow* logWnd = new LogWindow();

		Logger->Register(logWnd);
		logWnd->SetLogLevel(ELogLevel::ALL);

		AddWindow(logWnd);
		AddWindow(new AddonsWindow());
		//AddWindow(new Keybinds());
		AddWindow(new MumbleOverlay());
		AddWindow(new AboutBox());

		Keybind options{};
		options.Ctrl = true;
		options.Key = 79;
		KeybindHandler::RegisterKeybind(L"RAIDCORE_OPTIONS", ProcessKeybind, options);

		IsSetup = true;
	}

	void ProcessKeybind(const wchar_t* aIdentifier)
	{
		Logger->LogDebug(aIdentifier);
		if (wcscmp(aIdentifier, L"RAIDCORE_OPTIONS") == 0)
		{
			IsMenuVisible = !IsMenuVisible;
			return;
		}
	}

	void Render()
	{
		if (State::AddonHost >= ggState::READY && !State::IsImGuiInitialized)
		{
			GUI::Initialize();
		}

		if (State::IsImGuiInitialized)
		{
			/* new frame */
			ImGui_ImplWin32_NewFrame();
			ImGui_ImplDX11_NewFrame();
			ImGui::NewFrame();

			/* draw overlay */
			RenderMenu();
			WindowsMutex.lock();
			for (IWindow* wnd : Windows)
			{
				wnd->Render();
			}
			WindowsMutex.unlock();

			/* TODO: ADDONS->RENDER() */
			/* TODO: RENDER UNDER UI */
			/* TODO: RENDER OVER UI */

			/* end frame */
			ImGui::EndFrame();

			/* render frame */
			ImGui::Render();
			Renderer::DeviceContext->OMSetRenderTargets(1, &Renderer::RenderTargetView, NULL);
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}
	}

	void RenderMenu()
	{
		if (!IsMenuVisible) { return; }

		if (ImGui::Begin("Menu", &IsMenuVisible, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
		{
			WindowsMutex.lock();

			for (IWindow* wnd : Windows) { wnd->MenuOption(0); }
			ImGui::Button("Keybinds", ImVec2(ImGui::GetFontSize() * 13.75f, 0.0f));
			ImGui::Button("Layout", ImVec2(ImGui::GetFontSize() * 13.75f, 0.0f));
			ImGui::Button("Options", ImVec2(ImGui::GetFontSize() * 13.75f, 0.0f));

			ImGui::Separator();

			for (IWindow* wnd : Windows) { wnd->MenuOption(1); }

			ImGui::Separator();

			for (IWindow* wnd : Windows) { wnd->MenuOption(2); }

			WindowsMutex.unlock();
		}
		ImGui::End();
	}

	void AddWindow(IWindow* aWindowPtr)
	{
		WindowsMutex.lock();

		Windows.push_back(aWindowPtr);

		WindowsMutex.unlock();
	}
}