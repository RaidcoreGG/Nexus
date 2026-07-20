///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AdoApiV5.h
/// Description  :  Addon API Revision 5.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "ApiBase.h"

#include <dxgi.h>

#include "thirdparty/imgui/imgui.h"
#include "thirdparty/minhook/mh_hook.h"

#include "Index/IdxFuncDefs.h"
#include "Core/DataLink/DlFuncDefs.h"
#include "Host/Events/EvtFuncDefs.h"
#include "Inputs/InputBinds/IbFuncDefs.h"
#include "Core/Logging/LogFuncDefs.h"
#include "Platform/RawInput/RiFuncDefs.h"
#include "UI/Services/Fonts/FuncDefs.h"
#include "UI/Services/Localization/LoclFuncDefs.h"
#include "Graphics/Textures/TxFuncDefs.h"
#include "UI/UiFuncDefs.h"
#include "UI/Widgets/Alerts/AlFuncDefs.h"
#include "UI/Widgets/QuickAccess/QaFuncDefs.h"

using namespace Raidcore::Nexus;

///----------------------------------------------------------------------------------------------------
/// AddonAPI5_t Struct
///----------------------------------------------------------------------------------------------------
struct AddonAPI5_t : AddonAPI_t
{
	/* Renderer */
	IDXGISwapChain*                   SwapChain;
	ImGuiContext*                     ImguiContext;
	void*                             ImguiMalloc;
	void*                             ImguiFree;
	GUI::GUI_ADDRENDER                     RegisterRender;
	GUI::GUI_REMRENDER                     DeregisterRender;

	/* Updater */
	UPDATER_REQUESTUPDATE             RequestUpdate;

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
	Core::LOGGER_LOG2                       Log;

	/* GUI Alerts */
	GUI::ALERTS_NOTIFY                     SendAlert;

	/* Events */
	Host::EVENTS_RAISE                      RaiseEvent;
	Host::EVENTS_RAISENOTIFICATION          RaiseEventNotification;
	Host::EVENTS_RAISE_TARGETED             RaiseEventTargeted;
	Host::EVENTS_RAISENOTIFICATION_TARGETED RaiseEventNotificationTargeted;
	Host::EVENTS_SUBSCRIBE                  SubscribeEvent;
	Host::EVENTS_SUBSCRIBE                  UnsubscribeEvent;

	/* WndProc */
	Platform::WNDPROC_ADDREM          RegisterWndProc;
	Platform::WNDPROC_ADDREM          DeregisterWndProc;
	Platform::WNDPROC_SENDTOGAME      SendWndProcToGameOnly;

	/* InputBinds */
	INPUTBINDS_INVOKE                 InvokeInputBind;
	INPUTBINDS_REGISTERWITHSTRING2    RegisterInputBindWithString;
	INPUTBINDS_REGISTERWITHSTRUCT2    RegisterInputBindWithStruct;
	INPUTBINDS_DEREGISTER             DeregisterInputBind;

	/* DataLink */
	Core::DATALINK_GETRESOURCE              GetResource;
	Core::DATALINK_SHARERESOURCE            ShareResource;

	/* Textures */
	Graphics::TEXTURES_GET                      GetTexture;
	Graphics::TEXTURES_GETORCREATEFROMFILE      GetTextureOrCreateFromFile;
	Graphics::TEXTURES_GETORCREATEFROMRESOURCE  GetTextureOrCreateFromResource;
	Graphics::TEXTURES_GETORCREATEFROMURL       GetTextureOrCreateFromURL;
	Graphics::TEXTURES_GETORCREATEFROMMEMORY    GetTextureOrCreateFromMemory;
	Graphics::TEXTURES_LOADFROMFILE             LoadTextureFromFile;
	Graphics::TEXTURES_LOADFROMRESOURCE         LoadTextureFromResource;
	Graphics::TEXTURES_LOADFROMURL              LoadTextureFromURL;
	Graphics::TEXTURES_LOADFROMMEMORY           LoadTextureFromMemory;

	/* Shortcuts */
	GUI::QUICKACCESS_ADDSHORTCUT           AddShortcut;
	GUI::QUICKACCESS_GENERIC               RemoveShortcut;
	GUI::QUICKACCESS_GENERIC               PushNotification;
	GUI::QUICKACCESS_ADDSIMPLE             AddSimpleShortcut;
	GUI::QUICKACCESS_GENERIC               RemoveSimpleShortcut;

	/* Localization */
	GUI::LOCALIZATION_TRANSLATE            Translate;
	GUI::LOCALIZATION_TRANSLATETO          TranslateTo;
	GUI::LOCALIZATION_SET                  SetTranslatedString;

	/* Fonts */
	GUI::FONTS_GETRELEASE                  GetFont;
	GUI::FONTS_GETRELEASE                  ReleaseFont;
	GUI::FONTS_ADDFROMFILE                 AddFontFromFile;
	GUI::FONTS_ADDFROMRESOURCE             AddFontFromResource;
	GUI::FONTS_ADDFROMMEMORY               AddFontFromMemory;
};
