///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AdoApiV6.h
/// Description  :  Addon API Revision 6.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "ApiBase.h"

#include <dxgi.h>

#include "thirdparty/imgui/imgui.h"
#include "thirdparty/minhook/mh_hook.h"

#include "Core/Index/IdxFuncDefs.h"
#include "Core/DataLink/DlFuncDefs.h"
#include "Host/Events/EvtFuncDefs.h"
#include "Engine/Inputs/InputBinds/IbFuncDefs.h"
#include "Core/Logging/LogFuncDefs.h"
#include "GW2/Inputs/GameBinds/GbFuncDefs.h"
#include "Platform/RawInput/RiFuncDefs.h"
#include "UI/Services/Fonts/FuncDefs.h"
#include "UI/Services/Localization/LoclFuncDefs.h"
#include "UI/Services/QoL/FuncDefs.h"
#include "Graphics/Textures/TxFuncDefs.h"
#include "UI/UiFuncDefs.h"
#include "UI/Widgets/Alerts/AlFuncDefs.h"
#include "UI/Widgets/QuickAccess/QaFuncDefs.h"

using namespace Raidcore::Nexus;

///----------------------------------------------------------------------------------------------------
/// AddonAPI6_t Struct
///----------------------------------------------------------------------------------------------------
struct AddonAPI6_t : AddonAPI_t
{
	/* Renderer */
	IDXGISwapChain*                       SwapChain;
	ImGuiContext*                         ImguiContext;
	void*                                 ImguiMalloc;
	void*                                 ImguiFree;

	struct RendererVT
	{
		GUI::GUI_ADDRENDER                     Register;
		GUI::GUI_REMRENDER                     Deregister;
	};
	RendererVT                            Renderer;

	/* Updater */
	UPDATER_REQUESTUPDATE                 RequestUpdate;

	/* Logging */
	Core::LOGGER_LOG2                           Log;

	/* User Interface */
	struct UIVT
	{
		GUI::ALERTS_NOTIFY                     SendAlert;
		GUI::GUI_REGISTERCLOSEONESCAPE         RegisterCloseOnEscape;
		GUI::GUI_DEREGISTERCLOSEONESCAPE       DeregisterCloseOnEscape;
	};
	UIVT                                  UI;

	/* Paths */
	struct PathsVT
	{
		IDX_GETGAMEDIR                    GetGameDirectory;
		IDX_GETADDONDIR                   GetAddonDirectory;
		IDX_GETCOMMONDIR                  GetCommonDirectory;
	};
	PathsVT                               Paths;

	/* Minhook */
	struct MinHookVT
	{
		MINHOOK_CREATE                    Create;
		MINHOOK_REMOVE                    Remove;
		MINHOOK_ENABLE                    Enable;
		MINHOOK_DISABLE                   Disable;
	};
	MinHookVT                             MinHook;

	/* Events */
	struct EventsVT
	{
		Host::EVENTS_RAISE                      Raise;
		Host::EVENTS_RAISENOTIFICATION          RaiseNotification;
		Host::EVENTS_RAISE_TARGETED             RaiseTargeted;
		Host::EVENTS_RAISENOTIFICATION_TARGETED RaiseNotificationTargeted;
		Host::EVENTS_SUBSCRIBE                  Subscribe;
		Host::EVENTS_SUBSCRIBE                  Unsubscribe;
	};
	EventsVT                              Events;

	/* WndProc */
	struct WndProcVT
	{
		Platform::WNDPROC_ADDREM          Register;
		Platform::WNDPROC_ADDREM          Deregister;
		Platform::WNDPROC_SENDTOGAME      SendToGameOnly;
	};
	WndProcVT                             WndProc;

	/* InputBinds */
	struct InputBindsVT
	{
		INPUTBINDS_INVOKE                 Invoke;
		INPUTBINDS_REGISTERWITHSTRING2    RegisterWithString;
		INPUTBINDS_REGISTERWITHSTRUCT2    RegisterWithStruct;
		INPUTBINDS_DEREGISTER             Deregister;
	};
	InputBindsVT                          InputBinds;

	/* GameBinds */
	struct GameBindsVT
	{
		GW2::GAMEBINDS_PRESSASYNC              PressAsync;
		GW2::GAMEBINDS_RELEASEASYNC            ReleaseAsync;
		GW2::GAMEBINDS_INVOKEASYNC             InvokeAsync;
		GW2::GAMEBINDS_PRESS                   Press;
		GW2::GAMEBINDS_RELEASE                 Release;
		GW2::GAMEBINDS_ISBOUND                 IsBound;
	};
	GameBindsVT                           GameBinds;

	/* DataLink */
	struct DataLinkVT
	{
		Core::DATALINK_GETRESOURCE              Get;
		Core::DATALINK_SHARERESOURCE            Share;
	};
	DataLinkVT                            DataLink;

	/* Textures */
	struct TexturesVT
	{
		Graphics::TEXTURES_GET                      Get;
		Graphics::TEXTURES_GETORCREATEFROMFILE      GetOrCreateFromFile;
		Graphics::TEXTURES_GETORCREATEFROMRESOURCE  GetOrCreateFromResource;
		Graphics::TEXTURES_GETORCREATEFROMURL       GetOrCreateFromURL;
		Graphics::TEXTURES_GETORCREATEFROMMEMORY    GetOrCreateFromMemory;
		Graphics::TEXTURES_LOADFROMFILE             LoadFromFile;
		Graphics::TEXTURES_LOADFROMRESOURCE         LoadFromResource;
		Graphics::TEXTURES_LOADFROMURL              LoadFromURL;
		Graphics::TEXTURES_LOADFROMMEMORY           LoadFromMemory;
	};
	TexturesVT                            Textures;

	/* Shortcuts */
	struct QuickAccessVT
	{
		GUI::QUICKACCESS_ADDSHORTCUT           Add;
		GUI::QUICKACCESS_GENERIC               Remove;
		GUI::QUICKACCESS_GENERIC               Notify;
		GUI::QUICKACCESS_ADDSIMPLE2            AddContextMenu;
		GUI::QUICKACCESS_GENERIC               RemoveContextMenu;
	};
	QuickAccessVT                         QuickAccess;

	/* Localization */
	struct LocalizationVT
	{
		GUI::LOCALIZATION_TRANSLATE            Translate;
		GUI::LOCALIZATION_TRANSLATETO          TranslateTo;
		GUI::LOCALIZATION_SET                  SetTranslatedString;
	 };
	LocalizationVT                        Localization;

	/* Fonts */
	struct FontsVT
	{
		GUI::FONTS_GETRELEASE                  Get;
		GUI::FONTS_GETRELEASE                  Release;
		GUI::FONTS_ADDFROMFILE                 AddFromFile;
		GUI::FONTS_ADDFROMRESOURCE             AddFromResource;
		GUI::FONTS_ADDFROMMEMORY               AddFromMemory;
		GUI::FONTS_RESIZE                      Resize;
	};
	FontsVT                               Fonts;
};
