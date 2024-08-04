#include "GUI.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>

#include "Branch.h"
#include "Consts.h"
#include "Index.h"
#include "Renderer.h"
#include "Shared.h"
#include "State.h"
#include "Version.h"

#include "Services/Mumble/Reader.h"

#include "Events/EventHandler.h"
#include "Inputs/InputBinds/InputBindHandler.h"
#include "Loader/Loader.h"
#include "Services/DataLink/DataLink.h"
#include "Services/Settings/Settings.h"
#include "Services/Textures/TextureLoader.h"

#include "imgui/imgui.h"
#include "imgui/imgui_extensions.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

#include "Widgets/Alerts/Alerts.h"
#include "Widgets/Menu/Menu.h"
#include "Widgets/Menu/MenuItem.h"
#include "Widgets/QuickAccess/QuickAccess.h"

#include "resource.h"
#include "Util/Base64.h"
#include "Util/Inputs.h"
#include "Util/Resources.h"
#include "Util/Time.h"

#ifndef STRINGIFY
#define STRINGIFY(x) #x
#endif
#ifndef TOSTRING
#define TOSTRING(x) STRINGIFY(x)
#endif

#ifndef WATERMARK
#define WATERMARK __DATE__ " " __TIME__ " (v" TOSTRING(V_MAJOR) "." TOSTRING(V_MINOR) "." TOSTRING(V_BUILD) "." TOSTRING(V_REVISION) ") [" BRANCH_NAME "]"
#endif

namespace GUI
{
	/* FIXME: this needs to be dependency injected. Fix before 2024/06/30. */
	CFontManager& FontManager = CFontManager::GetInstance();

	CAddonsWindow*	AddonsWindow	= nullptr;
	COptionsWindow*	OptionsWindow	= nullptr;
	CLogWindow*		LogWindow		= nullptr;
	CDebugWindow*	DebugWindow		= nullptr;
	CAboutBox*		AboutWindow		= nullptr;
	CEULAModal*		EULAWindow		= nullptr;

	/* internal forward declarations */
	void OnMumbleIdentityChanged(void* aEventArgs);
	void OnLanguageChanged(void* aEventArgs);
	void Setup();

	Mumble::Data*							MumbleLink					= nullptr;
	NexusLinkData*							NexusLink					= nullptr;

	std::mutex								Mutex;
	std::vector<GUI_RENDER>					RegistryPreRender;
	std::vector<GUI_RENDER>					RegistryRender;
	std::vector<GUI_RENDER>					RegistryPostRender;
	std::vector<GUI_RENDER>					RegistryOptionsRender;
	std::unordered_map<std::string, bool*>	RegistryCloseOnEscape;

	std::map<EFont, ImFont*>				FontIndex;
	std::string								FontFile;
	float									FontSize					= 16.0f;
	bool									CloseMenuAfterSelecting		= true;
	bool									CloseOnEscape				= true;
	bool									LinkArcDPSStyle				= true;

	bool									IsUIVisible					= true;

	bool									IsRightClickHeld			= false;
	bool									IsLeftClickHeld				= false;
	bool									IsSetup						= false;
	float									LastScaling					= 1.0f;

	bool									HasAcceptedEULA				= false;
	bool									NotifyChangelog				= false;

	bool									ShowAddonsWindowAfterDUU	= false;

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
					KeyLParam keylp = LParamToKMF(lParam);

					if (!keylp.PreviousKeyState)
					{
						ImVector<ImGuiWindow*> windows = Renderer::GuiContext->Windows;

						for (int i = windows.Size - 1; i > 0; i--)
						{
							std::string windowName = windows[i]->Name;

							for (auto& [wndName, boolptr] : RegistryCloseOnEscape)
							{
								if (windowName == wndName && *boolptr)
								{
									*boolptr = false;
									return 0;
								}
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

		if (Language->Advance())
		{
			FontManager.Reload();
		}
		if (FontManager.Advance())
		{
			OnMumbleIdentityChanged(nullptr);
			Shutdown();
		}

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
				/* draw addons*/
				for (GUI_RENDER callback : RegistryRender)
				{
					if (callback) { callback(); }
				}
				/* draw addons end*/

				/* draw nexus windows */
				AddonsWindow->Render();
				OptionsWindow->Render();
				LogWindow->Render();
				DebugWindow->Render();
				AboutWindow->Render();
				if (!HasAcceptedEULA && EULAWindow)
				{
					EULAWindow->Render();
				}
				/* draw nexus windows end */

				/* draw menu & qa */
				Menu::Render();
				QuickAccess::Render();
				Alerts::Render();
				/* draw menu & qa end*/
			}
			/* draw overlay end */

#ifdef _DEBUG
			ImGui::PushFont(MonospaceFont);
			ImVec2 sz = ImGui::CalcTextSize(WATERMARK);

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::SetNextWindowPos(ImVec2((Renderer::Width - sz.x) / 2, 0));
			if (ImGui::Begin("NEXUS_BUILDINFO", (bool*)0, WindowFlags_Watermark))
			{
				ImGui::TextOutlined(WATERMARK);
			};
			ImGui::End();
			ImGui::PopStyleVar();
			ImGui::PopFont();
#endif

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

	void ImportArcDPSStyle()
	{
		if (LinkArcDPSStyle)
		{
			std::filesystem::path arcIniPath = Index::D_GW2_ADDONS / "arcdps/arcdps.ini";

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

					try
					{
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
					}
					catch (...)
					{
						Logger->Debug(CH_CORE, "Couldn't parse ArcDPS style.");
					}
					
					arcIni.close();
				}
			}
		}
	}

	void StyleColorsRaidcoreNexus()
	{
		return;

		ImGuiStyle* style = &ImGui::GetStyle();
		ImVec4* colors = style->Colors;

		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		colors[ImGuiCol_Tab] = ImLerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.80f);
		colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
		colors[ImGuiCol_TabActive] = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
		colors[ImGuiCol_TabUnfocused] = ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
		colors[ImGuiCol_TabUnfocusedActive] = ImLerp(colors[ImGuiCol_TabActive], colors[ImGuiCol_TitleBg], 0.40f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);   // Prefer using Alpha=1.0 here
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);   // Prefer using Alpha=1.0 here
		colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	}

	void ProcessInputBind(const char* aIdentifier)
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
			AddonsWindow->Visible = !AddonsWindow->Visible;
		}
		else if (str == KB_DEBUG)
		{
			DebugWindow->Visible = !DebugWindow->Visible;
		}
		else if (str == KB_LOG)
		{
			LogWindow->Visible = !LogWindow->Visible;
		}
		else if (str == KB_OPTIONS)
		{
			OptionsWindow->Visible = !OptionsWindow->Visible;
		}
		else if (str == KB_MUMBLEOVERLAY)
		{
			DebugWindow->MumbleWindow->Visible = !DebugWindow->MumbleWindow->Visible;
		}
	}

	void Rescale()
	{
		float currScaling = Mumble::IdentityParsed
			? Mumble::GetScalingFactor(Mumble::IdentityParsed->UISize)
			: 1.0f;

		ImGuiIO& io = ImGui::GetIO();
		Renderer::Scaling = currScaling * io.FontGlobalScale;
	}
	
	void OnMumbleIdentityChanged(void* aEventArgs)
	{
		if (Mumble::IdentityParsed)
		{
			float currScaling = Mumble::GetScalingFactor(Mumble::IdentityParsed->UISize);
			if (currScaling != LastScaling && NexusLink->IsGameplay)
			{
				Rescale();
				LastScaling = currScaling;
				Settings::Settings[OPT_LASTUISCALE] = currScaling;
				Settings::Save();
			}

			switch (Mumble::IdentityParsed->UISize)
			{
			case Mumble::EUIScale::Small:
				Font = FontIndex[EFont::Menomonia_Small];
				FontBig = FontIndex[EFont::MenomoniaBig_Small];
				FontUI = FontIndex[EFont::Trebuchet_Small];
				break;
			default:
			case Mumble::EUIScale::Normal:
				Font = FontIndex[EFont::Menomonia_Normal];
				FontBig = FontIndex[EFont::MenomoniaBig_Normal];
				FontUI = FontIndex[EFont::Trebuchet_Normal];
				break;
			case Mumble::EUIScale::Large:
				Font = FontIndex[EFont::Menomonia_Large];
				FontBig = FontIndex[EFont::MenomoniaBig_Large];
				FontUI = FontIndex[EFont::Trebuchet_Large];
				break;
			case Mumble::EUIScale::Larger:
				Font = FontIndex[EFont::Menomonia_Larger];
				FontBig = FontIndex[EFont::MenomoniaBig_Larger];
				FontUI = FontIndex[EFont::Trebuchet_Larger];
				break;
			}
		}
	}

	void OnLanguageChanged(void* aEventArgs)
	{
		int langId = *(int*)aEventArgs;

		switch (langId)
		{
		case 0:
			Language->SetLanguage("en");
			break;
		case 2:
			Language->SetLanguage("fr");
			break;
		case 3:
			Language->SetLanguage("de");
			break;
		case 4:
			Language->SetLanguage("es");
			break;
		/*case 5:
			Language.SetLanguage(u8"Chinese");
			break;*/
		}
	}

	void OnAddonDUU(void* aEventArgs)
	{
		if (!ShowAddonsWindowAfterDUU) { return; }

		AddonsWindow->Visible = true;
	}

	void FontReceiver(const char* aIdentifier, ImFont* aFont)
	{
		std::string str = aIdentifier;

		if (str == "USER_FONT")
		{
			UserFont = aFont;
			if (UserFont)
			{
				ImGuiIO& io = ImGui::GetIO();
				io.FontDefault = UserFont;
			}
		}
		if (str == "FONT_DEFAULT")
		{
			MonospaceFont = aFont;

			if (FontFile.empty())
			{
				UserFont = aFont;
			}
		}

		if (str == "MENOMONIA_S")		{ FontIndex[EFont::Menomonia_Small] = aFont; }
		if (str == "MENOMONIA_BIG_S")	{ FontIndex[EFont::MenomoniaBig_Small] = aFont; }
		if (str == "TREBUCHET_S")		{ FontIndex[EFont::Trebuchet_Small] = aFont; }

		if (str == "MENOMONIA_N")		{ FontIndex[EFont::Menomonia_Normal] = aFont; }
		if (str == "MENOMONIA_BIG_N")	{ FontIndex[EFont::MenomoniaBig_Normal] = aFont; }
		if (str == "TREBUCHET_N")		{ FontIndex[EFont::Trebuchet_Normal] = aFont; }

		if (str == "MENOMONIA_L")		{ FontIndex[EFont::Menomonia_Large] = aFont; }
		if (str == "MENOMONIA_BIG_L")	{ FontIndex[EFont::MenomoniaBig_Large] = aFont; }
		if (str == "TREBUCHET_L")		{ FontIndex[EFont::Trebuchet_Large] = aFont; }

		if (str == "MENOMONIA_XL")		{ FontIndex[EFont::Menomonia_Larger] = aFont; }
		if (str == "MENOMONIA_BIG_XL")	{ FontIndex[EFont::MenomoniaBig_Larger] = aFont; }
		if (str == "TREBUCHET_XL")		{ FontIndex[EFont::Trebuchet_Larger] = aFont; }
	}

	void LoadFonts()
	{
		std::filesystem::path fontPath{};

		/* add user font */
		if (!FontFile.empty() && std::filesystem::exists(Index::D_GW2_ADDONS_NEXUS_FONTS / FontFile))
		{
			fontPath = Index::D_GW2_ADDONS_NEXUS_FONTS / FontFile;
			FontManager.ReplaceFont("USER_FONT", FontSize, fontPath.string().c_str(), FontReceiver, nullptr);
		}
		else if (LinkArcDPSStyle && std::filesystem::exists(Index::D_GW2_ADDONS / "arcdps" / "arcdps_font.ttf"))
		{
			fontPath = Index::D_GW2_ADDONS / "arcdps" / "arcdps_font.ttf";
			FontManager.ReplaceFont("USER_FONT", FontSize, fontPath.string().c_str(), FontReceiver, nullptr);
		}

		/* add default font for monospace */
		FontManager.AddDefaultFont(FontReceiver);

		ImFontConfig config;
		config.MergeMode = true;

		/* small UI*/
		FontManager.ReplaceFont("MENOMONIA_S", 16.0f, RES_FONT_MENOMONIA, NexusHandle, FontReceiver, nullptr);
		if (!fontPath.empty()) { FontManager.ReplaceFont("MENOMONIA_S_MERGE", 16.0f, fontPath.string().c_str(), FontReceiver, &config); }
		FontManager.ReplaceFont("MENOMONIA_BIG_S", 22.0f, RES_FONT_MENOMONIA, NexusHandle, FontReceiver, nullptr);
		if (!fontPath.empty()) { FontManager.ReplaceFont("MENOMONIA_BIG_S_MERGE", 22.0f, fontPath.string().c_str(), FontReceiver, &config); }
		FontManager.ReplaceFont("TREBUCHET_S", 15.0f, RES_FONT_TREBUCHET, NexusHandle, FontReceiver, nullptr);
		if (!fontPath.empty()) { FontManager.ReplaceFont("TREBUCHET_S_MERGE", 15.0f, fontPath.string().c_str(), FontReceiver, &config); }

		/* normal UI*/
		FontManager.ReplaceFont("MENOMONIA_N", 18.0f, RES_FONT_MENOMONIA, NexusHandle, FontReceiver, nullptr);
		if (!fontPath.empty()) { FontManager.ReplaceFont("MENOMONIA_N_MERGE", 18.0f, fontPath.string().c_str(), FontReceiver, &config); }
		FontManager.ReplaceFont("MENOMONIA_BIG_N", 24.0f, RES_FONT_MENOMONIA, NexusHandle, FontReceiver, nullptr);
		if (!fontPath.empty()) { FontManager.ReplaceFont("MENOMONIA_BIG_N_MERGE", 24.0f, fontPath.string().c_str(), FontReceiver, &config); }
		FontManager.ReplaceFont("TREBUCHET_N", 16.0f, RES_FONT_TREBUCHET, NexusHandle, FontReceiver, nullptr);
		if (!fontPath.empty()) { FontManager.ReplaceFont("TREBUCHET_N_MERGE", 16.0f, fontPath.string().c_str(), FontReceiver, &config); }

		/* large UI*/
		FontManager.ReplaceFont("MENOMONIA_L", 20.0f, RES_FONT_MENOMONIA, NexusHandle, FontReceiver, nullptr);
		if (!fontPath.empty()) { FontManager.ReplaceFont("MENOMONIA_L_MERGE", 20.0f, fontPath.string().c_str(), FontReceiver, &config); }
		FontManager.ReplaceFont("MENOMONIA_BIG_L", 26.0f, RES_FONT_MENOMONIA, NexusHandle, FontReceiver, nullptr);
		if (!fontPath.empty()) { FontManager.ReplaceFont("MENOMONIA_BIG_L_MERGE", 26.0f, fontPath.string().c_str(), FontReceiver, &config); }
		FontManager.ReplaceFont("TREBUCHET_L", 17.5f, RES_FONT_TREBUCHET, NexusHandle, FontReceiver, nullptr);
		if (!fontPath.empty()) { FontManager.ReplaceFont("TREBUCHET_L_MERGE", 17.5f, fontPath.string().c_str(), FontReceiver, &config); }

		/* larger UI*/
		FontManager.ReplaceFont("MENOMONIA_XL", 22.0f, RES_FONT_MENOMONIA, NexusHandle, FontReceiver, nullptr);
		if (!fontPath.empty()) { FontManager.ReplaceFont("MENOMONIA_XL_MERGE", 22.0f, fontPath.string().c_str(), FontReceiver, &config); }
		FontManager.ReplaceFont("MENOMONIA_BIG_XL", 28.0f, RES_FONT_MENOMONIA, NexusHandle, FontReceiver, nullptr);
		if (!fontPath.empty()) { FontManager.ReplaceFont("MENOMONIA_BIG_XL_MERGE", 28.0f, fontPath.string().c_str(), FontReceiver, &config); }
		FontManager.ReplaceFont("TREBUCHET_XL", 19.5f, RES_FONT_TREBUCHET, NexusHandle, FontReceiver, nullptr);
		if (!fontPath.empty()) { FontManager.ReplaceFont("TREBUCHET_XL_MERGE", 19.5f, fontPath.string().c_str(), FontReceiver, &config); }
	}

	void Setup()
	{
		MumbleLink = (Mumble::Data*)DataLinkService->GetResource(DL_MUMBLE_LINK);
		NexusLink = (NexusLinkData*)DataLinkService->GetResource(DL_NEXUS_LINK);

		Language->SetLocaleDirectory(Index::D_GW2_ADDONS_NEXUS_LOCALES);
		Resources::Unpack(NexusHandle, Index::F_LOCALE_EN, RES_LOCALE_EN, "JSON");
		Resources::Unpack(NexusHandle, Index::F_LOCALE_DE, RES_LOCALE_DE, "JSON");
		Resources::Unpack(NexusHandle, Index::F_LOCALE_FR, RES_LOCALE_FR, "JSON");
		Resources::Unpack(NexusHandle, Index::F_LOCALE_ES, RES_LOCALE_ES, "JSON");
		Resources::Unpack(NexusHandle, Index::F_LOCALE_BR, RES_LOCALE_BR, "JSON");
		Resources::Unpack(NexusHandle, Index::F_LOCALE_CZ, RES_LOCALE_CZ, "JSON");
		Resources::Unpack(NexusHandle, Index::F_LOCALE_IT, RES_LOCALE_IT, "JSON");
		Resources::Unpack(NexusHandle, Index::F_LOCALE_PL, RES_LOCALE_PL, "JSON");
		Resources::Unpack(NexusHandle, Index::F_LOCALE_RU, RES_LOCALE_RU, "JSON");
		Resources::Unpack(NexusHandle, Index::F_LOCALE_CN, RES_LOCALE_CN, "JSON");
		Language->Advance(); // advance once to build lang atlas prior to creation of Quick Access

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

			if (!Settings::Settings[OPT_USERFONT].is_null())
			{
				Settings::Settings[OPT_USERFONT].get_to(FontFile);
			}
			else
			{
				FontFile = "font.ttf";
			}

			if (!std::filesystem::exists(Index::D_GW2_ADDONS_NEXUS_FONTS / FontFile))
			{
				FontFile = "";
			}

			if (!Settings::Settings[OPT_FONTSIZE].is_null())
			{
				Settings::Settings[OPT_FONTSIZE].get_to(FontSize);
			}
			else
			{
				FontSize = 16.0f;
			}

			if (!Settings::Settings[OPT_LASTUISCALE].is_null())
			{
				Settings::Settings[OPT_LASTUISCALE].get_to(LastScaling);
			}
			else
			{
				LastScaling = SC_NORMAL;
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
				Settings::Settings[OPT_QALOCATION] = EQAPosition::Extend;
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
				Language->SetLanguage(Settings::Settings[OPT_LANGUAGE].get<std::string>());
			}
			else
			{
				Language->SetLanguage("en");
			}

			/* legacy quick access visibility */
			if (!Settings::Settings[OPT_ALWAYSSHOWQUICKACCESS].is_null())
			{
				/* delete legacy key */
				Settings::Settings.erase(OPT_ALWAYSSHOWQUICKACCESS);
			}

			if (!Settings::Settings[OPT_QAVISIBILITY].is_null())
			{
				Settings::Settings[OPT_QAVISIBILITY].get_to(QuickAccess::Visibility);
			}
			else
			{
				QuickAccess::Visibility = EQAVisibility::AlwaysShow;
				Settings::Settings[OPT_QAVISIBILITY] = EQAVisibility::AlwaysShow;
			}

			ImGuiIO& io = ImGui::GetIO();

			if (!Settings::Settings[OPT_GLOBALSCALE].is_null())
			{
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

			if (!Settings::Settings[OPT_SHOWADDONSWINDOWAFTERDUU].is_null())
			{
				Settings::Settings[OPT_SHOWADDONSWINDOWAFTERDUU].get_to(ShowAddonsWindowAfterDUU);
			}
			else
			{
				Settings::Settings[OPT_SHOWADDONSWINDOWAFTERDUU] = false;
			}

			Renderer::Scaling = LastScaling * io.FontGlobalScale;
		}

		StyleColorsRaidcoreNexus();
		ImportArcDPSStyle();

		LoadFonts();

		EventApi->Subscribe(EV_MUMBLE_IDENTITY_UPDATED, OnMumbleIdentityChanged, true);
		EventApi->Subscribe("EV_UNOFFICIAL_EXTRAS_LANGUAGE_CHANGED", OnLanguageChanged, true);
		EventApi->Subscribe(EV_VOLATILE_ADDON_DISABLED, OnAddonDUU, true);
		OnMumbleIdentityChanged(nullptr);

		AddonsWindow	= new CAddonsWindow("Addons");
		OptionsWindow	= new COptionsWindow("Options");
		LogWindow		= new CLogWindow("Log", ELogLevel::ALL);
		DebugWindow		= new CDebugWindow("Debug");
		AboutWindow		= new CAboutBox("About");

		Logger->RegisterLogger(GUI::LogWindow);

		/* add menu items */
		Menu::AddMenuItem("((000083))",		ICON_RETURN,	RES_ICON_RETURN,	&Menu::Visible);
		Menu::AddMenuItem("((000003))",		ICON_ADDONS,	RES_ICON_ADDONS,	&AddonsWindow->Visible);
		Menu::AddMenuItem("((000004))",		ICON_OPTIONS,	RES_ICON_OPTIONS,	&OptionsWindow->Visible);
		Menu::AddMenuItem("((000006))",		ICON_LOG,		RES_ICON_LOG,		&LogWindow->Visible);
		if (State::IsDeveloperMode)
		{
			Menu::AddMenuItem("((000007))", ICON_DEBUG, RES_ICON_DEBUG, &DebugWindow->Visible);
		}
		Menu::AddMenuItem("((000008))",		ICON_ABOUT,		RES_ICON_ABOUT,		&AboutWindow->Visible);

		/* register close on escape */
		RegisterCloseOnEscape("Menu", &Menu::Visible);
		RegisterCloseOnEscape("Addons", &AddonsWindow->Visible);
		RegisterCloseOnEscape("Options", &OptionsWindow->Visible);
		RegisterCloseOnEscape("Log", &LogWindow->Visible);
		RegisterCloseOnEscape("Debug", &DebugWindow->Visible);
		RegisterCloseOnEscape("Dear ImGui Metrics/Debugger", &DebugWindow->IsMetricsWindowVisible);
		RegisterCloseOnEscape("Memory Viewer", &DebugWindow->MemoryViewer.Open);
		RegisterCloseOnEscape("About", &AboutWindow->Visible);
		
		/* register InputBinds */
		InputBindApi->Register(KB_MENU, EIBHType::DownOnly, ProcessInputBind, "CTRL+O");
		InputBindApi->Register(KB_ADDONS, EIBHType::DownOnly, ProcessInputBind, "(null)");
		InputBindApi->Register(KB_OPTIONS, EIBHType::DownOnly, ProcessInputBind, "(null)");
		InputBindApi->Register(KB_LOG, EIBHType::DownOnly, ProcessInputBind, "(null)");
		InputBindApi->Register(KB_DEBUG, EIBHType::DownOnly, ProcessInputBind, "(null)");
		InputBindApi->Register(KB_MUMBLEOVERLAY, EIBHType::DownOnly, ProcessInputBind, "(null)");
		InputBindApi->Register(KB_TOGGLEHIDEUI, EIBHType::DownOnly, ProcessInputBind, "CTRL+H");

		/* load icons */
		int month = Time::GetMonth();

		switch (month)
		{
		case 10:
			TextureService->Load(ICON_NEXUS, RES_ICON_NEXUS_HALLOWEEN, NexusHandle, nullptr);
			TextureService->Load(ICON_NEXUS_HOVER, RES_ICON_NEXUS_HALLOWEEN_HOVER, NexusHandle, nullptr);
			break;
		case 12:
			TextureService->Load(ICON_NEXUS, RES_ICON_NEXUS_XMAS, NexusHandle, nullptr);
			TextureService->Load(ICON_NEXUS_HOVER, RES_ICON_NEXUS_XMAS_HOVER, NexusHandle, nullptr);
			break;
		default:
			TextureService->Load(ICON_NEXUS, RES_ICON_NEXUS, NexusHandle, nullptr);
			TextureService->Load(ICON_NEXUS_HOVER, RES_ICON_NEXUS_HOVER, NexusHandle, nullptr);
			break;
		}

		TextureService->Load(ICON_GENERIC, RES_ICON_GENERIC, NexusHandle, nullptr);
		TextureService->Load(ICON_GENERIC_HOVER, RES_ICON_GENERIC_HOVER, NexusHandle, nullptr);

		/* add shortcut */
		QuickAccess::AddShortcut(QA_MENU, ICON_NEXUS, ICON_NEXUS_HOVER, KB_MENU, "((000009))");

		if (IsUpdateAvailable && NotifyChangelog)
		{
			QuickAccess::NotifyShortcut(QA_MENU);
		}

		if (!HasAcceptedEULA)
		{
			EULAWindow = new CEULAModal();
		}

		IsSetup = true;
	}

	void Register(ERenderType aRenderType, GUI_RENDER aRenderCallback)
	{
		if (!aRenderCallback) { return; }

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

	void RegisterCloseOnEscape(const char* aWindowName, bool* aIsVisible)
	{
		const std::lock_guard<std::mutex> lock(Mutex);

		RegistryCloseOnEscape[aWindowName] = aIsVisible;
	}
	void DeregisterCloseOnEscape(const char* aWindowName)
	{
		const std::lock_guard<std::mutex> lock(Mutex);

		RegistryCloseOnEscape.erase(aWindowName);
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
		for (auto& [windowname, boolptr] : RegistryCloseOnEscape)
		{
			if (boolptr >= aStartAddress && boolptr <= aEndAddress)
			{
				RegistryCloseOnEscape.erase(windowname);
				refCounter++;
			}
		}

		FontManager.Verify(aStartAddress, aEndAddress);

		return refCounter;
	}
}
