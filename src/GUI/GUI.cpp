#include "GUI.h"

namespace GUI
{
	/* internal forward declarations */
	void OnMumbleIdentityChanged(void* aEventArgs);
	void ConfigureFonts();
	void Setup();

	std::mutex					Mutex;
	std::vector<GUI_RENDER>	Registry;
	std::vector<IWindow*>		Windows;
	std::map<EFont, ImFont*>	FontIndex;
	float						FontSize;
	bool						CloseMenuAfterSelecting;

	bool						IsUIVisible			= true;

	bool						IsRightClickHeld	= false;
	bool						IsLeftClickHeld		= false;
	bool						IsSetup				= false;
	float						LastScaling;

	void Initialize()
	{
		// Init imgui
		ImGui_ImplWin32_Init(Renderer::WindowHandle);
		ImGui_ImplDX11_Init(Renderer::Device, Renderer::DeviceContext);
		//ImGui::GetIO().ImeWindowHandle = Renderer::WindowHandle;

		// create buffers
		ID3D11Texture2D* pBackBuffer;
		Renderer::SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
		Renderer::Device->CreateRenderTargetView(pBackBuffer, NULL, &Renderer::RenderTargetView);
		pBackBuffer->Release();

		if (!IsSetup) { Setup(); }

		State::IsImGuiInitialized = true;
	}
	void Shutdown()
	{
		if (State::IsImGuiInitialized)
		{
			State::IsImGuiInitialized = false;

			ImGui_ImplDX11_Shutdown();
			ImGui_ImplWin32_Shutdown();
			if (Renderer::RenderTargetView) { Renderer::RenderTargetView->Release(); Renderer::RenderTargetView = NULL; }
		}
	}

	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (State::IsImGuiInitialized)
		{
			ImGuiIO& io = ImGui::GetIO();

			switch (uMsg)
			{
				// mouse input
				case WM_MOUSEMOVE:
					io.MousePos = ImVec2((float)(LOWORD(lParam)), (float)(HIWORD(lParam)));

					if (IsLeftClickHeld || IsRightClickHeld)
					{
						io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
					}
					else if (io.WantCaptureMouse)
					{
						return 0;
					}
					break;

				case WM_LBUTTONDBLCLK:
				case WM_LBUTTONDOWN:
					if (io.WantCaptureMouse && !IsLeftClickHeld && !IsRightClickHeld)
					{
						io.MouseDown[0] = true;
						return 0;
					}
					else //if (!io.WantCaptureMouse)
					{
						IsLeftClickHeld = true;
						ImGui::ClearActiveID();
					}
					break;
				case WM_RBUTTONDBLCLK:
				case WM_RBUTTONDOWN:
					if (io.WantCaptureMouse && !IsLeftClickHeld && !IsRightClickHeld)
					{
						io.MouseDown[1] = true;
						return 0;
					}
					else //if (!io.WantCaptureMouse)
					{
						IsRightClickHeld = true;
					}
					break;

				// doesn't hurt passing these through to the game
				case WM_LBUTTONUP:
					IsLeftClickHeld = false; io.MouseDown[0] = false;
					break;
				case WM_RBUTTONUP:
					IsRightClickHeld = false; io.MouseDown[1] = false;
					break;

				// scroll
				case WM_MOUSEWHEEL:
					if (io.WantCaptureMouse && !IsLeftClickHeld && !IsRightClickHeld)
					{
						io.MouseWheel += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
						return 0;
					}
					break;
				case WM_MOUSEHWHEEL:
					if (io.WantCaptureMouse && !IsLeftClickHeld && !IsRightClickHeld)
					{
						io.MouseWheelH += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
						return 0;
					}
					break;

				// key input
				case WM_KEYDOWN:
				case WM_SYSKEYDOWN:
					if (io.WantTextInput)
					{
						if (wParam < 256)
							io.KeysDown[wParam] = true;
						return 0;
					}
					break;
				case WM_KEYUP:
				case WM_SYSKEYUP:
					if (wParam < 256)
						io.KeysDown[wParam] = false;
					break;
				case WM_CHAR:
					if (io.WantTextInput)
					{
						// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
						if (wParam > 0 && wParam < 0x10000)
							io.AddInputCharacterUTF16((unsigned short)wParam);
						return 0;
					}
					break;

				// other
				case WM_ACTIVATEAPP:
					// alt tab should reset clickHeld state
					if (!wParam)
					{
						IsLeftClickHeld = false;
						IsRightClickHeld = false;
					}
					break;
			}
		}

		return 1;
	}

	void Render()
	{
		if (State::AddonHost >= ENexusState::UI_READY && !State::IsImGuiInitialized)
		{
			GUI::Initialize();
		}

		if (State::IsImGuiInitialized)
		{
			/* new frame */
			ImGui_ImplWin32_NewFrame();
			ImGui_ImplDX11_NewFrame();
			ImGui::NewFrame();
			/* new frame end */

			/* draw overlay */
			if (IsUIVisible)
			{
				/* draw menu & qa */
				Menu::Render();
				QuickAccess::Render();

				/* draw windows */
				Mutex.lock();
				{
					for (IWindow* wnd : Windows)
					{
						wnd->Render();
					}
				}
				Mutex.unlock();

				/* TODO: RENDER UNDER UI */

#define WATERMARK
#ifdef WATERMARK
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
				ImGui::SetNextWindowPos(ImVec2(16.0f, Renderer::Height != 0 ? Renderer::Height - ImGui::GetTextLineHeight() - 16.0f : 0));
				if (ImGui::Begin("NEXUS_BUILDINFO", (bool*)0, WindowFlags_Watermark))
				{
					ImGui::SetCursorPos(ImVec2(0, 0));
					ImGui::TextOutlined("Limited Test Build");
					ImGui::SameLine();
					ImGui::TextOutlined(__DATE__ " " __TIME__);
					ImGui::SameLine();
					ImGui::TextOutlined(("(v" + Version->ToString() + ")").c_str());
					ImGui::SameLine();
#ifdef _DEBUG
					ImGui::TextOutlined("debug/" BRANCH_NAME);
#else
					ImGui::TextOutlined("release/" BRANCH_NAME);
#endif
				};
				ImGui::End();
				ImGui::PopStyleVar();
#endif
			}

			/* draw addons*/
			Mutex.lock();
			{
				for (GUI_RENDER callback : Registry)
				{
					if (callback) { callback(IsUIVisible); }
				}
			}
			Mutex.unlock();

			/* draw overlay end */

			/* end frame */
			ImGui::EndFrame();
			ImGui::Render();
			Renderer::DeviceContext->OMSetRenderTargets(1, &Renderer::RenderTargetView, NULL);
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			/* end frame end*/
		}
	}

	void AddWindow(IWindow* aWindowPtr)
	{
		Mutex.lock();
		{
			Windows.push_back(aWindowPtr);
		}
		Mutex.unlock();
	}

	void ResizeFonts()
	{
		ImGuiIO& io = ImGui::GetIO();

		switch (MumbleIdentity->UISize)
		{
			case 0:
				Font	= FontIndex[EFont::Menomonia_Small];
				FontBig	= FontIndex[EFont::MenomoniaBig_Small];
				FontUI	= FontIndex[EFont::Trebuchet_Small];
				break;
			default:
			case 1:
				Font	= FontIndex[EFont::Menomonia_Normal];
				FontBig	= FontIndex[EFont::MenomoniaBig_Normal];
				FontUI	= FontIndex[EFont::Trebuchet_Normal];
				break;
			case 2:
				Font	= FontIndex[EFont::Menomonia_Large];
				FontBig	= FontIndex[EFont::MenomoniaBig_Large];
				FontUI	= FontIndex[EFont::Trebuchet_Large];
				break;
			case 3:
				Font	= FontIndex[EFont::Menomonia_Larger];
				FontBig	= FontIndex[EFont::MenomoniaBig_Larger];
				FontUI	= FontIndex[EFont::Trebuchet_Larger];
				break;
		}
	}

	void ProcessKeybind(const char* aIdentifier)
	{
		std::string str = aIdentifier;

		if (str == KB_MENU)
		{
			Menu::Visible = !Menu::Visible;
			return;
		}
		else if (str == KB_TOGGLEHIDEUI)
		{
			IsUIVisible = !IsUIVisible;
		}
	}
	
	void OnMumbleIdentityChanged(void* aEventArgs)
	{
		if (Renderer::Scaling != LastScaling && IsGameplay)
		{
			ResizeFonts();

			LastScaling = Renderer::Scaling;
			Settings::Settings[OPT_LASTUISCALE] = Renderer::Scaling;
			Settings::Save();
		}
	}

	void ConfigureFonts()
	{
		ImGuiIO& io = ImGui::GetIO();

		/* add user font, or fallback to default */
		if (std::filesystem::exists(Path::F_FONT))
		{
			FontSize = 16.0f;
			if (!Settings::Settings.is_null())
			{
				if (!Settings::Settings[OPT_FONTSIZE].is_null())
				{
					FontSize = Settings::Settings[OPT_FONTSIZE].get<float>();
				}
				else
				{
					Settings::Settings[OPT_FONTSIZE] = FontSize;
					Settings::Save();
				}
			}
			io.Fonts->AddFontFromFileTTF(Path::F_FONT, FontSize);
		}
		else
		{
			io.Fonts->AddFontDefault();
		}

		/* load gw2 fonts */

		LPVOID resM{}; DWORD szM{};
		HRSRC hResM = FindResourceA(NexusHandle, MAKEINTRESOURCE(RES_FONT_MENOMONIA), RT_FONT);
		if (hResM)
		{
			HGLOBAL hLResM = LoadResource(NexusHandle, hResM);

			if (hLResM)
			{
				LPVOID pLResM = LockResource(hLResM);

				if (pLResM)
				{
					DWORD dwResSzM = SizeofResource(NexusHandle, hResM);

					if (0 != dwResSzM)
					{
						resM = pLResM;
						szM = dwResSzM;
					}
				}
			}
		}

		LPVOID resT{}; DWORD szT{};
		HRSRC hResT = FindResourceA(NexusHandle, MAKEINTRESOURCE(RES_FONT_TREBUCHET), RT_FONT);
		if (hResT)
		{
			HGLOBAL hLResT = LoadResource(NexusHandle, hResT);

			if (hLResT)
			{
				LPVOID pLResT = LockResource(hLResT);

				if (pLResT)
				{
					DWORD dwResSzT = SizeofResource(NexusHandle, hResT);

					if (0 != dwResSzT)
					{
						resT = pLResT;
						szT = dwResSzT;
					}
				}
			}
		}

		Mutex.lock();
		{
			/* small UI*/
			FontIndex.emplace(EFont::Menomonia_Small, io.Fonts->AddFontFromMemoryTTF(resM, szM, 16.0f));
			FontIndex.emplace(EFont::MenomoniaBig_Small, io.Fonts->AddFontFromMemoryTTF(resM, szM, 22.0f));
			FontIndex.emplace(EFont::Trebuchet_Small, io.Fonts->AddFontFromMemoryTTF(resT, szT, 15.5f));

			/* normal UI*/
			FontIndex.emplace(EFont::Menomonia_Normal, io.Fonts->AddFontFromMemoryTTF(resM, szM, 18.0f));
			FontIndex.emplace(EFont::MenomoniaBig_Normal, io.Fonts->AddFontFromMemoryTTF(resM, szM, 24.0f));
			FontIndex.emplace(EFont::Trebuchet_Normal, io.Fonts->AddFontFromMemoryTTF(resT, szT, 16.0f));

			/* large UI*/
			FontIndex.emplace(EFont::Menomonia_Large, io.Fonts->AddFontFromMemoryTTF(resM, szM, 20.0f));
			FontIndex.emplace(EFont::MenomoniaBig_Large, io.Fonts->AddFontFromMemoryTTF(resM, szM, 26.0f));
			FontIndex.emplace(EFont::Trebuchet_Large, io.Fonts->AddFontFromMemoryTTF(resT, szT, 17.5f));

			/* larger UI*/
			FontIndex.emplace(EFont::Menomonia_Larger, io.Fonts->AddFontFromMemoryTTF(resM, szM, 22.0f));
			FontIndex.emplace(EFont::MenomoniaBig_Larger, io.Fonts->AddFontFromMemoryTTF(resM, szM, 28.0f));
			FontIndex.emplace(EFont::Trebuchet_Larger, io.Fonts->AddFontFromMemoryTTF(resT, szT, 19.5f));
		}
		Mutex.unlock();

		ResizeFonts();

		io.Fonts->Build();
	}
	void Setup()
	{
		ConfigureFonts();

		Events::Subscribe(EV_MUMBLE_IDENTITY_UPDATED, OnMumbleIdentityChanged);

		/* set up and add windows */
		AddonsWindow* addonsWnd = new AddonsWindow();
		LogWindow* logWnd = new LogWindow(ELogLevel::ALL);
		RegisterLogger(logWnd);
		OptionsWindow* opsWnd = new OptionsWindow();
		DebugWindow* dbgWnd = new DebugWindow();
		AboutBox* aboutWnd = new AboutBox();

		AddWindow(addonsWnd);
		AddWindow(opsWnd);
		AddWindow(logWnd);
		AddWindow(dbgWnd);
		AddWindow(aboutWnd);

		Menu::AddMenuItem("Addons",		&addonsWnd->Visible);
		Menu::AddMenuItem("Options",	&opsWnd->Visible);
		Menu::AddMenuItem("Log",		&logWnd->Visible);
		Menu::AddMenuItem("Debug",		&dbgWnd->Visible);
		Menu::AddMenuItem("About",		&aboutWnd->Visible);

		/* register keybinds */
		Keybinds::Register(KB_MENU, ProcessKeybind, "CTRL+O");
		Keybinds::Register(KB_TOGGLEHIDEUI, ProcessKeybind, "CTRL+H");

		/* load icons */
		TextureLoader::LoadFromResource(ICON_NEXUS, RES_ICON_NEXUS, NexusHandle, nullptr);
		TextureLoader::LoadFromResource(ICON_NEXUS_HOVER, RES_ICON_NEXUS_HOVER, NexusHandle, nullptr);

		TextureLoader::LoadFromResource(ICON_GENERIC, RES_ICON_GENERIC, NexusHandle, nullptr);
		TextureLoader::LoadFromResource(ICON_GENERIC_HOVER, RES_ICON_GENERIC_HOVER, NexusHandle, nullptr);

		TextureLoader::LoadFromResource(TEX_MENU_BACKGROUND, RES_TEX_MENU_BACKGROUND, NexusHandle, Menu::ReceiveTextures);
		TextureLoader::LoadFromResource(TEX_MENU_BUTTON, RES_TEX_MENU_BUTTON, NexusHandle, Menu::ReceiveTextures);
		TextureLoader::LoadFromResource(TEX_MENU_BUTTON_HOVER, RES_TEX_MENU_BUTTON_HOVER, NexusHandle, Menu::ReceiveTextures);

		/* add shortcut */
		QuickAccess::AddShortcut(QA_MENU, ICON_NEXUS, ICON_NEXUS_HOVER, KB_MENU, "Nexus Menu");

		if (!Settings::Settings.is_null())
		{
			if (!Settings::Settings[OPT_LASTUISCALE].is_null() && Renderer::Scaling == 0)
			{
				LastScaling = Settings::Settings[OPT_LASTUISCALE].get<float>();
				Renderer::Scaling = Settings::Settings[OPT_LASTUISCALE].get<float>();
			}

			if (!Settings::Settings[OPT_QAVERTICAL].is_null()) { QuickAccess::VerticalLayout = Settings::Settings[OPT_QAVERTICAL].get<bool>(); }
			if (!Settings::Settings[OPT_QALOCATION].is_null()) { QuickAccess::Location = (EQAPosition)Settings::Settings[OPT_QALOCATION].get<int>(); }
			if (!Settings::Settings[OPT_QAOFFSETX].is_null() && !Settings::Settings[OPT_QAOFFSETY].is_null())
			{
				QuickAccess::Offset = ImVec2(Settings::Settings[OPT_QAOFFSETX].get<float>(), Settings::Settings[OPT_QAOFFSETY].get<float>());
			}

			if (!Settings::Settings[OPT_CLOSEMENU].is_null()) { CloseMenuAfterSelecting = Settings::Settings[OPT_CLOSEMENU].get<bool>(); }
		}

		IsSetup = true;
	}

	void Register(ERenderType aRenderType, GUI_RENDER aRenderCallback)
	{
		Mutex.lock();
		{
			Registry.push_back(aRenderCallback);
		}
		Mutex.unlock();
	}

	void Unregister(GUI_RENDER aRenderCallback)
	{
		Mutex.lock();
		{
			Registry.erase(std::remove(Registry.begin(), Registry.end(), aRenderCallback), Registry.end());
		}
		Mutex.unlock();
	}

	int Verify(void* aStartAddress, void* aEndAddress)
	{
		int refCounter = 0;

		Mutex.lock();
		{
			for (GUI_RENDER renderCb : Registry)
			{
				if (renderCb >= aStartAddress && renderCb <= aEndAddress)
				{
					Registry.erase(std::remove(Registry.begin(), Registry.end(), renderCb), Registry.end());
					refCounter++;
				}
			}
		}
		Mutex.unlock();

		return refCounter;
	}
}