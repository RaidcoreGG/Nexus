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

#include "Core/Logging/LogEnum.h"
#include "Engine/Events/EvtSubscriber.h"
#include "Engine/Inputs/InputBinds/IbMapping.h"
#include "Engine/Inputs/RawInput/RiApi.h"
#include "UI/Textures/TxQueueEntry.h"
#include "UI/UiRender.h"

typedef void* (*DATALINK_GETRESOURCE)  (const char* aIdentifier);
typedef void* (*DATALINK_SHARERESOURCE)(const char* aIdentifier, size_t aResourceSize);

typedef void (*EVENTS_RAISE)    (const char* aIdentifier, void* aEventData);
typedef void (*EVENTS_SUBSCRIBE)(const char* aIdentifier, EVENT_CONSUME aConsumeEventCallback);

typedef void (*INPUTBINDS_REGISTERWITHSTRING)(const char* aIdentifier, INPUTBINDS_PROCESS aInputBindHandler, const char* aInputBind);
typedef void (*INPUTBINDS_REGISTERWITHSTRUCT)(const char* aIdentifier, INPUTBINDS_PROCESS aInputBindHandler, InputBindV1_t aInputBind);
typedef void (*INPUTBINDS_DEREGISTER)        (const char* aIdentifier);

typedef void (*WNDPROC_ADDREM)(WNDPROC_CALLBACK aWndProcCallback);

typedef const char* (*IDX_GETGAMEDIR)  ();
typedef const char* (*IDX_GETADDONDIR) (const char* aName);
typedef const char* (*IDX_GETCOMMONDIR)();

typedef Texture_t* (*TEXTURES_GET)             (const char* aIdentifier);
typedef void       (*TEXTURES_LOADFROMFILE)    (const char* aIdentifier, const char* aFilename, TEXTURES_RECEIVECALLBACK aCallback);
typedef void       (*TEXTURES_LOADFROMRESOURCE)(const char* aIdentifier, unsigned aResourceID, HMODULE aModule, TEXTURES_RECEIVECALLBACK aCallback);
typedef void       (*TEXTURES_LOADFROMURL)     (const char* aIdentifier, const char* aRemote, const char* aEndpoint, TEXTURES_RECEIVECALLBACK aCallback);

typedef void (*GUI_ADDRENDER)(ERenderType aRenderType, GUI_RENDER aRenderCallback);
typedef void (*GUI_REMRENDER)(GUI_RENDER aRenderCallback);

typedef void (*QUICKACCESS_ADDSHORTCUT)(const char* aIdentifier, const char* aTextureIdentifier, const char* aTextureHoverIdentifier, const char* aInputBindIdentifier, const char* aTooltipText);
typedef void (*QUICKACCESS_ADDSIMPLE)  (const char* aIdentifier, GUI_RENDER aShortcutRenderCallback);
typedef void (*QUICKACCESS_GENERIC)    (const char* aIdentifier);

typedef void (*LOGGER_LOG)(ELogLevel aLogLevel, const char* aStr);

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
	EVENTS_RAISE                  RaiseEvent;
	EVENTS_SUBSCRIBE              SubscribeEvent;
	EVENTS_SUBSCRIBE              UnsubscribeEvent;

	/* WndProc */
	WNDPROC_ADDREM                RegisterWndProc;
	WNDPROC_ADDREM                DeregisterWndProc;

	/* InputBinds */
	INPUTBINDS_REGISTERWITHSTRING RegisterInputBindWithString;
	INPUTBINDS_REGISTERWITHSTRUCT RegisterInputBindWithStruct;
	INPUTBINDS_DEREGISTER         DeregisterInputBind;

	/* DataLink */
	DATALINK_GETRESOURCE          GetResource;
	DATALINK_SHARERESOURCE        ShareResource;

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
