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
#include "Widgets/Addons/CAddonsWindow.h"
#include "Widgets/Options/COptionsWindow.h"
#include "Widgets/Changelog/CChangelogWindow.h"
#include "Widgets/Log/CLogWindow.h"
#include "Widgets/Debug/CDebugWindow.h"
#include "Widgets/About/CAboutBox.h"
#include "Widgets/QuickAccess/QuickAccess.h"
#include "Widgets/EULA/CEULAModal.h"
#include "Widgets/Alerts/Alerts.h"

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
		const std::lock_guard<std::mutex> lock(GUI::Mutex);

		/* pre-render callbacks */
		for (GUI_RENDER callback : RegistryPreRender)
		{
			if (callback) { callback(); }
		}
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
				Alerts::Render();
				/* draw menu & qa end*/

				/* draw nexus windows */
				for (IWindow* wnd : Windows)
				{
					wnd->Render();
				}
				/* draw nexus windows end */

				/* draw addons*/
				for (GUI_RENDER callback : RegistryRender)
				{
					if (callback) { callback(); }
				}
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
		for (GUI_RENDER callback : RegistryPostRender)
		{
			if (callback) { callback(); }
		}
		/* post-render callbacks end*/
	}

	void AddWindow(IWindow* aWindowPtr)
	{
		const std::lock_guard<std::mutex> lock(Mutex);
		
		Windows.push_back(aWindowPtr);
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
					std::string styleKey = "appearance_imgui_style180=";
					std::string coloursKey = "appearance_imgui_colours180=";
					std::string fontSizeKey = "font_size=";

					ImGuiStyle* style = &ImGui::GetStyle();

					while (std::getline(arcIni, line))
					{
						if (line.find(styleKey, 0) != line.npos)
						{
							line = line.substr(styleKey.length());
							std::string decode = Base64::Decode(&line[0], line.length());
							memcpy(style, &decode[0], decode.length());
						}
						else if (line.find(coloursKey, 0) != line.npos)
						{
							line = line.substr(coloursKey.length());
							std::string decode = Base64::Decode(&line[0], line.length());
							memcpy(&style->Colors[0], &decode[0], decode.length());
						}
						else if (line.find(fontSizeKey, 0) != line.npos)
						{
							line = line.substr(fontSizeKey.length());
							FontSize = std::stof(line);
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
			((CDebugWindow*)(*it))->MumbleWindow->Visible = !((CDebugWindow*)(*it))->MumbleWindow->Visible;
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

	void Setup()
	{
		ImGuiIO& io = ImGui::GetIO();

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

			if (!Settings::Settings[OPT_FONTSIZE].is_null())
			{
				Settings::Settings[OPT_FONTSIZE].get_to(FontSize);
			}
			else
			{
				FontSize = 16.0f;
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

			ImGuiStyle* style = &ImGui::GetStyle();
			if (!Settings::Settings[OPT_IMGUISTYLE].is_null())
			{
				std::string style64 = Settings::Settings[OPT_IMGUISTYLE].get<std::string>();
				std::string decode = Base64::Decode(&style64[0], style64.length());
				memcpy(style, &decode[0], decode.length());
			}

			if (!Settings::Settings[OPT_IMGUICOLORS].is_null())
			{
				std::string colors64 = Settings::Settings[OPT_IMGUICOLORS].get<std::string>();
				std::string decode = Base64::Decode(&colors64[0], colors64.length());
				memcpy(&style->Colors[0], &decode[0], decode.length());
			}

			if (!Settings::Settings[OPT_LANGUAGE].is_null())
			{
				Language.SetLanguage(Settings::Settings[OPT_LANGUAGE].get<std::string>());
			}
			else
			{
				Language.SetLanguage("en");
			}

			if (!Settings::Settings[OPT_ALWAYSSHOWQUICKACCESS].is_null())
			{
				Settings::Settings[OPT_ALWAYSSHOWQUICKACCESS].get_to(QuickAccess::AlwaysShow);
			}
			else
			{
				Settings::Settings[OPT_ALWAYSSHOWQUICKACCESS] = false;
			}

			if (!Settings::Settings[OPT_GLOBALSCALE].is_null())
			{
				ImGuiIO& io = ImGui::GetIO();
				Settings::Settings[OPT_GLOBALSCALE].get_to(io.FontGlobalScale);
				
				if (io.FontGlobalScale < 0.75f)
				{
					io.FontGlobalScale = 0.75f;
				}
			}
			else
			{
				Settings::Settings[OPT_GLOBALSCALE] = 1.0f;
			}
		}

		ImportArcDPSStyle();

		std::filesystem::path fontPath{};

		ImVector<ImWchar> ranges;

		ImFontGlyphRangesBuilder rb{};
		ImWchar rangesLatinExt[] =
		{
			0x0100, 0x017F,
			0x0180, 0x024F,
			0,
		};
		rb.AddRanges(io.Fonts->GetGlyphRangesDefault());
		rb.AddRanges(rangesLatinExt);
		rb.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
		rb.AddRanges(io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
		rb.BuildRanges(&ranges);

		/* add user font, or fallback to default */
		if (!LinkArcDPSStyle && std::filesystem::exists(Path::F_FONT))
		{
			fontPath = Path::F_FONT;
			std::string strFont = Path::F_FONT.string();
			io.Fonts->AddFontFromFileTTF(strFont.c_str(), FontSize, nullptr, ranges.Data);
		}
		else if (LinkArcDPSStyle && std::filesystem::exists(Path::D_GW2_ADDONS / "arcdps" / "arcdps_font.ttf"))
		{
			fontPath = Path::D_GW2_ADDONS / "arcdps" / "arcdps_font.ttf";
			std::string strFont = (Path::D_GW2_ADDONS / "arcdps" / "arcdps_font.ttf").string();
			io.Fonts->AddFontFromFileTTF(strFont.c_str(), FontSize, nullptr, ranges.Data);
		}
		else
		{
			io.Fonts->AddFontDefault();
		}

		/* load gw2 fonts */
		LPVOID resM{}; DWORD szM{};
		GetResource(NexusHandle, MAKEINTRESOURCE(RES_FONT_MENOMONIA), RT_FONT, &resM, &szM);

		LPVOID resT{}; DWORD szT{};
		GetResource(NexusHandle, MAKEINTRESOURCE(RES_FONT_TREBUCHET), RT_FONT, &resT, &szT);

		{
			const std::lock_guard<std::mutex> lock(Mutex);

			ImFontConfig config;
			config.MergeMode = true;

			/* small UI*/
			FontIndex.emplace(EFont::Menomonia_Small, io.Fonts->AddFontFromMemoryTTF(resM, szM, 16.0f, 0, ranges.Data));
			if (!fontPath.empty()) { io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), 16.0f, &config, ranges.Data); }
			FontIndex.emplace(EFont::MenomoniaBig_Small, io.Fonts->AddFontFromMemoryTTF(resM, szM, 22.0f, 0, ranges.Data));
			if (!fontPath.empty()) { io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), 22.0f, &config, ranges.Data); }
			FontIndex.emplace(EFont::Trebuchet_Small, io.Fonts->AddFontFromMemoryTTF(resT, szT, 15.0f, 0, ranges.Data));
			if (!fontPath.empty()) { io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), 15.0f, &config, ranges.Data); }

			/* normal UI*/
			FontIndex.emplace(EFont::Menomonia_Normal, io.Fonts->AddFontFromMemoryTTF(resM, szM, 18.0f, 0, ranges.Data));
			if (!fontPath.empty()) { io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), 18.0f, &config, ranges.Data); }
			FontIndex.emplace(EFont::MenomoniaBig_Normal, io.Fonts->AddFontFromMemoryTTF(resM, szM, 24.0f, 0, ranges.Data));
			if (!fontPath.empty()) { io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), 24.0f, &config, ranges.Data); }
			FontIndex.emplace(EFont::Trebuchet_Normal, io.Fonts->AddFontFromMemoryTTF(resT, szT, 16.0f, 0, ranges.Data));
			if (!fontPath.empty()) { io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), 16.0f, &config, ranges.Data); }

			/* large UI*/
			FontIndex.emplace(EFont::Menomonia_Large, io.Fonts->AddFontFromMemoryTTF(resM, szM, 20.0f, 0, ranges.Data));
			if (!fontPath.empty()) { io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), 20.0f, &config, ranges.Data); }
			FontIndex.emplace(EFont::MenomoniaBig_Large, io.Fonts->AddFontFromMemoryTTF(resM, szM, 26.0f, 0, ranges.Data));
			if (!fontPath.empty()) { io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), 26.0f, &config, ranges.Data); }
			FontIndex.emplace(EFont::Trebuchet_Large, io.Fonts->AddFontFromMemoryTTF(resT, szT, 17.5f, 0, ranges.Data));
			if (!fontPath.empty()) { io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), 17.5f, &config, ranges.Data); }

			/* larger UI*/
			FontIndex.emplace(EFont::Menomonia_Larger, io.Fonts->AddFontFromMemoryTTF(resM, szM, 22.0f, 0, ranges.Data));
			if (!fontPath.empty()) { io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), 22.0f, &config, ranges.Data); }
			FontIndex.emplace(EFont::MenomoniaBig_Larger, io.Fonts->AddFontFromMemoryTTF(resM, szM, 28.0f, 0, ranges.Data));
			if (!fontPath.empty()) { io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), 28.0f, &config, ranges.Data); }
			FontIndex.emplace(EFont::Trebuchet_Larger, io.Fonts->AddFontFromMemoryTTF(resT, szT, 19.5f, 0, ranges.Data));
			if (!fontPath.empty()) { io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), 19.5f, &config, ranges.Data); }
		}

		io.Fonts->Build();

		Events::Subscribe(EV_MUMBLE_IDENTITY_UPDATED, OnMumbleIdentityChanged);
		OnMumbleIdentityChanged(nullptr);

		/* set up and add windows */
		CAddonsWindow* addonsWnd = new CAddonsWindow("Addons");

		COptionsWindow* opsWnd = new COptionsWindow("Options");
		CChangelogWindow* chlWnd = new CChangelogWindow("Changelog");
		CLogWindow* logWnd = new CLogWindow("Log", ELogLevel::ALL);
		LogHandler::RegisterLogger(logWnd);
		CDebugWindow* dbgWnd = new CDebugWindow("Debug");
		CAboutBox* aboutWnd = new CAboutBox("About");

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

		Menu::AddMenuItem("((000003))",		ICON_ADDONS,	RES_ICON_ADDONS,	&addonsWnd->Visible);
		Menu::AddMenuItem("((000004))",		ICON_OPTIONS,	RES_ICON_OPTIONS,	&opsWnd->Visible);
		Menu::AddMenuItem("((000005))",		ICON_CHANGELOG, RES_ICON_CHANGELOG,	&chlWnd->Visible);
		Menu::AddMenuItem("((000006))",		ICON_LOG,		RES_ICON_LOG,		&logWnd->Visible);
		Menu::AddMenuItem("((000007))",		ICON_DEBUG,		RES_ICON_DEBUG,		&dbgWnd->Visible);
		Menu::AddMenuItem("((000008))",		ICON_ABOUT,		RES_ICON_ABOUT,		&aboutWnd->Visible);

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

		/* add shortcut */
		QuickAccess::AddShortcut(QA_MENU, ICON_NEXUS, ICON_NEXUS_HOVER, KB_MENU, Language.Translate("((000009))"));

		if (IsUpdateAvailable && NotifyChangelog)
		{
			QuickAccess::NotifyShortcut(QA_MENU);
		}

		if (!HasAcceptedEULA)
		{
			CEULAModal* eulaModal = new CEULAModal();
			AddWindow(eulaModal);
		}

		IsSetup = true;
	}

	void Register(ERenderType aRenderType, GUI_RENDER aRenderCallback)
	{
		const std::lock_guard<std::mutex> lock(Mutex);

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
	void Deregister(GUI_RENDER aRenderCallback)
	{
		const std::lock_guard<std::mutex> lock(Mutex);

		RegistryPreRender.erase(std::remove(RegistryPreRender.begin(), RegistryPreRender.end(), aRenderCallback), RegistryPreRender.end());
		RegistryRender.erase(std::remove(RegistryRender.begin(), RegistryRender.end(), aRenderCallback), RegistryRender.end());
		RegistryPostRender.erase(std::remove(RegistryPostRender.begin(), RegistryPostRender.end(), aRenderCallback), RegistryPostRender.end());
		RegistryOptionsRender.erase(std::remove(RegistryOptionsRender.begin(), RegistryOptionsRender.end(), aRenderCallback), RegistryOptionsRender.end());
	}

	int Verify(void* aStartAddress, void* aEndAddress)
	{
		int refCounter = 0;

		const std::lock_guard<std::mutex> lock(Mutex);
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

		return refCounter;
	}
}
