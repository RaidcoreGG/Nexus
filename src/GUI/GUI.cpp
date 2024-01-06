#include "GUI.h"

#include <filesystem>
#include <algorithm>
#include <chrono>
#include <fstream>

#include "State.h"
#include "Renderer.h"
#include "Paths.h"
#include "Shared.h"
#include "Consts.h"
#include "Branch.h"

#include "Events/EventHandler.h"
#include "Keybinds/KeybindHandler.h"
#include "Loader/Loader.h"
#include "Settings/Settings.h"
#include "Textures/TextureLoader.h"

#include "imgui.h"
#include "imgui_extensions.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "Widgets/Menu/Menu.h"
#include "Widgets/Menu/MenuItem.h"
#include "Widgets/Addons/AddonsWindow.h"
#include "Widgets/Options/OptionsWindow.h"
#include "Widgets/Changelog/ChangelogWindow.h"
#include "Widgets/Log/LogWindow.h"
#include "Widgets/Debug/DebugWindow.h"
#include "Widgets/About/AboutBox.h"
#include "Widgets/QuickAccess/QuickAccess.h"
#include "Widgets/EULA/EULAModal.h"

#include "resource.h"
#include "Textures/Texture.h"

namespace GUI
{
	/* internal forward declarations */
	void OnMumbleIdentityChanged(void* aEventArgs);
	void Setup();

	std::mutex					Mutex;
	std::vector<GUI_RENDER>		RegistryPreRender;
	std::vector<GUI_RENDER>		RegistryRender;
	std::vector<GUI_RENDER>		RegistryPostRender;
	std::vector<GUI_RENDER>		RegistryOptionsRender;
	std::vector<IWindow*>		Windows;
	std::map<EFont, ImFont*>	FontIndex;
	float						FontSize					= 16.0f;
	bool						CloseMenuAfterSelecting		= true;
	bool						CloseOnEscape				= true;
	bool						LinkArcDPSStyle				= true;

	bool						IsUIVisible					= true;

	bool						IsRightClickHeld			= false;
	bool						IsLeftClickHeld				= false;
	bool						IsSetup						= false;
	float						LastScaling;

	bool						HasAcceptedEULA				= false;
	bool						NotifyChangelog				= false;

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
			if (Renderer::RenderTargetView)
			{
				Renderer::RenderTargetView->Release();
				Renderer::RenderTargetView = 0;
			}
		}
	}

	UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (State::IsImGuiInitialized)
		{
			ImGuiIO& io = ImGui::GetIO();

			if (CloseOnEscape)
			{
				if (uMsg == WM_KEYDOWN && wParam == VK_ESCAPE)
				{
					ImVector<ImGuiWindow*> windows = Renderer::GuiContext->Windows;

					for (int i = windows.Size - 1; i > 0; i--)
					{
						if (strcmp(windows[i]->Name, "Menu") == 0 && Menu::Visible)
						{
							Menu::Visible = false;
							return 0;
						}

						for (IWindow* wnd : Windows)
						{
							if (wnd->Name == windows[i]->Name && wnd->Visible)
							{
								wnd->Visible = false;
								return 0;
							}
						}
					}
				}
			}

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
		/* pre-render callbacks */
		GUI::Mutex.lock();
		{
			for (GUI_RENDER callback : RegistryPreRender)
			{
				if (callback) { callback(); }
			}
		}
		GUI::Mutex.unlock();
		/* pre-render callbacks end*/

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
				/* draw menu & qa end*/

				/* draw nexus windows */
				GUI::Mutex.lock();
				{
					for (IWindow* wnd : Windows)
					{
						wnd->Render();
					}
				}
				GUI::Mutex.unlock();
				/* draw nexus windows end */

				/* draw addons*/
				GUI::Mutex.lock();
				{
					for (GUI_RENDER callback : RegistryRender)
					{
						if (callback) { callback(); }
					}
				}
				GUI::Mutex.unlock();
				/* draw addons end*/
			}
			/* draw overlay end */

			/* end frame */
			ImGui::EndFrame();
			ImGui::Render();
			Renderer::DeviceContext->OMSetRenderTargets(1, &Renderer::RenderTargetView, NULL);
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			/* end frame end*/
		}

		/* post-render callbacks */
		GUI::Mutex.lock();
		{
			for (GUI_RENDER callback : RegistryPostRender)
			{
				if (callback) { callback(); }
			}
		}
		GUI::Mutex.unlock();
		/* post-render callbacks end*/
	}

	void AddWindow(IWindow* aWindowPtr)
	{
		GUI::Mutex.lock();
		{
			Windows.push_back(aWindowPtr);
		}
		GUI::Mutex.unlock();
	}

	void ImportArcDPSStyle()
	{
		if (LinkArcDPSStyle)
		{
			std::filesystem::path arcIniPath = Path::D_GW2_ADDONS / "arcdps/arcdps.ini";

			if (std::filesystem::exists(arcIniPath))
			{
				std::ifstream arcIni(arcIniPath);

				if (arcIni.is_open())
				{
					std::string line;
					std::string style = "appearance_imgui_style180=";
					std::string colours = "appearance_imgui_colours180=";

					while (std::getline(arcIni, line))
					{
						if (line.find(style, 0) != line.npos)
						{
							line = line.substr(style.length());

							ImGuiStyle* style = &ImGui::GetStyle();

							std::string decode = Base64::Decode(&line[0], line.length());

							memcpy(style, &decode[0], decode.length());
						}
						else if (line.find(colours, 0) != line.npos)
						{
							line = line.substr(colours.length());

							ImGuiStyle* style = &ImGui::GetStyle();

							std::string decode = Base64::Decode(&line[0], line.length());

							memcpy(&style->Colors[0], &decode[0], decode.length());
						}
					}
					arcIni.close();
				}
			}
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
			return;
		}
		else if (str == KB_ADDONS)
		{
			const auto& it = std::find_if(Windows.begin(), Windows.end(), [](const IWindow* wnd) { return wnd->Name == "Addons"; });
			if (it == Windows.end()) { return; }
			(*it)->Visible = !(*it)->Visible;
		}
		else if (str == KB_CHANGELOG)
		{
			const auto& it = std::find_if(Windows.begin(), Windows.end(), [](const IWindow* wnd) { return wnd->Name == "Changelog"; });
			if (it == Windows.end()) { return; }
			(*it)->Visible = !(*it)->Visible;
		}
		else if (str == KB_DEBUG)
		{
			const auto& it = std::find_if(Windows.begin(), Windows.end(), [](const IWindow* wnd) { return wnd->Name == "Debug"; });
			if (it == Windows.end()) { return; }
			(*it)->Visible = !(*it)->Visible;
		}
		else if (str == KB_LOG)
		{
			const auto& it = std::find_if(Windows.begin(), Windows.end(), [](const IWindow* wnd) { return wnd->Name == "Log"; });
			if (it == Windows.end()) { return; }
			(*it)->Visible = !(*it)->Visible;
		}
		else if (str == KB_OPTIONS)
		{
			const auto& it = std::find_if(Windows.begin(), Windows.end(), [](const IWindow* wnd) { return wnd->Name == "Options"; });
			if (it == Windows.end()) { return; }
			(*it)->Visible = !(*it)->Visible;
		}
		else if (str == KB_MUMBLEOVERLAY)
		{
			const auto& it = std::find_if(Windows.begin(), Windows.end(), [](const IWindow* wnd) { return wnd->Name == "Debug"; });
			if (it == Windows.end()) { return; }
			((DebugWindow*)(*it))->MumbleWindow->Visible = !((DebugWindow*)(*it))->MumbleWindow->Visible;
		}
	}
	
	void OnMumbleIdentityChanged(void* aEventArgs)
	{
		if (Renderer::Scaling != LastScaling && IsGameplay)
		{
			LastScaling = Renderer::Scaling;
			Settings::Settings[OPT_LASTUISCALE] = Renderer::Scaling;
			Settings::Save();
		}

		ImGuiIO& io = ImGui::GetIO();

		if (MumbleIdentity)
		{
			switch (MumbleIdentity->UISize)
			{
			case 0:
				Font = FontIndex[EFont::Menomonia_Small];
				FontBig = FontIndex[EFont::MenomoniaBig_Small];
				FontUI = FontIndex[EFont::Trebuchet_Small];
				break;
			default:
			case 1:
				Font = FontIndex[EFont::Menomonia_Normal];
				FontBig = FontIndex[EFont::MenomoniaBig_Normal];
				FontUI = FontIndex[EFont::Trebuchet_Normal];
				break;
			case 2:
				Font = FontIndex[EFont::Menomonia_Large];
				FontBig = FontIndex[EFont::MenomoniaBig_Large];
				FontUI = FontIndex[EFont::Trebuchet_Large];
				break;
			case 3:
				Font = FontIndex[EFont::Menomonia_Larger];
				FontBig = FontIndex[EFont::MenomoniaBig_Larger];
				FontUI = FontIndex[EFont::Trebuchet_Larger];
				break;
			}
		}
	}
	void OnEULAAccepted(void* aEventArgs)
	{
		Mutex.lock();
		auto it = std::find_if(Windows.begin(), Windows.end(), [](const IWindow* wnd) {
			return wnd->Name == "EULAModal";
		});

		if (it != Windows.end())
		{
			delete (*it);
			Windows.erase(it);
		}
		Mutex.unlock();
		Events::Unsubscribe(EV_EULA_ACCEPTED, OnEULAAccepted);
	}

	void Setup()
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
					Settings::Settings[OPT_FONTSIZE].get_to(FontSize);
				}
				else
				{
					Settings::Settings[OPT_FONTSIZE] = FontSize;
					Settings::Save();
				}
			}
			std::string strFont = Path::F_FONT.string();

			io.Fonts->AddFontFromFileTTF(strFont.c_str(), FontSize);
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

		GUI::Mutex.lock();
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
		GUI::Mutex.unlock();

		io.Fonts->Build();

		Events::Subscribe(EV_MUMBLE_IDENTITY_UPDATED, OnMumbleIdentityChanged);
		OnMumbleIdentityChanged(nullptr);

		/* set up and add windows */
		AddonsWindow* addonsWnd = new AddonsWindow("Addons");
		OptionsWindow* opsWnd = new OptionsWindow("Options");
		ChangelogWindow* chlWnd = new ChangelogWindow("Changelog");
		LogWindow* logWnd = new LogWindow("Log", ELogLevel::ALL);
		LogHandler::RegisterLogger(logWnd);
		DebugWindow* dbgWnd = new DebugWindow("Debug");
		AboutBox* aboutWnd = new AboutBox("About");

		Keybinds::Register(KB_ADDONS, ProcessKeybind, "(null)");
		Keybinds::Register(KB_OPTIONS, ProcessKeybind, "(null)");
		Keybinds::Register(KB_CHANGELOG, ProcessKeybind, "(null)");
		Keybinds::Register(KB_LOG, ProcessKeybind, "(null)");
		Keybinds::Register(KB_DEBUG, ProcessKeybind, "(null)");
		Keybinds::Register(KB_MUMBLEOVERLAY, ProcessKeybind, "(null)");

		AddWindow(addonsWnd);
		AddWindow(opsWnd);
		AddWindow(chlWnd);
		AddWindow(logWnd);
		AddWindow(dbgWnd);
		AddWindow(aboutWnd);

		Menu::AddMenuItem("Addons",		ICON_ADDONS,	&addonsWnd->Visible);
		Menu::AddMenuItem("Options",	ICON_OPTIONS,	&opsWnd->Visible);
		Menu::AddMenuItem("Changelog",	ICON_CHANGELOG, &chlWnd->Visible);
		Menu::AddMenuItem("Log",		ICON_LOG,		&logWnd->Visible);
		Menu::AddMenuItem("Debug",		ICON_DEBUG,		&dbgWnd->Visible);
		Menu::AddMenuItem("About",		ICON_ABOUT,		&aboutWnd->Visible);

		/* register keybinds */
		Keybinds::Register(KB_MENU, ProcessKeybind, "CTRL+O");
		Keybinds::Register(KB_TOGGLEHIDEUI, ProcessKeybind, "CTRL+H");

		/* load icons */
		std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		tm local_tm = *localtime(&t);
		int month = local_tm.tm_mon + 1;

		switch (month)
		{
		case 10:
			TextureLoader::LoadFromResource(ICON_NEXUS, RES_ICON_NEXUS_HALLOWEEN, NexusHandle, nullptr);
			TextureLoader::LoadFromResource(ICON_NEXUS_HOVER, RES_ICON_NEXUS_HALLOWEEN_HOVER, NexusHandle, nullptr);
			break;
		case 12:
			TextureLoader::LoadFromResource(ICON_NEXUS, RES_ICON_NEXUS_XMAS, NexusHandle, nullptr);
			TextureLoader::LoadFromResource(ICON_NEXUS_HOVER, RES_ICON_NEXUS_XMAS_HOVER, NexusHandle, nullptr);
			break;
		default:
			TextureLoader::LoadFromResource(ICON_NEXUS, RES_ICON_NEXUS, NexusHandle, nullptr);
			TextureLoader::LoadFromResource(ICON_NEXUS_HOVER, RES_ICON_NEXUS_HOVER, NexusHandle, nullptr);
			break;
		}

		TextureLoader::LoadFromResource(ICON_GENERIC, RES_ICON_GENERIC, NexusHandle, nullptr);
		TextureLoader::LoadFromResource(ICON_GENERIC_HOVER, RES_ICON_GENERIC_HOVER, NexusHandle, nullptr);

		TextureLoader::LoadFromResource(ICON_NOTIFICATION, RES_ICON_NOTIFICATION, NexusHandle, QuickAccess::ReceiveTextures);

		TextureLoader::LoadFromResource(ICON_ADDONS, RES_ICON_ADDONS, NexusHandle, nullptr);
		TextureLoader::LoadFromResource(ICON_OPTIONS, RES_ICON_OPTIONS, NexusHandle, nullptr);
		TextureLoader::LoadFromResource(ICON_CHANGELOG, RES_ICON_CHANGELOG, NexusHandle, nullptr);
		TextureLoader::LoadFromResource(ICON_LOG, RES_ICON_LOG, NexusHandle, nullptr);
		TextureLoader::LoadFromResource(ICON_DEBUG, RES_ICON_DEBUG, NexusHandle, nullptr);
		TextureLoader::LoadFromResource(ICON_ABOUT, RES_ICON_ABOUT, NexusHandle, nullptr);

		TextureLoader::LoadFromResource(TEX_MENU_BACKGROUND, RES_TEX_MENU_BACKGROUND, NexusHandle, Menu::ReceiveTextures);
		TextureLoader::LoadFromResource(TEX_MENU_BUTTON, RES_TEX_MENU_BUTTON, NexusHandle, Menu::ReceiveTextures);
		TextureLoader::LoadFromResource(TEX_MENU_BUTTON_HOVER, RES_TEX_MENU_BUTTON_HOVER, NexusHandle, Menu::ReceiveTextures);

		/* add shortcut */
		QuickAccess::AddShortcut(QA_MENU, ICON_NEXUS, ICON_NEXUS_HOVER, KB_MENU, "Nexus Menu");

		if (IsUpdateAvailable && NotifyChangelog)
		{
			QuickAccess::NotifyShortcut(QA_MENU);
		}

		if (!Settings::Settings.is_null())
		{
			if (!Settings::Settings[OPT_ACCEPTEULA].is_null())
			{
				Settings::Settings[OPT_ACCEPTEULA].get_to(HasAcceptedEULA);
			}
			else
			{
				HasAcceptedEULA = false;
			}

			if (!Settings::Settings[OPT_NOTIFYCHANGELOG].is_null())
			{
				Settings::Settings[OPT_NOTIFYCHANGELOG].get_to(NotifyChangelog);
			}
			else
			{
				NotifyChangelog = false;
			}

			if (!Settings::Settings[OPT_LASTUISCALE].is_null() && Renderer::Scaling == 0)
			{
				Settings::Settings[OPT_LASTUISCALE].get_to(LastScaling);
				Settings::Settings[OPT_LASTUISCALE].get_to(Renderer::Scaling);
			}
			else
			{
				LastScaling = SC_NORMAL;
				Renderer::Scaling = SC_NORMAL;
				Settings::Settings[OPT_LASTUISCALE] = SC_NORMAL;
			}

			if (!Settings::Settings[OPT_QAVERTICAL].is_null())
			{
				Settings::Settings[OPT_QAVERTICAL].get_to(QuickAccess::VerticalLayout);
			}
			else
			{
				QuickAccess::VerticalLayout = false;
				Settings::Settings[OPT_QAVERTICAL] = false;
			}

			if (!Settings::Settings[OPT_QALOCATION].is_null())
			{
				Settings::Settings[OPT_QALOCATION].get_to(QuickAccess::Location);
			}
			else
			{
				QuickAccess::Location = EQAPosition::Extend;
				Settings::Settings[OPT_QALOCATION] = 0;
			}

			if (!Settings::Settings[OPT_QAOFFSETX].is_null() && !Settings::Settings[OPT_QAOFFSETY].is_null())
			{
				Settings::Settings[OPT_QAOFFSETX].get_to(QuickAccess::Offset.x);
				Settings::Settings[OPT_QAOFFSETY].get_to(QuickAccess::Offset.y);
			}
			else
			{
				QuickAccess::Offset = ImVec2(0.0f, 0.0f);
				Settings::Settings[OPT_QAOFFSETX] = 0.0f;
				Settings::Settings[OPT_QAOFFSETY] = 0.0f;
			}

			if (!Settings::Settings[OPT_CLOSEMENU].is_null())
			{
				Settings::Settings[OPT_CLOSEMENU].get_to(CloseMenuAfterSelecting);
			}
			else
			{
				CloseMenuAfterSelecting = true;
				Settings::Settings[OPT_CLOSEMENU] = true;
			}

			if (!Settings::Settings[OPT_CLOSEESCAPE].is_null())
			{
				Settings::Settings[OPT_CLOSEESCAPE].get_to(CloseOnEscape);
			}
			else
			{
				CloseOnEscape = true;
				Settings::Settings[OPT_CLOSEESCAPE] = true;
			}

			if (!Settings::Settings[OPT_LINKARCSTYLE].is_null())
			{
				Settings::Settings[OPT_LINKARCSTYLE].get_to(LinkArcDPSStyle);
			}
			else
			{
				LinkArcDPSStyle = true;
				Settings::Settings[OPT_LINKARCSTYLE] = true;
			}
		}

		if (!HasAcceptedEULA)
		{
			EULAModal* eulaModal = new EULAModal();
			AddWindow(eulaModal);
			Events::Subscribe(EV_EULA_ACCEPTED, OnEULAAccepted);
		}

		ImportArcDPSStyle();

		IsSetup = true;
	}

	void Register(ERenderType aRenderType, GUI_RENDER aRenderCallback)
	{
		GUI::Mutex.lock();
		{
			switch (aRenderType)
			{
			case ERenderType::PreRender:
				RegistryPreRender.push_back(aRenderCallback);
				break;
			case ERenderType::Render:
				RegistryRender.push_back(aRenderCallback);
				break;
			case ERenderType::PostRender:
				RegistryPostRender.push_back(aRenderCallback);
				break;
			case ERenderType::OptionsRender:
				RegistryOptionsRender.push_back(aRenderCallback);
				break;
			}
		}
		GUI::Mutex.unlock();
	}
	void Deregister(GUI_RENDER aRenderCallback)
	{
		GUI::Mutex.lock();
		{
			RegistryPreRender.erase(std::remove(RegistryPreRender.begin(), RegistryPreRender.end(), aRenderCallback), RegistryPreRender.end());
			RegistryRender.erase(std::remove(RegistryRender.begin(), RegistryRender.end(), aRenderCallback), RegistryRender.end());
			RegistryPostRender.erase(std::remove(RegistryPostRender.begin(), RegistryPostRender.end(), aRenderCallback), RegistryPostRender.end());
			RegistryOptionsRender.erase(std::remove(RegistryOptionsRender.begin(), RegistryOptionsRender.end(), aRenderCallback), RegistryOptionsRender.end());
		}
		GUI::Mutex.unlock();
	}

	int Verify(void* aStartAddress, void* aEndAddress)
	{
		int refCounter = 0;

		GUI::Mutex.lock();
		{
			for (GUI_RENDER renderCb : RegistryPreRender)
			{
				if (renderCb >= aStartAddress && renderCb <= aEndAddress)
				{
					RegistryPreRender.erase(std::remove(RegistryPreRender.begin(), RegistryPreRender.end(), renderCb), RegistryPreRender.end());
					refCounter++;
				}
			}
			for (GUI_RENDER renderCb : RegistryRender)
			{
				if (renderCb >= aStartAddress && renderCb <= aEndAddress)
				{
					RegistryRender.erase(std::remove(RegistryRender.begin(), RegistryRender.end(), renderCb), RegistryRender.end());
					refCounter++;
				}
			}
			for (GUI_RENDER renderCb : RegistryPostRender)
			{
				if (renderCb >= aStartAddress && renderCb <= aEndAddress)
				{
					RegistryPostRender.erase(std::remove(RegistryPostRender.begin(), RegistryPostRender.end(), renderCb), RegistryPostRender.end());
					refCounter++;
				}
			}
			for (GUI_RENDER renderCb : RegistryOptionsRender)
			{
				if (renderCb >= aStartAddress && renderCb <= aEndAddress)
				{
					RegistryOptionsRender.erase(std::remove(RegistryOptionsRender.begin(), RegistryOptionsRender.end(), renderCb), RegistryOptionsRender.end());
					refCounter++;
				}
			}
		}
		GUI::Mutex.unlock();

		return refCounter;
	}
}