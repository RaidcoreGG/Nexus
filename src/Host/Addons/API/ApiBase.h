///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AdoApiBase.h
/// Description  :  Addon API base.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <imgui.h>
#include <windows.h>

#include "Core/Logging/LogEnum.h"
#include "Graphics/Textures/TxQueueEntry.h"
#include "Graphics/Textures/TxTexture.h"
#include "GW2/Inputs/GameBinds/GbEnum.h"
#include "Host/Events/EvtSubscriber.h"
#include "Inputs/InputBinds/IbBind.h"
#include "Inputs/InputBinds/IbMapping.h"
#include "Platform/RawInput/RiApi.h"
#include "UI/Services/Fonts/FontManager.h"
#include "UI/UiEnum.h"
#include "UI/UiFuncDefs.h"
#include "UI/Views/Alerts/AlEnum.h"

using namespace Raidcore::Nexus;

typedef void (*UPDATER_REQUESTUPDATE)(signed int aSignature, const char* aUpdateURL);

typedef void* (*DATALINK_GETRESOURCE)  (const char* aIdentifier);
typedef void* (*DATALINK_SHARERESOURCE)(const char* aIdentifier, size_t aResourceSize);

typedef void (*LOGGER_LOG) (Core::ELogLevel aLogLevel, const char* aStr);
typedef void (*LOGGER_LOG2)(Core::ELogLevel aLogLevel, const char* aChannel, const char* aStr);

typedef Graphics::Texture_t* (*TEXTURES_GET)                    (const char* aIdentifier);
typedef Graphics::Texture_t* (*TEXTURES_GETORCREATEFROMFILE)    (const char* aIdentifier, const char* aFilename);
typedef Graphics::Texture_t* (*TEXTURES_GETORCREATEFROMRESOURCE)(const char* aIdentifier, unsigned aResourceID, HMODULE aModule);
typedef Graphics::Texture_t* (*TEXTURES_GETORCREATEFROMURL)     (const char* aIdentifier, const char* aRemote, const char* aEndpoint);
typedef Graphics::Texture_t* (*TEXTURES_GETORCREATEFROMURL2)    (const char* aIdentifier, const char* aURL);
typedef Graphics::Texture_t* (*TEXTURES_GETORCREATEFROMMEMORY)  (const char* aIdentifier, void* aData, size_t aSize);
typedef void       (*TEXTURES_LOADFROMFILE)           (const char* aIdentifier, const char* aFilename, Graphics::TEXTURES_RECEIVECALLBACK aCallback);
typedef void       (*TEXTURES_LOADFROMRESOURCE)       (const char* aIdentifier, unsigned aResourceID, HMODULE aModule, Graphics::TEXTURES_RECEIVECALLBACK aCallback);
typedef void       (*TEXTURES_LOADFROMURL)            (const char* aIdentifier, const char* aRemote, const char* aEndpoint, Graphics::TEXTURES_RECEIVECALLBACK aCallback);
typedef void       (*TEXTURES_LOADFROMURL2)           (const char* aIdentifier, const char* aURL, Graphics::TEXTURES_RECEIVECALLBACK aCallback);
typedef void       (*TEXTURES_LOADFROMMEMORY)         (const char* aIdentifier, void* aData, size_t aSize, Graphics::TEXTURES_RECEIVECALLBACK aCallback);

typedef void (*GAMEBINDS_PRESSASYNC)  (GW2::EGameBinds aGameBind);
typedef void (*GAMEBINDS_RELEASEASYNC)(GW2::EGameBinds aGameBind);
typedef void (*GAMEBINDS_INVOKEASYNC) (GW2::EGameBinds aGameBind, int aDuration);
typedef void (*GAMEBINDS_PRESS)       (GW2::EGameBinds aGameBind);
typedef void (*GAMEBINDS_RELEASE)     (GW2::EGameBinds aGameBind);
typedef bool (*GAMEBINDS_ISBOUND)     (GW2::EGameBinds aGameBind);

typedef void (*EVENTS_RAISE)                     (const char* aIdentifier, void* aEventData);
typedef void (*EVENTS_RAISENOTIFICATION)         (const char* aIdentifier);
typedef void (*EVENTS_RAISE_TARGETED)            (uint32_t aSignature, const char* aIdentifier, void* aEventData);
typedef void (*EVENTS_RAISENOTIFICATION_TARGETED)(uint32_t aSignature, const char* aIdentifier);
typedef void (*EVENTS_SUBSCRIBE)                 (const char* aIdentifier, Host::EVENT_CONSUME aConsumeEventCallback);

typedef const char* (*IDX_GETGAMEDIR)  ();
typedef const char* (*IDX_GETADDONDIR) (const char* aName);
typedef const char* (*IDX_GETCOMMONDIR)();

/* Input Handler Down Only */
typedef void (*INPUTBINDS_REGISTERWITHSTRING) (const char* aIdentifier, Input::INPUTBINDS_PROCESS aInputBindHandler, const char* aInputBind);
typedef void (*INPUTBINDS_REGISTERWITHSTRUCT) (const char* aIdentifier, Input::INPUTBINDS_PROCESS aInputBindHandler, Input::InputBindV1_t aInputBind);

/* Input Handler Down and Release */
typedef void (*INPUTBINDS_REGISTERWITHSTRING2)(const char* aIdentifier, Input::INPUTBINDS_PROCESS2 aInputBindHandler, const char* aInputBind);
typedef void (*INPUTBINDS_REGISTERWITHSTRUCT2)(const char* aIdentifier, Input::INPUTBINDS_PROCESS2 aInputBindHandler, Input::InputBindV1_t aInputBind);
typedef void (*INPUTBINDS_INVOKE)             (const char* aIdentifier, bool aIsRelease);

/* Input Handler Down and Release also return whether input should not be processed further. */
typedef void (*INPUTBINDS_REGISTERWITHSTRING3)(const char* aIdentifier, Input::INPUTBINDS_PROCESS3 aInputBindHandler, const char* aInputBind);
typedef void (*INPUTBINDS_REGISTERWITHSTRUCT3)(const char* aIdentifier, Input::INPUTBINDS_PROCESS3 aInputBindHandler, Input::InputBindV1_t aInputBind);
typedef bool (*INPUTBINDS_INVOKE2)            (const char* aIdentifier, bool aIsRelease);

typedef void (*INPUTBINDS_DEREGISTER)         (const char* aIdentifier);

typedef void    (*WNDPROC_ADDREM)    (Platform::WNDPROC_CALLBACK aWndProcCallback);
typedef LRESULT(*WNDPROC_SENDTOGAME)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef void (*FONTS_GETRELEASE)(const char* aIdentifier, GUI::FONTS_RECEIVECALLBACK aCallback);
typedef void (*FONTS_ADDFROMFILE)(const char* aIdentifier, float aFontSize, const char* aFilename, GUI::FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig);
typedef void (*FONTS_ADDFROMRESOURCE)(const char* aIdentifier, float aFontSize, unsigned aResourceID, HMODULE aModule, GUI::FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig);
typedef void (*FONTS_ADDFROMMEMORY)(const char* aIdentifier, float aFontSize, void* aData, size_t aSize, GUI::FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig);
typedef void (*FONTS_RESIZE)(const char* aIdentifier, float aFontSize);

typedef const char* (*LOCALIZATION_TRANSLATE)  (const char* aIdentifier);
typedef const char* (*LOCALIZATION_TRANSLATETO)(const char* aIdentifier, const char* aLanguageIdentifier);
typedef void        (*LOCALIZATION_SET)        (const char* aIdentifier, const char* aLanguageIdentifier, const char* aString);

typedef void (*GUI_REGISTERCLOSEONESCAPE)(const char* aWindowName, bool* aIsVisible);
typedef void (*GUI_DEREGISTERCLOSEONESCAPE)(const char* aWindowName);

typedef void (*ALERTS_NOTIFY)  (const char* aMessage);
typedef void (*ALERTS_NOTIFY2) (GUI::EAlertType aType, const char* aMessage);

typedef void (*QUICKACCESS_ADDSHORTCUT) (const char* aIdentifier, const char* aTextureIdentifier, const char* aTextureHoverIdentifier, const char* aInputBindIdentifier, const char* aTooltipText);
typedef void (*QUICKACCESS_ADDSIMPLE)   (const char* aIdentifier, GUI::GUI_RENDER aShortcutRenderCallback);
typedef void (*QUICKACCESS_ADDSIMPLE2)  (const char* aIdentifier, const char* aTargetShortcutIdentifier, GUI::GUI_RENDER aShortcutRenderCallback);
typedef void (*QUICKACCESS_GENERIC)     (const char* aIdentifier);

typedef void (*GUI_ADDRENDER)(GUI::ERenderType aRenderType, GUI::GUI_RENDER aRenderCallback);
typedef void (*GUI_REMRENDER)(GUI::GUI_RENDER aRenderCallback);

///----------------------------------------------------------------------------------------------------
/// AddonAPI_t Struct
///----------------------------------------------------------------------------------------------------
struct AddonAPI_t {};
