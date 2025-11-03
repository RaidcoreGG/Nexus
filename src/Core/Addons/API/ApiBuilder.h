///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  ApiBuilds.h
/// Description  :  Contains the logic to map engine functions to the exposed API definitions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <windows.h>

#include "ApiBase.h"
#include "Engine/Events/EvtFuncDefs.h"
#include "Engine/Inputs/InputBinds/IbFuncDefs.h"
#include "Engine/Inputs/RawInput/RiFuncDefs.h"
#include "Engine/Logging/LogEnum.h"
#include "Engine/Textures/TxFuncDefs.h"
#include "GW2/Inputs/GameBinds/GbEnum.h"
#include "UI/FuncDefs.h"
#include "UI/Services/Fonts/FuncDefs.h"

///----------------------------------------------------------------------------------------------------
/// ADDONAPI Namespace
///----------------------------------------------------------------------------------------------------
namespace ADDONAPI
{
	///----------------------------------------------------------------------------------------------------
	/// DataLink Namespace
	///----------------------------------------------------------------------------------------------------
	namespace DataLink
	{
		///----------------------------------------------------------------------------------------------------
		/// GetResource:
		/// 	Addon API wrapper function for GetResource.
		///----------------------------------------------------------------------------------------------------
		void* GetResource(const char* aIdentifier);

		///----------------------------------------------------------------------------------------------------
		/// ShareResource:
		/// 	Addon API wrapper function for ShareResource.
		///----------------------------------------------------------------------------------------------------
		void* ShareResource(const char* aIdentifier, size_t aResourceSize);
	}

	///----------------------------------------------------------------------------------------------------
	/// Events Namespace
	///----------------------------------------------------------------------------------------------------
	namespace Events
	{
		///----------------------------------------------------------------------------------------------------
		/// Subscribe:
		/// 	Addon API wrapper function for subscribing to events.
		///----------------------------------------------------------------------------------------------------
		void Subscribe(const char* aIdentifier, EVENT_CONSUME aConsumeEventCallback);

		///----------------------------------------------------------------------------------------------------
		/// Unsubscribe:
		/// 	Addon API wrapper function for unsubscribing from events.
		///----------------------------------------------------------------------------------------------------
		void Unsubscribe(const char* aIdentifier, EVENT_CONSUME aConsumeEventCallback);

		///----------------------------------------------------------------------------------------------------
		/// RaiseEvent:
		/// 	Addon API wrapper function for raising events.
		///----------------------------------------------------------------------------------------------------
		void RaiseEvent(const char* aIdentifier, void* aEventData);

		///----------------------------------------------------------------------------------------------------
		/// RaiseNotification:
		/// 	Addon API wrapper function for raising events without payloads.
		///----------------------------------------------------------------------------------------------------
		void RaiseNotification(const char* aIdentifier);

		///----------------------------------------------------------------------------------------------------
		/// RaiseEventTargeted:
		/// 	Addon API wrapper function for raising events targeted at a specific subscriber.
		///----------------------------------------------------------------------------------------------------
		void RaiseEventTargeted(uint32_t aSignature, const char* aIdentifier, void* aEventData);

		///----------------------------------------------------------------------------------------------------
		/// RaiseNotificationTargeted:
		/// 	Addon API wrapper function for raising notifications targeted at a specific subscriber.
		///----------------------------------------------------------------------------------------------------
		void RaiseNotificationTargeted(uint32_t aSignature, const char* aIdentifier);
	}

	///----------------------------------------------------------------------------------------------------
	/// GameBinds Namespace
	///----------------------------------------------------------------------------------------------------
	namespace GameBinds
	{
		///----------------------------------------------------------------------------------------------------
		/// PressAsync:
		/// 	Presses the keys of a game bind.
		///----------------------------------------------------------------------------------------------------
		void PressAsync(EGameBinds aGameBind);

		///----------------------------------------------------------------------------------------------------
		/// ReleaseAsync:
		/// 	Releases the keys of a game bind.
		///----------------------------------------------------------------------------------------------------
		void ReleaseAsync(EGameBinds aGameBind);

		///----------------------------------------------------------------------------------------------------
		/// InvokeAsync:
		/// 	Presses and releases the keys of a game bind.
		/// 	aDuration is the wait time in milliseconds between press and release.
		///----------------------------------------------------------------------------------------------------
		void InvokeAsync(EGameBinds aGameBind, int aDuration);

		///----------------------------------------------------------------------------------------------------
		/// Press:
		/// 	Presses the keys of a game bind.
		///----------------------------------------------------------------------------------------------------
		void Press(EGameBinds aGameBind);

		///----------------------------------------------------------------------------------------------------
		/// Release:
		/// 	Releases the keys of a game bind.
		///----------------------------------------------------------------------------------------------------
		void Release(EGameBinds aGameBind);

		///----------------------------------------------------------------------------------------------------
		/// IsBound:
		/// 	Returns whether a game bind has a InputBind_t set or not.
		///----------------------------------------------------------------------------------------------------
		bool IsBound(EGameBinds aGameBind);
	}

	///----------------------------------------------------------------------------------------------------
	/// Paths Namespace
	///----------------------------------------------------------------------------------------------------
	namespace Paths
	{
		///----------------------------------------------------------------------------------------------------
		/// GetGameDirectory:
		/// 	Returns the game directory.
		///----------------------------------------------------------------------------------------------------
		const char* GetGameDirectory();

		///----------------------------------------------------------------------------------------------------
		/// GetAddonDirectory:
		/// 	Returns "<game>/addons/<aName>" or "<game>/addons" if nullptr is passed.
		///----------------------------------------------------------------------------------------------------
		const char* GetAddonDirectory(const char* aName);

		///----------------------------------------------------------------------------------------------------
		/// GetCommonDirectory:
		/// 	Returns the "<game>/addons/common" directory.
		///----------------------------------------------------------------------------------------------------
		const char* GetCommonDirectory();
	}

	///----------------------------------------------------------------------------------------------------
	/// InputBinds Namespace
	///----------------------------------------------------------------------------------------------------
	namespace InputBinds
	{
		///----------------------------------------------------------------------------------------------------
		/// RegisterWithString:
		/// 	[Revision 1] Addon API wrapper function for Register from string.
		///----------------------------------------------------------------------------------------------------
		void RegisterWithString(const char* aIdentifier, INPUTBINDS_PROCESS aInputBindHandler, const char* aInputBind);

		///----------------------------------------------------------------------------------------------------
		/// RegisterWithStruct:
		/// 	[Revision 1] Addon API wrapper function for Register from struct.
		///----------------------------------------------------------------------------------------------------
		void RegisterWithStruct(const char* aIdentifier, INPUTBINDS_PROCESS aInputBindHandler, InputBindV1_t aInputBind);

		///----------------------------------------------------------------------------------------------------
		/// RegisterWithString:
		/// 	[Revision 2] Addon API wrapper function for Register from string.
		///----------------------------------------------------------------------------------------------------
		void RegisterWithString2(const char* aIdentifier, INPUTBINDS_PROCESS2 aInputBindHandler, const char* aInputBind);

		///----------------------------------------------------------------------------------------------------
		/// RegisterWithStruct:
		/// 	[Revision 2] Addon API wrapper function for Register from struct.
		///----------------------------------------------------------------------------------------------------
		void RegisterWithStruct2(const char* aIdentifier, INPUTBINDS_PROCESS2 aInputBindHandler, InputBindV1_t aInputBind);

		///----------------------------------------------------------------------------------------------------
		/// InvokeInputBind:
		/// 	Addon API wrapper function for Invoke.
		///----------------------------------------------------------------------------------------------------
		void InvokeInputBind(const char* aIdentifier, bool aIsRelease);

		///----------------------------------------------------------------------------------------------------
		/// Deregister:
		/// 	Addon API wrapper function for Deregister.
		///----------------------------------------------------------------------------------------------------
		void Deregister(const char* aIdentifier);
	}

	///----------------------------------------------------------------------------------------------------
	/// RawInput Namespace
	///----------------------------------------------------------------------------------------------------
	namespace RawInput
	{
		///----------------------------------------------------------------------------------------------------
		/// SendWndProcToGame:
		/// 	Skips all WndProc callbacks and sends it directly to the original.
		///----------------------------------------------------------------------------------------------------
		LRESULT SendWndProcToGame(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		///----------------------------------------------------------------------------------------------------
		/// Register:
		/// 	Registers the provided WndProcCallback.
		///----------------------------------------------------------------------------------------------------
		void Register(WNDPROC_CALLBACK aWndProcCallback);

		///----------------------------------------------------------------------------------------------------
		/// Deregister:
		/// 	Deregisters the provided WndProcCallback.
		///----------------------------------------------------------------------------------------------------
		void Deregister(WNDPROC_CALLBACK aWndProcCallback);
	}

	///----------------------------------------------------------------------------------------------------
	/// Localization Namespace
	///----------------------------------------------------------------------------------------------------
	namespace Localization
	{
		///----------------------------------------------------------------------------------------------------
		/// Translate:
		/// 	Addon API wrapper function for Translate. Translates into the currently set language.
		///----------------------------------------------------------------------------------------------------
		const char* Translate(const char* aIdentifier);

		///----------------------------------------------------------------------------------------------------
		/// TranslateTo:
		/// 	Addon API wrapper function for Translate. Translates into a specific language.
		///----------------------------------------------------------------------------------------------------
		const char* TranslateTo(const char* aIdentifier, const char* aLanguageIdentifier);

		///----------------------------------------------------------------------------------------------------
		/// Set:
		/// 	Addon API wrapper function to set translated strings.
		///----------------------------------------------------------------------------------------------------
		void Set(const char* aIdentifier, const char* aLanguageIdentifier, const char* aString);
	}

	///----------------------------------------------------------------------------------------------------
	/// Logger Namespace
	///----------------------------------------------------------------------------------------------------
	namespace Logger
	{
		///----------------------------------------------------------------------------------------------------
		/// LogMessage:
		/// 	[Revision 1] Logs a message.
		///----------------------------------------------------------------------------------------------------
		void LogMessage(ELogLevel aLogLevel, const char* aStr);

		///----------------------------------------------------------------------------------------------------
		/// LogMessage2:
		/// 	[Revision 2] Logs a message with a custom channel.
		///----------------------------------------------------------------------------------------------------
		void LogMessage2(ELogLevel aLogLevel, const char* aChannel, const char* aStr);
	}

	///----------------------------------------------------------------------------------------------------
	/// TextureLoader Namespace
	///----------------------------------------------------------------------------------------------------
	namespace TextureLoader
	{
		///----------------------------------------------------------------------------------------------------
		/// Get:
		/// 	Addon API wrapper function for Get.
		///----------------------------------------------------------------------------------------------------
		Texture_t* Get(const char* aIdentifier);

		///----------------------------------------------------------------------------------------------------
		/// GetOrCreateFromFile:
		/// 	Addon API wrapper function for GetOrCreate from file.
		///----------------------------------------------------------------------------------------------------
		Texture_t* GetOrCreateFromFile(const char* aIdentifier, const char* aFilename);

		///----------------------------------------------------------------------------------------------------
		/// GetOrCreateFromResource:
		/// 	Addon API wrapper function for GetOrCreate from embedded resource.
		///----------------------------------------------------------------------------------------------------
		Texture_t* GetOrCreateFromResource(const char* aIdentifier, unsigned aResourceID, HMODULE aModule);

		///----------------------------------------------------------------------------------------------------
		/// GetOrCreateFromURL:
		/// 	Addon API wrapper function for GetOrCreate from remote URL.
		///----------------------------------------------------------------------------------------------------
		Texture_t* GetOrCreateFromURL(const char* aIdentifier, const char* aRemote, const char* aEndpoint);

		///----------------------------------------------------------------------------------------------------
		/// GetOrCreateFromMemory:
		/// 	Addon API wrapper function for GetOrCreate from memory.
		///----------------------------------------------------------------------------------------------------
		Texture_t* GetOrCreateFromMemory(const char* aIdentifier, void* aData, size_t aSize);

		///----------------------------------------------------------------------------------------------------
		/// LoadFromFile:
		/// 	Addon API wrapper function for LoadFromFile.
		///----------------------------------------------------------------------------------------------------
		void LoadFromFile(const char* aIdentifier, const char* aFilename, TEXTURES_RECEIVECALLBACK aCallback);

		///----------------------------------------------------------------------------------------------------
		/// LoadFromResource:
		/// 	Addon API wrapper function for LoadFromResource.
		///----------------------------------------------------------------------------------------------------
		void LoadFromResource(const char* aIdentifier, unsigned aResourceID, HMODULE aModule, TEXTURES_RECEIVECALLBACK aCallback);

		///----------------------------------------------------------------------------------------------------
		/// LoadFromURL:
		/// 	Addon API wrapper function for LoadFromURL.
		///----------------------------------------------------------------------------------------------------
		void LoadFromURL(const char* aIdentifier, const char* aRemote, const char* aEndpoint, TEXTURES_RECEIVECALLBACK aCallback);

		///----------------------------------------------------------------------------------------------------
		/// LoadFromMemory:
		/// 	Addon API wrapper function for LoadFromMemory.
		///----------------------------------------------------------------------------------------------------
		void LoadFromMemory(const char* aIdentifier, void* aData, size_t aSize, TEXTURES_RECEIVECALLBACK aCallback);
	}

	///----------------------------------------------------------------------------------------------------
	/// Updater Namespace
	///----------------------------------------------------------------------------------------------------
	namespace Updater
	{
		///----------------------------------------------------------------------------------------------------
		/// RequestUpdate:
		/// 	Addon API wrapper to self update.
		///----------------------------------------------------------------------------------------------------
		void RequestUpdate(signed int aSignature, const char* aUpdateURL);
	}

	namespace UIRoot::GUI
	{
		void Register(ERenderType aRenderType, GUI_RENDER aRenderCallback);

		void Deregister(GUI_RENDER aRenderCallback);
	}

	namespace UIRoot::Fonts
	{
		void Get(const char* aIdentifier, FONTS_RECEIVECALLBACK aCallback);

		void Release(const char* aIdentifier, FONTS_RECEIVECALLBACK aCallback);

		void AddFontFromFile(const char* aIdentifier, float aFontSize, const char* aFilename, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig);

		void AddFontFromResource(const char* aIdentifier, float aFontSize, unsigned aResourceID, HMODULE aModule, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig);

		void AddFontFromMemory(const char* aIdentifier, float aFontSize, void* aData, size_t aSize, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig);

		void ResizeFont(const char* aIdentifier, float aFontSize);
	}

	namespace UIRoot::Alerts
	{
		void Notify(const char* aMessage);
	}

	namespace UIRoot::QuickAccess
	{
		void AddShortcut(const char* aIdentifier, const char* aTextureIdentifier, const char* aTextureHoverIdentifier, const char* aInputBindIdentifier, const char* aTooltipText);

		void RemoveShortcut(const char* aIdentifier);

		void PushNotification(const char* aIdentifier);

		void SetNotificationShortcut(const char* aIdentifier, bool aState);

		void AddContextItem(const char* aIdentifier, GUI_RENDER aShortcutRenderCallback);

		void AddContextItem2(const char* aIdentifier, const char* aTargetShortcutIdentifier, GUI_RENDER aShortcutRenderCallback);

		void RemoveContextItem(const char* aIdentifier);
	}
	namespace UIRoot::EscapeClosing
	{
		void Register(const char* aWindowName, bool* aIsVisible);

		void Deregister(const char* aWindowName);
	}

	///----------------------------------------------------------------------------------------------------
	/// Get:
	/// 	Gets or creates a pointer to the provided version, or nullptr if no such version exists.
	///----------------------------------------------------------------------------------------------------
	AddonAPI_t* Get(uint32_t aVersion, bool aSetImGuiContext = true);

	///----------------------------------------------------------------------------------------------------
	/// GetSize:
	/// 	Returns the size of the provided API version.
	///----------------------------------------------------------------------------------------------------
	size_t GetSize(uint32_t aVersion);
}
