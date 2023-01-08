#include "GUI.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui_impl_dx11.h"

#include "../State.h"
#include "../Renderer.h"
#include "../Paths.h"
#include "../Shared.h"

namespace GUI
{
	bool		MenuVisible = true;

	//Addons	AddonsWindow;
	//Keybinds	KeybindsWindow;
	//Log		LogWindow;
	About		AboutWindow;

	void Initialize()
	{
		// create imgui context
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

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

			/* draw */
			/* TODO: CLEAN THIS UP SO IT'S NOT HARDCODED? */
			ShowMenu();
			//Addons.Show();
			//Keybinds.Show();
			//Log.Show();
			AboutWindow.Show();
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

	void ShowMenu()
	{
		if (ImGui::Begin("Raidcore", &MenuVisible, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
		{

			ImGui::Button("Addons", ImVec2(ImGui::GetFontSize() * 13.75f, ImGui::GetFontSize() * 1.25f));
			ImGui::Button("Keybinds", ImVec2(ImGui::GetFontSize() * 13.75f, ImGui::GetFontSize() * 1.25f));
			ImGui::Button("Layout", ImVec2(ImGui::GetFontSize() * 13.75f, ImGui::GetFontSize() * 1.25f));

			ImGui::Separator();

			ImGui::Button("Log", ImVec2(ImGui::GetFontSize() * 13.75f, ImGui::GetFontSize() * 1.25f));
			ImGui::Button("Memory Editor", ImVec2(ImGui::GetFontSize() * 13.75f, ImGui::GetFontSize() * 1.25f));

			ImGui::Separator();

			if (ImGui::Button("About", ImVec2(ImGui::GetFontSize() * 13.75f, ImGui::GetFontSize() * 1.25f))) { AboutWindow.Visible = !AboutWindow.Visible; }
		}
		ImGui::End();
	}
}