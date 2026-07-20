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

#include "Core/Index/IdxFuncDefs.h"
#include "Core/DataLink/DlFuncDefs.h"
#include "Host/Events/EvtFuncDefs.h"
#include "Engine/Inputs/InputBinds/IbFuncDefs.h"
#include "Core/Logging/LogFuncDefs.h"
#include "Platform/RawInput/RiFuncDefs.h"
#include "Graphics/Textures/TxFuncDefs.h"
#include "UI/UiFuncDefs.h"
#include "UI/Widgets/QuickAccess/QaFuncDefs.h"

using namespace Raidcore::Nexus;

///----------------------------------------------------------------------------------------------------
/// AddonAPI1_t Struct
///----------------------------------------------------------------------------------------------------
struct AddonAPI1_t : AddonAPI_t
{
	/* Renderer */
	IDXGISwapChain*               SwapChain;
	ImGuiContext*                 ImguiContext;
	void*                         ImguiMalloc;
	void*                         ImguiFree;
	GUI::GUI_ADDRENDER                 RegisterRender;
	GUI::GUI_REMRENDER                 DeregisterRender;

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
	Host::EVENTS_RAISE            RaiseEvent;
	Host::EVENTS_SUBSCRIBE        SubscribeEvent;
	Host::EVENTS_SUBSCRIBE        UnsubscribeEvent;

	/* WndProc */
	Platform::WNDPROC_ADDREM      RegisterWndProc;
	Platform::WNDPROC_ADDREM      DeregisterWndProc;

	/* InputBinds */
	INPUTBINDS_REGISTERWITHSTRING RegisterInputBindWithString;
	INPUTBINDS_REGISTERWITHSTRUCT RegisterInputBindWithStruct;
	INPUTBINDS_DEREGISTER         DeregisterInputBind;

	/* DataLink */
	DATALINK_GETRESOURCE          GetResource;
	DATALINK_SHARERESOURCE        ShareResource;

	/* Textures */
	Graphics::TEXTURES_GET                  GetTexture;
	Graphics::TEXTURES_LOADFROMFILE         LoadTextureFromFile;
	Graphics::TEXTURES_LOADFROMRESOURCE     LoadTextureFromResource;
	Graphics::TEXTURES_LOADFROMURL          LoadTextureFromURL;

	/* Shortcuts */
	GUI::QUICKACCESS_ADDSHORTCUT       AddShortcut;
	GUI::QUICKACCESS_GENERIC           RemoveShortcut;
	GUI::QUICKACCESS_ADDSIMPLE         AddSimpleShortcut;
	GUI::QUICKACCESS_GENERIC           RemoveSimpleShortcut;
};
