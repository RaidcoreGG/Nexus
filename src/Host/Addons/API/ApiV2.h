///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AdoApiV2.h
/// Description  :  Addon API Revision 2.
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
#include "Platform/RawInput/RiFuncDefs.h"
#include "UI/Services/Localization/LoclFuncDefs.h"
#include "Graphics/Textures/TxFuncDefs.h"
#include "UI/UiFuncDefs.h"
#include "UI/Widgets/QuickAccess/QaFuncDefs.h"

using namespace Raidcore::Nexus;

///----------------------------------------------------------------------------------------------------
/// AddonAPI2_t Struct
///----------------------------------------------------------------------------------------------------
struct AddonAPI2_t : AddonAPI_t
{
	/* Renderer */
	IDXGISwapChain*                  SwapChain;
	ImGuiContext*                    ImguiContext;
	void*                            ImguiMalloc;
	void*                            ImguiFree;
	GUI_ADDRENDER                    RegisterRender;
	GUI_REMRENDER                    DeregisterRender;

	/* Paths */
	IDX_GETGAMEDIR                   GetGameDirectory;
	IDX_GETADDONDIR                  GetAddonDirectory;
	IDX_GETCOMMONDIR                 GetCommonDirectory;

	/* Minhook */
	MINHOOK_CREATE                   CreateHook;
	MINHOOK_REMOVE                   RemoveHook;
	MINHOOK_ENABLE                   EnableHook;
	MINHOOK_DISABLE                  DisableHook;

	/* Logging */
	LOGGER_LOG2                      Log;

	/* Events */
	Host::EVENTS_RAISE               RaiseEvent;
	Host::EVENTS_RAISENOTIFICATION   RaiseEventNotification;
	Host::EVENTS_SUBSCRIBE           SubscribeEvent;
	Host::EVENTS_SUBSCRIBE           UnsubscribeEvent;

	/* WndProc */
	Platform::WNDPROC_ADDREM         RegisterWndProc;
	Platform::WNDPROC_ADDREM         DeregisterWndProc;
	Platform::WNDPROC_SENDTOGAME     SendWndProcToGameOnly;

	/* InputBinds */
	INPUTBINDS_REGISTERWITHSTRING    RegisterInputBindWithString;
	INPUTBINDS_REGISTERWITHSTRUCT    RegisterInputBindWithStruct;
	INPUTBINDS_DEREGISTER            DeregisterInputBind;

	/* DataLink */
	DATALINK_GETRESOURCE             GetResource;
	DATALINK_SHARERESOURCE           ShareResource;

	/* Textures */
	Graphics::TEXTURES_GET                     GetTexture;
	Graphics::TEXTURES_GETORCREATEFROMFILE     GetTextureOrCreateFromFile;
	Graphics::TEXTURES_GETORCREATEFROMRESOURCE GetTextureOrCreateFromResource;
	Graphics::TEXTURES_GETORCREATEFROMURL      GetTextureOrCreateFromURL;
	Graphics::TEXTURES_GETORCREATEFROMMEMORY   GetTextureOrCreateFromMemory;
	Graphics::TEXTURES_LOADFROMFILE            LoadTextureFromFile;
	Graphics::TEXTURES_LOADFROMRESOURCE        LoadTextureFromResource;
	Graphics::TEXTURES_LOADFROMURL             LoadTextureFromURL;
	Graphics::TEXTURES_LOADFROMMEMORY          LoadTextureFromMemory;

	/* Shortcuts */
	QUICKACCESS_ADDSHORTCUT          AddShortcut;
	QUICKACCESS_GENERIC              RemoveShortcut;
	QUICKACCESS_GENERIC              PushNotification;
	QUICKACCESS_ADDSIMPLE            AddSimpleShortcut;
	QUICKACCESS_GENERIC              RemoveSimpleShortcut;

	LOCALIZATION_TRANSLATE           Translate;
	LOCALIZATION_TRANSLATETO         TranslateTo;
};
