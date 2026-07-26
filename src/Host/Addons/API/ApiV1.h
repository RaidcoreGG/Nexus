///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AdoApiV1.h
/// Description  :  Addon API Revision 1.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "ApiBase.h"

#include <dxgi.h>

#include "thirdparty/imgui/imgui.h"
#include "thirdparty/minhook/mh_hook.h"

using namespace Raidcore::Nexus;

///----------------------------------------------------------------------------------------------------
/// AddonAPI1_t Struct
///----------------------------------------------------------------------------------------------------
struct AddonAPI1_t : AddonAPI_t
{
	/* Renderer */
	IDXGISwapChain* SwapChain;
	ImGuiContext* ImguiContext;
	void* ImguiMalloc;
	void* ImguiFree;
	GUI_ADDRENDER                 RegisterRender;
	GUI_REMRENDER                 DeregisterRender;

	/* Paths */
	IDX_GETGAMEDIR                GetGameDirectory;
	IDX_GETADDONDIR               GetAddonDirectory;
	IDX_GETCOMMONDIR              GetCommonDirectory;

	/* Minhook */
	MINHOOK_CREATE                CreateHook;
	MINHOOK_REMOVE                RemoveHook;
	MINHOOK_ENABLE                EnableHook;
	MINHOOK_DISABLE               DisableHook;

	/* Logging */
	LOGGER_LOG                    Log;

	/* Events */
	EVENTS_RAISE            RaiseEvent;
	EVENTS_SUBSCRIBE        SubscribeEvent;
	EVENTS_SUBSCRIBE        UnsubscribeEvent;

	/* WndProc */
	WNDPROC_ADDREM      RegisterWndProc;
	WNDPROC_ADDREM      DeregisterWndProc;

	/* InputBinds */
	INPUTBINDS_REGISTERWITHSTRING RegisterInputBindWithString;
	INPUTBINDS_REGISTERWITHSTRUCT RegisterInputBindWithStruct;
	INPUTBINDS_DEREGISTER         DeregisterInputBind;

	/* DataLink */
	DATALINK_GETRESOURCE          Get;
	DATALINK_SHARERESOURCE        Share;

	/* Textures */
	TEXTURES_GET                  GetTexture;
	TEXTURES_LOADFROMFILE         LoadTextureFromFile;
	TEXTURES_LOADFROMRESOURCE     LoadTextureFromResource;
	TEXTURES_LOADFROMURL          LoadTextureFromURL;

	/* Shortcuts */
	QUICKACCESS_ADDSHORTCUT       AddShortcut;
	QUICKACCESS_GENERIC           RemoveShortcut;
	QUICKACCESS_ADDSIMPLE         AddSimpleShortcut;
	QUICKACCESS_GENERIC           RemoveSimpleShortcut;
};
