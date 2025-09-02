///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AdoApiV3.h
/// Description  :  Addon API Revision 3.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef ADOAPIV3_H
#define ADOAPIV3_H

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
#include "Engine/Textures/TxFuncDefs.h"
#include "UI/FuncDefs.h"
#include "UI/Services/Localization/LoclFuncDefs.h"
#include "UI/Widgets/Alerts/AlFuncDefs.h"
#include "UI/Widgets/QuickAccess/QaFuncDefs.h"

///----------------------------------------------------------------------------------------------------
/// AddonAPI3_t Struct
///----------------------------------------------------------------------------------------------------
struct AddonAPI3_t : AddonAPI_t
{
	/* Renderer */
	IDXGISwapChain*                   SwapChain;
	ImGuiContext*                     ImguiContext;
	void*                             ImguiMalloc;
	void*                             ImguiFree;
	GUI_ADDRENDER                     RegisterRender;
	GUI_REMRENDER                     DeregisterRender;

	/* Paths */
	IDX_GETGAMEDIR                    GetGameDirectory;
	IDX_GETADDONDIR                   GetAddonDirectory;
	IDX_GETCOMMONDIR                  GetCommonDirectory;

	/* Minhook */
	MINHOOK_CREATE                    CreateHook;
	MINHOOK_REMOVE                    RemoveHook;
	MINHOOK_ENABLE                    EnableHook;
	MINHOOK_DISABLE                   DisableHook;

	/* Logging */
	LOGGER_LOG2                       Log;

	/* GUI Alerts */
	ALERTS_NOTIFY                     SendAlert;

	/* Events */
	EVENTS_RAISE                      RaiseEvent;
	EVENTS_RAISENOTIFICATION          RaiseEventNotification;
	EVENTS_RAISE_TARGETED             RaiseEventTargeted;
	EVENTS_RAISENOTIFICATION_TARGETED RaiseEventNotificationTargeted;
	EVENTS_SUBSCRIBE                  SubscribeEvent;
	EVENTS_SUBSCRIBE                  UnsubscribeEvent;

	/* WndProc */
	WNDPROC_ADDREM                    RegisterWndProc;
	WNDPROC_ADDREM                    DeregisterWndProc;
	WNDPROC_SENDTOGAME                SendWndProcToGameOnly;

	/* InputBinds */
	INPUTBINDS_REGISTERWITHSTRING     RegisterInputBindWithString;
	INPUTBINDS_REGISTERWITHSTRUCT     RegisterInputBindWithStruct;
	INPUTBINDS_DEREGISTER             DeregisterInputBind;

	/* DataLink */
	DATALINK_GETRESOURCE              GetResource;
	DATALINK_SHARERESOURCE            ShareResource;

	/* Textures */
	TEXTURES_GET                      GetTexture;
	TEXTURES_GETORCREATEFROMFILE      GetTextureOrCreateFromFile;
	TEXTURES_GETORCREATEFROMRESOURCE  GetTextureOrCreateFromResource;
	TEXTURES_GETORCREATEFROMURL       GetTextureOrCreateFromURL;
	TEXTURES_GETORCREATEFROMMEMORY    GetTextureOrCreateFromMemory;
	TEXTURES_LOADFROMFILE             LoadTextureFromFile;
	TEXTURES_LOADFROMRESOURCE         LoadTextureFromResource;
	TEXTURES_LOADFROMURL              LoadTextureFromURL;
	TEXTURES_LOADFROMMEMORY           LoadTextureFromMemory;

	/* Shortcuts */
	QUICKACCESS_ADDSHORTCUT           AddShortcut;
	QUICKACCESS_GENERIC               RemoveShortcut;
	QUICKACCESS_GENERIC               NotifyShortcut;
	QUICKACCESS_ADDSIMPLE             AddSimpleShortcut;
	QUICKACCESS_GENERIC               RemoveSimpleShortcut;

	/* Localization */
	LOCALIZATION_TRANSLATE            Translate;
	LOCALIZATION_TRANSLATETO          TranslateTo;
};


#endif
