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
#include "Engine/DataLink/DlFuncDefs.h"
#include "Engine/Events/EvtFuncDefs.h"
#include "Engine/Inputs/InputBinds/IbFuncDefs.h"
#include "Engine/Inputs/RawInput/RiFuncDefs.h"
#include "Engine/Logging/LogFuncDefs.h"
#include "GW2/Inputs/GameBinds/GbFuncDefs.h"
#include "UI/Services/Fonts/FuncDefs.h"
#include "UI/Services/Localization/LoclFuncDefs.h"
#include "UI/Services/QoL/FuncDefs.h"
#include "UI/Textures/TxFuncDefs.h"
#include "UI/UiFuncDefs.h"
#include "UI/Widgets/Alerts/AlFuncDefs.h"
#include "UI/Widgets/QuickAccess/QaFuncDefs.h"

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
		GUI_ADDRENDER                     Register;
		GUI_REMRENDER                     Deregister;
	};
	RendererVT                            Renderer;

	/* Updater */
	UPDATER_REQUESTUPDATE                 RequestUpdate;

	/* Logging */
	LOGGER_LOG2                           Log;

	/* User Interface */
	struct UIVT
	{
		ALERTS_NOTIFY                     SendAlert;
		GUI_REGISTERCLOSEONESCAPE         RegisterCloseOnEscape;
		GUI_DEREGISTERCLOSEONESCAPE       DeregisterCloseOnEscape;
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
		EVENTS_RAISE                      Raise;
		EVENTS_RAISENOTIFICATION          RaiseNotification;
		EVENTS_RAISE_TARGETED             RaiseTargeted;
		EVENTS_RAISENOTIFICATION_TARGETED RaiseNotificationTargeted;
		EVENTS_SUBSCRIBE                  Subscribe;
		EVENTS_SUBSCRIBE                  Unsubscribe;
	};
	EventsVT                              Events;

	/* WndProc */
	struct WndProcVT
	{
		WNDPROC_ADDREM                    Register;
		WNDPROC_ADDREM                    Deregister;
		WNDPROC_SENDTOGAME                SendToGameOnly;
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
		GAMEBINDS_PRESSASYNC              PressAsync;
		GAMEBINDS_RELEASEASYNC            ReleaseAsync;
		GAMEBINDS_INVOKEASYNC             InvokeAsync;
		GAMEBINDS_PRESS                   Press;
		GAMEBINDS_RELEASE                 Release;
		GAMEBINDS_ISBOUND                 IsBound;
	};
	GameBindsVT                           GameBinds;

	/* DataLink */
	struct DataLinkVT
	{
		DATALINK_GETRESOURCE              Get;
		DATALINK_SHARERESOURCE            Share;
	};
	DataLinkVT                            DataLink;

	/* Textures */
	struct TexturesVT
	{
		TEXTURES_GET                      Get;
		TEXTURES_GETORCREATEFROMFILE      GetOrCreateFromFile;
		TEXTURES_GETORCREATEFROMRESOURCE  GetOrCreateFromResource;
		TEXTURES_GETORCREATEFROMURL       GetOrCreateFromURL;
		TEXTURES_GETORCREATEFROMMEMORY    GetOrCreateFromMemory;
		TEXTURES_LOADFROMFILE             LoadFromFile;
		TEXTURES_LOADFROMRESOURCE         LoadFromResource;
		TEXTURES_LOADFROMURL              LoadFromURL;
		TEXTURES_LOADFROMMEMORY           LoadFromMemory;
	};
	TexturesVT                            Textures;

	/* Shortcuts */
	struct QuickAccessVT
	{
		QUICKACCESS_ADDSHORTCUT           Add;
		QUICKACCESS_GENERIC               Remove;
		QUICKACCESS_GENERIC               Notify;
		QUICKACCESS_ADDSIMPLE2            AddContextMenu;
		QUICKACCESS_GENERIC               RemoveContextMenu;
	};
	QuickAccessVT                         QuickAccess;

	/* Localization */
	struct LocalizationVT
	{
		LOCALIZATION_TRANSLATE            Translate;
		LOCALIZATION_TRANSLATETO          TranslateTo;
		LOCALIZATION_SET                  SetTranslatedString;
	};
	LocalizationVT                        Localization;

	/* Fonts */
	struct FontsVT
	{
		FONTS_GETRELEASE                  Get;
		FONTS_GETRELEASE                  Release;
		FONTS_ADDFROMFILE                 AddFromFile;
		FONTS_ADDFROMRESOURCE             AddFromResource;
		FONTS_ADDFROMMEMORY               AddFromMemory;
		FONTS_RESIZE                      Resize;
	};
	FontsVT                               Fonts;
};
