///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ApiFunctionMapper.cpp
/// Description  :  Contains the logic to map engine functions to the exposed API definitions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "ApiFunctionMapper.h"

#include <assert.h>

#include "Context.h"
#include "Events/EvtApi.h"
#include "Index.h"
#include "Inputs/GameBinds/GbApi.h"
#include "Inputs/InputBinds/IbApi.h"
#include "Inputs/RawInput/RiApi.h"
#include "Renderer.h"
#include "Services/DataLink/DlApi.h"
#include "Services/Localization/Localization.h"
#include "Services/Logging/LogHandler.h"
#include "Services/Textures/TxLoader.h"
#include "Services/Updater/Updater.h"
#include "UI/Services/Fonts/FontManager.h"
#include "UI/Services/QoL/EscapeClosing.h"
#include "UI/UiContext.h"
#include "UI/Widgets/Alerts/Alerts.h"
#include "UI/Widgets/QuickAccess/QuickAccess.h"

namespace ADDONAPI
{
	static bool            s_IsInitialized = false;

	static CDataLinkApi*   s_DataLinkApi   = nullptr;
	static CEventApi*      s_EventApi      = nullptr;
	static CGameBindsApi*  s_GameBindsApi  = nullptr;
	static CInputBindApi*  s_InputBindApi  = nullptr;
	static CRawInputApi*   s_RawInputApi   = nullptr;
	static CLocalization*  s_Localization  = nullptr;
	static CLogHandler*    s_Logger        = nullptr;
	static CTextureLoader* s_TextureApi    = nullptr;
	static CUpdater*       s_Updater       = nullptr;

	static CUiContext*     s_UiContext     = nullptr;
	static CFontManager*   s_FontManager   = nullptr;
	static CAlerts*        s_Alerts        = nullptr;
	static CQuickAccess*   s_QuickAccess   = nullptr;
	static CEscapeClosing* s_EscapeClosing = nullptr;


	namespace DataLink
	{
		void* GetResource(const char* aIdentifier)
		{
			assert(s_DataLinkApi);
			return s_DataLinkApi->GetResource(aIdentifier);
		}

		void* ShareResource(const char* aIdentifier, size_t aResourceSize)
		{
			assert(s_DataLinkApi);
			return s_DataLinkApi->ShareResource(aIdentifier, aResourceSize, "", true);
		}
	}

	namespace Events
	{
		void Subscribe(const char* aIdentifier, EVENT_CONSUME aConsumeEventCallback)
		{
			assert(s_EventApi);
			s_EventApi->Subscribe(aIdentifier, aConsumeEventCallback);
		}

		void Unsubscribe(const char* aIdentifier, EVENT_CONSUME aConsumeEventCallback)
		{
			assert(s_EventApi);
			s_EventApi->Unsubscribe(aIdentifier, aConsumeEventCallback);
		}

		void RaiseEvent(const char* aIdentifier, void* aEventData)
		{
			assert(s_EventApi);
			s_EventApi->Raise(aIdentifier, aEventData);
		}

		void RaiseNotification(const char* aIdentifier)
		{
			assert(s_EventApi);
			s_EventApi->Raise(aIdentifier, nullptr);
		}

		void RaiseEventTargeted(signed int aSignature, const char* aIdentifier, void* aEventData)
		{
			assert(s_EventApi);
			s_EventApi->Raise(aSignature, aIdentifier, aEventData);
		}

		void RaiseNotificationTargeted(signed int aSignature, const char* aIdentifier)
		{
			assert(s_EventApi);
			s_EventApi->Raise(aSignature, aIdentifier, nullptr);
		}
	}

	namespace GameBinds
	{
		void PressAsync(EGameBinds aGameBind)
		{
			assert(s_GameBindsApi);
			s_GameBindsApi->PressAsync(aGameBind);
		}

		void ReleaseAsync(EGameBinds aGameBind)
		{
			assert(s_GameBindsApi);
			s_GameBindsApi->ReleaseAsync(aGameBind);
		}

		void InvokeAsync(EGameBinds aGameBind, int aDuration)
		{
			assert(s_GameBindsApi);
			s_GameBindsApi->InvokeAsync(aGameBind, aDuration);
		}

		void Press(EGameBinds aGameBind)
		{
			assert(s_GameBindsApi);
			s_GameBindsApi->Press(aGameBind);
		}

		void Release(EGameBinds aGameBind)
		{
			assert(s_GameBindsApi);
			s_GameBindsApi->Release(aGameBind);
		}

		bool IsBound(EGameBinds aGameBind)
		{
			assert(s_GameBindsApi);
			return s_GameBindsApi->IsBound(aGameBind);
		}
	}

	namespace InputBinds
	{
		void RegisterWithString(const char* aIdentifier, INPUTBINDS_PROCESS aInputBindHandler, const char* aInputBind)
		{
			assert(s_InputBindApi);
			s_InputBindApi->Register(aIdentifier, EIbHandlerType::DownAsync, aInputBindHandler, aInputBind);
		}

		void RegisterWithStruct(const char* aIdentifier, INPUTBINDS_PROCESS aInputBindHandler, InputBindV1 aInputBind)
		{
			assert(s_InputBindApi);
			s_InputBindApi->Register(aIdentifier, EIbHandlerType::DownAsync, aInputBindHandler, aInputBind);
		}

		void RegisterWithString2(const char* aIdentifier, INPUTBINDS_PROCESS2 aInputBindHandler, const char* aInputBind)
		{
			assert(s_InputBindApi);
			s_InputBindApi->Register(aIdentifier, EIbHandlerType::DownReleaseAsync, aInputBindHandler, aInputBind);
		}

		void RegisterWithStruct2(const char* aIdentifier, INPUTBINDS_PROCESS2 aInputBindHandler, InputBindV1 aInputBind)
		{
			assert(s_InputBindApi);
			s_InputBindApi->Register(aIdentifier, EIbHandlerType::DownReleaseAsync, aInputBindHandler, aInputBind);
		}

		void InvokeInputBind(const char* aIdentifier, bool aIsRelease)
		{
			assert(s_InputBindApi);
			s_InputBindApi->Invoke(aIdentifier, aIsRelease);
		}

		void Deregister(const char* aIdentifier)
		{
			assert(s_InputBindApi);
			s_InputBindApi->Deregister(aIdentifier);
		}
	}

	namespace RawInput
	{
		LRESULT SendWndProcToGame(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			assert(s_RawInputApi);
			return s_RawInputApi->SendWndProcToGame(hWnd, uMsg, wParam, lParam);
		}

		void Register(WNDPROC_CALLBACK aWndProcCallback)
		{
			assert(s_RawInputApi);
			s_RawInputApi->Register(aWndProcCallback);
		}

		void Deregister(WNDPROC_CALLBACK aWndProcCallback)
		{
			assert(s_RawInputApi);
			s_RawInputApi->Deregister(aWndProcCallback);
		}
	}

	namespace Localization
	{
		const char* Translate(const char* aIdentifier)
		{
			assert(s_Localization);
			return s_Localization->Translate(aIdentifier);
		}

		const char* TranslateTo(const char* aIdentifier, const char* aLanguageIdentifier)
		{
			assert(s_Localization);
			return s_Localization->Translate(aIdentifier, aLanguageIdentifier);
		}

		void Set(const char* aIdentifier, const char* aLanguageIdentifier, const char* aString)
		{
			assert(s_Localization);
			s_Localization->Set(aIdentifier, aLanguageIdentifier, aString);
		}
	}

	namespace Logger
	{
		void LogMessage(ELogLevel aLogLevel, const char* aStr)
		{
			assert(s_Logger);
			s_Logger->LogMessageUnformatted(aLogLevel, "Addon", aStr);
		}

		void LogMessage2(ELogLevel aLogLevel, const char* aChannel, const char* aStr)
		{
			assert(s_Logger);
			s_Logger->LogMessageUnformatted(aLogLevel, aChannel, aStr);
		}
	}

	namespace TextureLoader
	{
		Texture* Get(const char* aIdentifier)
		{
			assert(s_TextureApi);
			return s_TextureApi->Get(aIdentifier);
		}

		Texture* GetOrCreateFromFile(const char* aIdentifier, const char* aFilename)
		{
			assert(s_TextureApi);
			return s_TextureApi->GetOrCreate(aIdentifier, aFilename);
		}

		Texture* GetOrCreateFromResource(const char* aIdentifier, unsigned aResourceID, HMODULE aModule)
		{
			assert(s_TextureApi);
			return s_TextureApi->GetOrCreate(aIdentifier, aResourceID, aModule);;
		}

		Texture* GetOrCreateFromURL(const char* aIdentifier, const char* aRemote, const char* aEndpoint)
		{
			assert(s_TextureApi);
			return s_TextureApi->GetOrCreate(aIdentifier, aRemote, aEndpoint);
		}

		Texture* GetOrCreateFromMemory(const char* aIdentifier, void* aData, size_t aSize)
		{
			assert(s_TextureApi);
			return s_TextureApi->GetOrCreate(aIdentifier, aData, aSize);
		}

		void LoadFromFile(const char* aIdentifier, const char* aFilename, TEXTURES_RECEIVECALLBACK aCallback)
		{
			assert(s_TextureApi);
			s_TextureApi->Load(aIdentifier, aFilename, aCallback, true);
		}

		void LoadFromResource(const char* aIdentifier, unsigned aResourceID, HMODULE aModule, TEXTURES_RECEIVECALLBACK aCallback)
		{
			assert(s_TextureApi);
			s_TextureApi->Load(aIdentifier, aResourceID, aModule, aCallback, true);
		}

		void LoadFromURL(const char* aIdentifier, const char* aRemote, const char* aEndpoint, TEXTURES_RECEIVECALLBACK aCallback)
		{
			assert(s_TextureApi);
			s_TextureApi->Load(aIdentifier, aRemote, aEndpoint, aCallback, true);
		}

		void LoadFromMemory(const char* aIdentifier, void* aData, size_t aSize, TEXTURES_RECEIVECALLBACK aCallback)
		{
			assert(s_TextureApi);
			s_TextureApi->Load(aIdentifier, aData, aSize, aCallback, true);
		}
	}

	namespace Updater
	{
		void RequestUpdate(signed int aSignature, const char* aUpdateURL)
		{
			assert(s_Updater);

			if (!aSignature) { return; }
			if (!aUpdateURL) { return; }

			const std::lock_guard<std::mutex> lock(Loader::Mutex);

			Addon* addon = Loader::FindAddonBySig(aSignature);

			if (!addon) { return; }
			if (!addon->Definitions) { return; }
			if (addon->Definitions->Provider != EUpdateProvider::Self)
			{
				s_Logger->Warning(CH_UPDATER, "Update requested for %s but provider is not EUpdateProvider::Self. Cancelling.", addon->Definitions->Name);
				return;
			}

			/* TODO: Add verification */
			// 1. Addon -> API::RequestUpdate(signature);
			// 2. Nexus -> Events::RaiseSingle(signature, password123);
			// 3. Addon -> API::ConfirmUpdate(password123);

			std::filesystem::path path = addon->Path;

			AddonInfo addonInfo
			{
				addon->Definitions->Signature,
				addon->Definitions->Name,
				addon->Definitions->Version,
				addon->Definitions->Provider,
				aUpdateURL,
				addon->MD5
			};

			std::thread([path, addonInfo]()
			{
				s_Updater->UpdateAddon(path, addonInfo);
			}).detach();
		}
	}

	namespace UIRoot::GUI
	{
		void Register(ERenderType aRenderType, GUI_RENDER aRenderCallback)
		{
			assert(s_UiContext);
			s_UiContext->Register(aRenderType, aRenderCallback);
		}

		void Deregister(GUI_RENDER aRenderCallback)
		{
			assert(s_UiContext);
			s_UiContext->Deregister(aRenderCallback);
		}
	}

	namespace UIRoot::Fonts
	{
		void Get(const char* aIdentifier, FONTS_RECEIVECALLBACK aCallback)
		{
			assert(s_FontManager);

			if (!aCallback) { return; }

			ManagedFont* font = s_FontManager->Get(aIdentifier);

			if (font)
			{
				font->Subscribers.push_back(aCallback);
				aCallback(aIdentifier, font->Pointer);
			}
			else
			{
				aCallback(aIdentifier, nullptr);
			}
		}

		void Release(const char* aIdentifier, FONTS_RECEIVECALLBACK aCallback)
		{
			assert(s_FontManager);

			if (!aCallback) { return; }

			s_FontManager->Release(aIdentifier, aCallback);
		}

		void AddFontFromFile(const char* aIdentifier, float aFontSize, const char* aFilename, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig)
		{
			assert(s_FontManager);

			s_FontManager->AddFont(aIdentifier, aFontSize, aFilename, aCallback, aConfig);
		}

		void AddFontFromResource(const char* aIdentifier, float aFontSize, unsigned aResourceID, HMODULE aModule, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig)
		{
			assert(s_FontManager);

			s_FontManager->AddFont(aIdentifier, aFontSize, aResourceID, aModule, aCallback, aConfig);
		}

		void AddFontFromMemory(const char* aIdentifier, float aFontSize, void* aData, size_t aSize, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig)
		{
			assert(s_FontManager);

			s_FontManager->AddFont(aIdentifier, aFontSize, aData, aSize, aCallback, aConfig);
		}

		void ResizeFont(const char* aIdentifier, float aFontSize)
		{
			assert(s_FontManager);

			s_FontManager->ResizeFont(aIdentifier, aFontSize);
		}
	}

	namespace UIRoot::Alerts
	{
		void Notify(const char* aMessage)
		{
			assert(s_Alerts);
			s_Alerts->Notify(EAlertType::Info, aMessage);
		}
	}

	namespace UIRoot::QuickAccess
	{
		void AddShortcut(const char* aIdentifier, const char* aTextureIdentifier, const char* aTextureHoverIdentifier, const char* aInputBindIdentifier, const char* aTooltipText)
		{
			assert(s_QuickAccess);
			s_QuickAccess->AddShortcut(aIdentifier, aTextureIdentifier, aTextureHoverIdentifier, aInputBindIdentifier, aTooltipText);
		}

		void RemoveShortcut(const char* aIdentifier)
		{
			assert(s_QuickAccess);
			s_QuickAccess->RemoveShortcut(aIdentifier);
		}

		void NotifyShortcut(const char* aIdentifier)
		{
			assert(s_QuickAccess);
			s_QuickAccess->NotifyShortcut(aIdentifier);
		}

		void SetNotificationShortcut(const char* aIdentifier, bool aState)
		{
			assert(s_QuickAccess);
			s_QuickAccess->SetNotificationShortcut(aIdentifier, aState);
		}

		void AddContextItem(const char* aIdentifier, GUI_RENDER aShortcutRenderCallback)
		{
			assert(s_QuickAccess);
			s_QuickAccess->AddContextItem(aIdentifier, aShortcutRenderCallback);
		}

		void AddContextItem2(const char* aIdentifier, const char* aTargetShortcutIdentifier, GUI_RENDER aShortcutRenderCallback)
		{
			assert(s_QuickAccess);
			s_QuickAccess->AddContextItem(aIdentifier, aTargetShortcutIdentifier, aShortcutRenderCallback);
		}

		void RemoveContextItem(const char* aIdentifier)
		{
			assert(s_QuickAccess);
			s_QuickAccess->RemoveContextItem(aIdentifier);
		}
	}

	namespace UIRoot::EscapeClosing
	{
		void Register(const char* aWindowName, bool* aIsVisible)
		{
			assert(s_EscapeClosing);
			s_EscapeClosing->Register(aWindowName, aIsVisible);
		}

		void Deregister(const char* aWindowName)
		{
			assert(s_EscapeClosing);
			s_EscapeClosing->Deregister(aWindowName);
		}
	}

	AddonAPI* Get(int aVersion)
	{
		if (!s_IsInitialized)
		{
			CContext* ctx   = CContext::GetContext();

			s_DataLinkApi   = ctx->GetDataLink();
			s_EventApi      = ctx->GetEventApi();
			s_GameBindsApi  = ctx->GetGameBindsApi();
			s_InputBindApi  = ctx->GetInputBindApi();
			s_RawInputApi   = ctx->GetRawInputApi();
			s_Localization  = ctx->GetLocalization();
			s_Logger        = ctx->GetLogger();
			s_TextureApi    = ctx->GetTextureService();
			s_Updater       = ctx->GetUpdater();

			s_UiContext     = ctx->GetUIContext();
			s_FontManager   = s_UiContext->GetFontManager();
			s_Alerts        = s_UiContext->GetAlerts();
			s_QuickAccess   = s_UiContext->GetQuickAccess();
			s_EscapeClosing = s_UiContext->GetEscapeClosingService();

			s_IsInitialized = true;
		}

		assert(s_DataLinkApi);

		std::string dlName = "ADDONAPI_" + std::to_string(aVersion);
		AddonAPI* defs = (AddonAPI*)s_DataLinkApi->GetResource(dlName.c_str());

		// API defs with that version already exist, just return them
		if (defs)
		{
			return defs;
		}

		// create the requested version, add it to the map and return it
		switch (aVersion)
		{
			case 1:
			{
				AddonAPI1* api = (AddonAPI1*)s_DataLinkApi->ShareResource(dlName.c_str(), GetSize(aVersion));
				assert(api);

				api->SwapChain = Renderer::SwapChain;
				api->ImguiContext = ImGui::GetCurrentContext();
				api->ImguiMalloc = ImGui::MemAlloc;
				api->ImguiFree = ImGui::MemFree;
				api->RegisterRender = UIRoot::GUI::Register;
				api->DeregisterRender = UIRoot::GUI::Deregister;

				api->GetGameDirectory = Index::GetGameDirectory;
				api->GetAddonDirectory = Index::GetAddonDirectory;
				api->GetCommonDirectory = Index::GetCommonDirectory;

				api->CreateHook = MH_CreateHook;
				api->RemoveHook = MH_RemoveHook;
				api->EnableHook = MH_EnableHook;
				api->DisableHook = MH_DisableHook;

				api->Log = Logger::LogMessage;

				api->RaiseEvent = Events::RaiseEvent;
				api->SubscribeEvent = Events::Subscribe;
				api->UnsubscribeEvent = Events::Unsubscribe;

				api->RegisterWndProc = RawInput::Register;
				api->DeregisterWndProc = RawInput::Deregister;

				api->RegisterInputBindWithString = InputBinds::RegisterWithString;
				api->RegisterInputBindWithStruct = InputBinds::RegisterWithStruct;
				api->DeregisterInputBind = InputBinds::Deregister;

				api->GetResource = DataLink::GetResource;
				api->ShareResource = DataLink::ShareResource;

				api->GetTexture = TextureLoader::Get;
				api->LoadTextureFromFile = TextureLoader::LoadFromFile;
				api->LoadTextureFromResource = TextureLoader::LoadFromResource;
				api->LoadTextureFromURL = TextureLoader::LoadFromURL;

				api->AddShortcut = UIRoot::QuickAccess::AddShortcut;
				api->RemoveShortcut = UIRoot::QuickAccess::RemoveShortcut;
				api->AddSimpleShortcut = UIRoot::QuickAccess::AddContextItem;
				api->RemoveSimpleShortcut = UIRoot::QuickAccess::RemoveContextItem;

				defs = api;
				break;
			}
			case 2:
			{
				AddonAPI2* api = (AddonAPI2*)s_DataLinkApi->ShareResource(dlName.c_str(), GetSize(aVersion));
				assert(api);

				api->SwapChain = Renderer::SwapChain;
				api->ImguiContext = ImGui::GetCurrentContext();
				api->ImguiMalloc = ImGui::MemAlloc;
				api->ImguiFree = ImGui::MemFree;
				api->RegisterRender = UIRoot::GUI::Register;
				api->DeregisterRender = UIRoot::GUI::Deregister;

				api->GetGameDirectory = Index::GetGameDirectory;
				api->GetAddonDirectory = Index::GetAddonDirectory;
				api->GetCommonDirectory = Index::GetCommonDirectory;

				api->CreateHook = MH_CreateHook;
				api->RemoveHook = MH_RemoveHook;
				api->EnableHook = MH_EnableHook;
				api->DisableHook = MH_DisableHook;

				api->Log = Logger::LogMessage2;

				api->RaiseEvent = Events::RaiseEvent;
				api->RaiseEventNotification = Events::RaiseNotification;
				api->SubscribeEvent = Events::Subscribe;
				api->UnsubscribeEvent = Events::Unsubscribe;

				api->RegisterWndProc = RawInput::Register;
				api->DeregisterWndProc = RawInput::Deregister;
				api->SendWndProcToGameOnly = RawInput::SendWndProcToGame;

				api->RegisterInputBindWithString = InputBinds::RegisterWithString;
				api->RegisterInputBindWithStruct = InputBinds::RegisterWithStruct;
				api->DeregisterInputBind = InputBinds::Deregister;

				api->GetResource = DataLink::GetResource;
				api->ShareResource = DataLink::ShareResource;

				api->GetTexture = TextureLoader::Get;
				api->GetTextureOrCreateFromFile = TextureLoader::GetOrCreateFromFile;
				api->GetTextureOrCreateFromResource = TextureLoader::GetOrCreateFromResource;
				api->GetTextureOrCreateFromURL = TextureLoader::GetOrCreateFromURL;
				api->GetTextureOrCreateFromMemory = TextureLoader::GetOrCreateFromMemory;
				api->LoadTextureFromFile = TextureLoader::LoadFromFile;
				api->LoadTextureFromResource = TextureLoader::LoadFromResource;
				api->LoadTextureFromURL = TextureLoader::LoadFromURL;
				api->LoadTextureFromMemory = TextureLoader::LoadFromMemory;

				api->AddShortcut = UIRoot::QuickAccess::AddShortcut;
				api->RemoveShortcut = UIRoot::QuickAccess::RemoveShortcut;
				api->NotifyShortcut = UIRoot::QuickAccess::NotifyShortcut;
				api->AddSimpleShortcut = UIRoot::QuickAccess::AddContextItem;
				api->RemoveSimpleShortcut = UIRoot::QuickAccess::RemoveContextItem;

				api->Translate = Localization::Translate;
				api->TranslateTo = Localization::TranslateTo;

				defs = api;
				break;
			}
			case 3:
			{
				AddonAPI3* api = (AddonAPI3*)s_DataLinkApi->ShareResource(dlName.c_str(), GetSize(aVersion));
				assert(api);

				api->SwapChain = Renderer::SwapChain;
				api->ImguiContext = ImGui::GetCurrentContext();
				api->ImguiMalloc = ImGui::MemAlloc;
				api->ImguiFree = ImGui::MemFree;
				api->RegisterRender = UIRoot::GUI::Register;
				api->DeregisterRender = UIRoot::GUI::Deregister;

				api->GetGameDirectory = Index::GetGameDirectory;
				api->GetAddonDirectory = Index::GetAddonDirectory;
				api->GetCommonDirectory = Index::GetCommonDirectory;

				api->CreateHook = MH_CreateHook;
				api->RemoveHook = MH_RemoveHook;
				api->EnableHook = MH_EnableHook;
				api->DisableHook = MH_DisableHook;

				api->Log = Logger::LogMessage2;

				api->SendAlert = UIRoot::Alerts::Notify;

				api->RaiseEvent = Events::RaiseEvent;
				api->RaiseEventNotification = Events::RaiseNotification;
				api->RaiseEventTargeted = Events::RaiseEventTargeted;
				api->RaiseEventNotificationTargeted = Events::RaiseNotificationTargeted;
				api->SubscribeEvent = Events::Subscribe;
				api->UnsubscribeEvent = Events::Unsubscribe;

				api->RegisterWndProc = RawInput::Register;
				api->DeregisterWndProc = RawInput::Deregister;
				api->SendWndProcToGameOnly = RawInput::SendWndProcToGame;

				api->RegisterInputBindWithString = InputBinds::RegisterWithString;
				api->RegisterInputBindWithStruct = InputBinds::RegisterWithStruct;
				api->DeregisterInputBind = InputBinds::Deregister;

				api->GetResource = DataLink::GetResource;
				api->ShareResource = DataLink::ShareResource;

				api->GetTexture = TextureLoader::Get;
				api->GetTextureOrCreateFromFile = TextureLoader::GetOrCreateFromFile;
				api->GetTextureOrCreateFromResource = TextureLoader::GetOrCreateFromResource;
				api->GetTextureOrCreateFromURL = TextureLoader::GetOrCreateFromURL;
				api->GetTextureOrCreateFromMemory = TextureLoader::GetOrCreateFromMemory;
				api->LoadTextureFromFile = TextureLoader::LoadFromFile;
				api->LoadTextureFromResource = TextureLoader::LoadFromResource;
				api->LoadTextureFromURL = TextureLoader::LoadFromURL;
				api->LoadTextureFromMemory = TextureLoader::LoadFromMemory;

				api->AddShortcut = UIRoot::QuickAccess::AddShortcut;
				api->RemoveShortcut = UIRoot::QuickAccess::RemoveShortcut;
				api->NotifyShortcut = UIRoot::QuickAccess::NotifyShortcut;
				api->AddSimpleShortcut = UIRoot::QuickAccess::AddContextItem;
				api->RemoveSimpleShortcut = UIRoot::QuickAccess::RemoveContextItem;

				api->Translate = Localization::Translate;
				api->TranslateTo = Localization::TranslateTo;

				defs = api;
				break;
			}
			case 4:
			{
				AddonAPI4* api = (AddonAPI4*)s_DataLinkApi->ShareResource(dlName.c_str(), GetSize(aVersion));
				assert(api);

				api->SwapChain = Renderer::SwapChain;
				api->ImguiContext = ImGui::GetCurrentContext();
				api->ImguiMalloc = ImGui::MemAlloc;
				api->ImguiFree = ImGui::MemFree;
				api->RegisterRender = UIRoot::GUI::Register;
				api->DeregisterRender = UIRoot::GUI::Deregister;

				api->RequestUpdate = Updater::RequestUpdate;

				api->GetGameDirectory = Index::GetGameDirectory;
				api->GetAddonDirectory = Index::GetAddonDirectory;
				api->GetCommonDirectory = Index::GetCommonDirectory;

				api->CreateHook = MH_CreateHook;
				api->RemoveHook = MH_RemoveHook;
				api->EnableHook = MH_EnableHook;
				api->DisableHook = MH_DisableHook;

				api->Log = Logger::LogMessage2;

				api->SendAlert = UIRoot::Alerts::Notify;

				api->RaiseEvent = Events::RaiseEvent;
				api->RaiseEventNotification = Events::RaiseNotification;
				api->RaiseEventTargeted = Events::RaiseEventTargeted;
				api->RaiseEventNotificationTargeted = Events::RaiseNotificationTargeted;
				api->SubscribeEvent = Events::Subscribe;
				api->UnsubscribeEvent = Events::Unsubscribe;

				api->RegisterWndProc = RawInput::Register;
				api->DeregisterWndProc = RawInput::Deregister;
				api->SendWndProcToGameOnly = RawInput::SendWndProcToGame;

				api->RegisterInputBindWithString = InputBinds::RegisterWithString2;
				api->RegisterInputBindWithStruct = InputBinds::RegisterWithStruct2;
				api->DeregisterInputBind = InputBinds::Deregister;

				api->GetResource = DataLink::GetResource;
				api->ShareResource = DataLink::ShareResource;

				api->GetTexture = TextureLoader::Get;
				api->GetTextureOrCreateFromFile = TextureLoader::GetOrCreateFromFile;
				api->GetTextureOrCreateFromResource = TextureLoader::GetOrCreateFromResource;
				api->GetTextureOrCreateFromURL = TextureLoader::GetOrCreateFromURL;
				api->GetTextureOrCreateFromMemory = TextureLoader::GetOrCreateFromMemory;
				api->LoadTextureFromFile = TextureLoader::LoadFromFile;
				api->LoadTextureFromResource = TextureLoader::LoadFromResource;
				api->LoadTextureFromURL = TextureLoader::LoadFromURL;
				api->LoadTextureFromMemory = TextureLoader::LoadFromMemory;

				api->AddShortcut = UIRoot::QuickAccess::AddShortcut;
				api->RemoveShortcut = UIRoot::QuickAccess::RemoveShortcut;
				api->NotifyShortcut = UIRoot::QuickAccess::NotifyShortcut;
				api->AddSimpleShortcut = UIRoot::QuickAccess::AddContextItem;
				api->RemoveSimpleShortcut = UIRoot::QuickAccess::RemoveContextItem;

				api->Translate = Localization::Translate;
				api->TranslateTo = Localization::TranslateTo;

				api->GetFont = UIRoot::Fonts::Get;
				api->ReleaseFont = UIRoot::Fonts::Release;
				api->AddFontFromFile = UIRoot::Fonts::AddFontFromFile;
				api->AddFontFromResource = UIRoot::Fonts::AddFontFromResource;
				api->AddFontFromMemory = UIRoot::Fonts::AddFontFromMemory;

				defs = api;
				break;
			}
			case 5:
			{
				AddonAPI5* api = (AddonAPI5*)s_DataLinkApi->ShareResource(dlName.c_str(), GetSize(aVersion));
				assert(api);

				api->SwapChain = Renderer::SwapChain;
				api->ImguiContext = ImGui::GetCurrentContext();
				api->ImguiMalloc = ImGui::MemAlloc;
				api->ImguiFree = ImGui::MemFree;
				api->RegisterRender = UIRoot::GUI::Register;
				api->DeregisterRender = UIRoot::GUI::Deregister;

				api->RequestUpdate = Updater::RequestUpdate;

				api->GetGameDirectory = Index::GetGameDirectory;
				api->GetAddonDirectory = Index::GetAddonDirectory;
				api->GetCommonDirectory = Index::GetCommonDirectory;

				api->CreateHook = MH_CreateHook;
				api->RemoveHook = MH_RemoveHook;
				api->EnableHook = MH_EnableHook;
				api->DisableHook = MH_DisableHook;

				api->Log = Logger::LogMessage2;

				api->SendAlert = UIRoot::Alerts::Notify;

				api->RaiseEvent = Events::RaiseEvent;
				api->RaiseEventNotification = Events::RaiseNotification;
				api->RaiseEventTargeted = Events::RaiseEventTargeted;
				api->RaiseEventNotificationTargeted = Events::RaiseNotificationTargeted;
				api->SubscribeEvent = Events::Subscribe;
				api->UnsubscribeEvent = Events::Unsubscribe;

				api->RegisterWndProc = RawInput::Register;
				api->DeregisterWndProc = RawInput::Deregister;
				api->SendWndProcToGameOnly = RawInput::SendWndProcToGame;

				api->InvokeInputBind = InputBinds::InvokeInputBind;
				api->RegisterInputBindWithString = InputBinds::RegisterWithString2;
				api->RegisterInputBindWithStruct = InputBinds::RegisterWithStruct2;
				api->DeregisterInputBind = InputBinds::Deregister;

				api->GetResource = DataLink::GetResource;
				api->ShareResource = DataLink::ShareResource;

				api->GetTexture = TextureLoader::Get;
				api->GetTextureOrCreateFromFile = TextureLoader::GetOrCreateFromFile;
				api->GetTextureOrCreateFromResource = TextureLoader::GetOrCreateFromResource;
				api->GetTextureOrCreateFromURL = TextureLoader::GetOrCreateFromURL;
				api->GetTextureOrCreateFromMemory = TextureLoader::GetOrCreateFromMemory;
				api->LoadTextureFromFile = TextureLoader::LoadFromFile;
				api->LoadTextureFromResource = TextureLoader::LoadFromResource;
				api->LoadTextureFromURL = TextureLoader::LoadFromURL;
				api->LoadTextureFromMemory = TextureLoader::LoadFromMemory;

				api->AddShortcut = UIRoot::QuickAccess::AddShortcut;
				api->RemoveShortcut = UIRoot::QuickAccess::RemoveShortcut;
				api->NotifyShortcut = UIRoot::QuickAccess::NotifyShortcut;
				api->AddSimpleShortcut = UIRoot::QuickAccess::AddContextItem;
				api->RemoveSimpleShortcut = UIRoot::QuickAccess::RemoveContextItem;

				api->Translate = Localization::Translate;
				api->TranslateTo = Localization::TranslateTo;
				api->SetTranslatedString = Localization::Set;

				api->GetFont = UIRoot::Fonts::Get;
				api->ReleaseFont = UIRoot::Fonts::Release;
				api->AddFontFromFile = UIRoot::Fonts::AddFontFromFile;
				api->AddFontFromResource = UIRoot::Fonts::AddFontFromResource;
				api->AddFontFromMemory = UIRoot::Fonts::AddFontFromMemory;

				defs = api;
				break;
			}
			case 6:
			{
				AddonAPI6* api = (AddonAPI6*)s_DataLinkApi->ShareResource(dlName.c_str(), GetSize(aVersion));
				assert(api);

				api->SwapChain = Renderer::SwapChain;
				api->ImguiContext = ImGui::GetCurrentContext();
				api->ImguiMalloc = ImGui::MemAlloc;
				api->ImguiFree = ImGui::MemFree;

				api->Renderer.Register = UIRoot::GUI::Register;
				api->Renderer.Deregister = UIRoot::GUI::Deregister;

				api->RequestUpdate = Updater::RequestUpdate;

				api->Log = Logger::LogMessage2;

				api->UI.SendAlert = UIRoot::Alerts::Notify;
				api->UI.RegisterCloseOnEscape = UIRoot::EscapeClosing::Register;
				api->UI.DeregisterCloseOnEscape = UIRoot::EscapeClosing::Deregister;

				api->Paths.GetGameDirectory = Index::GetGameDirectory;
				api->Paths.GetAddonDirectory = Index::GetAddonDirectory;
				api->Paths.GetCommonDirectory = Index::GetCommonDirectory;

				api->MinHook.Create = MH_CreateHook;
				api->MinHook.Remove = MH_RemoveHook;
				api->MinHook.Enable = MH_EnableHook;
				api->MinHook.Disable = MH_DisableHook;

				api->Events.Raise = Events::RaiseEvent;
				api->Events.RaiseNotification = Events::RaiseNotification;
				api->Events.RaiseTargeted = Events::RaiseEventTargeted;
				api->Events.RaiseNotificationTargeted = Events::RaiseNotificationTargeted;
				api->Events.Subscribe = Events::Subscribe;
				api->Events.Unsubscribe = Events::Unsubscribe;

				api->WndProc.Register = RawInput::Register;
				api->WndProc.Deregister = RawInput::Deregister;
				api->WndProc.SendToGameOnly = RawInput::SendWndProcToGame;

				api->InputBinds.Invoke = InputBinds::InvokeInputBind;
				api->InputBinds.RegisterWithString = InputBinds::RegisterWithString2;
				api->InputBinds.RegisterWithStruct = InputBinds::RegisterWithStruct2;
				api->InputBinds.Deregister = InputBinds::Deregister;

				api->GameBinds.PressAsync = GameBinds::PressAsync;
				api->GameBinds.ReleaseAsync = GameBinds::ReleaseAsync;
				api->GameBinds.InvokeAsync = GameBinds::InvokeAsync;
				api->GameBinds.Press = GameBinds::Press;
				api->GameBinds.Release = GameBinds::Release;
				api->GameBinds.IsBound = GameBinds::IsBound;

				api->DataLink.Get = DataLink::GetResource;
				api->DataLink.Share = DataLink::ShareResource;

				api->Textures.Get = TextureLoader::Get;
				api->Textures.GetOrCreateFromFile = TextureLoader::GetOrCreateFromFile;
				api->Textures.GetOrCreateFromResource = TextureLoader::GetOrCreateFromResource;
				api->Textures.GetOrCreateFromURL = TextureLoader::GetOrCreateFromURL;
				api->Textures.GetOrCreateFromMemory = TextureLoader::GetOrCreateFromMemory;
				api->Textures.LoadFromFile = TextureLoader::LoadFromFile;
				api->Textures.LoadFromResource = TextureLoader::LoadFromResource;
				api->Textures.LoadFromURL = TextureLoader::LoadFromURL;
				api->Textures.LoadFromMemory = TextureLoader::LoadFromMemory;

				api->QuickAccess.Add = UIRoot::QuickAccess::AddShortcut;
				api->QuickAccess.Remove = UIRoot::QuickAccess::RemoveShortcut;
				api->QuickAccess.Notify = UIRoot::QuickAccess::NotifyShortcut;
				api->QuickAccess.AddContextMenu = UIRoot::QuickAccess::AddContextItem2;
				api->QuickAccess.RemoveContextMenu = UIRoot::QuickAccess::RemoveContextItem;

				api->Localization.Translate = Localization::Translate;
				api->Localization.TranslateTo = Localization::TranslateTo;
				api->Localization.SetTranslatedString = Localization::Set;

				api->Fonts.Get = UIRoot::Fonts::Get;
				api->Fonts.Release = UIRoot::Fonts::Release;
				api->Fonts.AddFromFile = UIRoot::Fonts::AddFontFromFile;
				api->Fonts.AddFromResource = UIRoot::Fonts::AddFontFromResource;
				api->Fonts.AddFromMemory = UIRoot::Fonts::AddFontFromMemory;
				api->Fonts.Resize = UIRoot::Fonts::ResizeFont;

				defs = api;
				break;
			}
		}

		return defs;
	}

	size_t GetSize(int aVersion)
	{
		switch (aVersion)
		{
			case 1:
				return sizeof(AddonAPI1);
			case 2:
				return sizeof(AddonAPI2);
			case 3:
				return sizeof(AddonAPI3);
			case 4:
				return sizeof(AddonAPI4);
			case 5:
				return sizeof(AddonAPI5);
			case 6:
				return sizeof(AddonAPI6);
		}

		return 0;
	}
}
