#ifndef ADDONAPI_H
#define ADDONAPI_H

#include <dxgi.h>
#include "Paths/FuncDefs.h"
#include "Services/Logging/FuncDefs.h"
#include "Events/FuncDefs.h"
#include "Inputs/RawInput/FuncDefs.h"
#include "Inputs/Keybinds/FuncDefs.h"
#include "imgui/imgui.h"
#include "minhook/mh_hook.h"
#include "Services/DataLink/FuncDefs.h"
#include "Services/Textures/FuncDefs.h"
#include "GUI/FuncDefs.h"
#include "GUI/Widgets/QuickAccess/FuncDefs.h"
#include "GUI/Widgets/Alerts/FuncDefs.h"
#include "Services/Localization/FuncDefs.h"
#include "Services/Updater/FuncDefs.h"
#include "GUI/Fonts/FuncDefs.h"

// Base
struct AddonAPI {};

// Revision 1
struct AddonAPI1 : AddonAPI
{
	/* Renderer */
	IDXGISwapChain*				SwapChain;
	ImGuiContext*				ImguiContext;
	void*						ImguiMalloc;
	void*						ImguiFree;
	GUI_ADDRENDER				RegisterRender;
	GUI_REMRENDER				DeregisterRender;

	/* Paths */
	PATHS_GETGAMEDIR			GetGameDirectory;
	PATHS_GETADDONDIR			GetAddonDirectory;
	PATHS_GETCOMMONDIR			GetCommonDirectory;

	/* Minhook */
	MINHOOK_CREATE				CreateHook;
	MINHOOK_REMOVE				RemoveHook;
	MINHOOK_ENABLE				EnableHook;
	MINHOOK_DISABLE				DisableHook;

	/* Logging */
	LOGGER_LOG					Log;

	/* Events */
	EVENTS_RAISE				RaiseEvent;
	EVENTS_SUBSCRIBE			SubscribeEvent;
	EVENTS_SUBSCRIBE			UnsubscribeEvent;

	/* WndProc */
	WNDPROC_ADDREM				RegisterWndProc;
	WNDPROC_ADDREM				DeregisterWndProc;

	/* Keybinds */
	KEYBINDS_REGISTERWITHSTRING	RegisterKeybindWithString;
	KEYBINDS_REGISTERWITHSTRUCT	RegisterKeybindWithStruct;
	KEYBINDS_DEREGISTER			DeregisterKeybind;

	/* DataLink */
	DATALINK_GETRESOURCE		GetResource;
	DATALINK_SHARERESOURCE		ShareResource;

	/* Textures */
	TEXTURES_GET				GetTexture;
	TEXTURES_LOADFROMFILE		LoadTextureFromFile;
	TEXTURES_LOADFROMRESOURCE	LoadTextureFromResource;
	TEXTURES_LOADFROMURL		LoadTextureFromURL;

	/* Shortcuts */
	QUICKACCESS_ADDSHORTCUT		AddShortcut;
	QUICKACCESS_GENERIC			RemoveShortcut;
	QUICKACCESS_ADDSIMPLE		AddSimpleShortcut;
	QUICKACCESS_GENERIC			RemoveSimpleShortcut;
};

// Revision 2
struct AddonAPI2 : AddonAPI
{
	/* Renderer */
	IDXGISwapChain*						SwapChain;
	ImGuiContext*						ImguiContext;
	void*								ImguiMalloc;
	void*								ImguiFree;
	GUI_ADDRENDER						RegisterRender;
	GUI_REMRENDER						DeregisterRender;

	/* Paths */
	PATHS_GETGAMEDIR					GetGameDirectory;
	PATHS_GETADDONDIR					GetAddonDirectory;
	PATHS_GETCOMMONDIR					GetCommonDirectory;

	/* Minhook */
	MINHOOK_CREATE						CreateHook;
	MINHOOK_REMOVE						RemoveHook;
	MINHOOK_ENABLE						EnableHook;
	MINHOOK_DISABLE						DisableHook;

	/* Logging */
	LOGGER_LOG2							Log;

	/* Events */
	EVENTS_RAISE						RaiseEvent;
	EVENTS_RAISENOTIFICATION			RaiseEventNotification;
	EVENTS_SUBSCRIBE					SubscribeEvent;
	EVENTS_SUBSCRIBE					UnsubscribeEvent;

	/* WndProc */
	WNDPROC_ADDREM						RegisterWndProc;
	WNDPROC_ADDREM						DeregisterWndProc;
	WNDPROC_SENDTOGAME					SendWndProcToGameOnly;

	/* Keybinds */
	KEYBINDS_REGISTERWITHSTRING			RegisterKeybindWithString;
	KEYBINDS_REGISTERWITHSTRUCT			RegisterKeybindWithStruct;
	KEYBINDS_DEREGISTER					DeregisterKeybind;

	/* DataLink */
	DATALINK_GETRESOURCE				GetResource;
	DATALINK_SHARERESOURCE				ShareResource;

	/* Textures */
	TEXTURES_GET						GetTexture;
	TEXTURES_GETORCREATEFROMFILE		GetTextureOrCreateFromFile;
	TEXTURES_GETORCREATEFROMRESOURCE	GetTextureOrCreateFromResource;
	TEXTURES_GETORCREATEFROMURL			GetTextureOrCreateFromURL;
	TEXTURES_GETORCREATEFROMMEMORY		GetTextureOrCreateFromMemory;
	TEXTURES_LOADFROMFILE				LoadTextureFromFile;
	TEXTURES_LOADFROMRESOURCE			LoadTextureFromResource;
	TEXTURES_LOADFROMURL				LoadTextureFromURL;
	TEXTURES_LOADFROMMEMORY				LoadTextureFromMemory;

	/* Shortcuts */
	QUICKACCESS_ADDSHORTCUT				AddShortcut;
	QUICKACCESS_GENERIC					RemoveShortcut;
	QUICKACCESS_GENERIC					NotifyShortcut;
	QUICKACCESS_ADDSIMPLE				AddSimpleShortcut;
	QUICKACCESS_GENERIC					RemoveSimpleShortcut;

	LOCALIZATION_TRANSLATE				Translate;
	LOCALIZATION_TRANSLATETO			TranslateTo;
};

// Revision 3
struct AddonAPI3 : AddonAPI
{
	/* Renderer */
	IDXGISwapChain*						SwapChain;
	ImGuiContext*						ImguiContext;
	void*								ImguiMalloc;
	void*								ImguiFree;
	GUI_ADDRENDER						RegisterRender;
	GUI_REMRENDER						DeregisterRender;

	/* Paths */
	PATHS_GETGAMEDIR					GetGameDirectory;
	PATHS_GETADDONDIR					GetAddonDirectory;
	PATHS_GETCOMMONDIR					GetCommonDirectory;

	/* Minhook */
	MINHOOK_CREATE						CreateHook;
	MINHOOK_REMOVE						RemoveHook;
	MINHOOK_ENABLE						EnableHook;
	MINHOOK_DISABLE						DisableHook;

	/* Logging */
	LOGGER_LOG2							Log;

	/* GUI Alerts */
	ALERTS_NOTIFY						SendAlert;

	/* Events */
	EVENTS_RAISE						RaiseEvent;
	EVENTS_RAISENOTIFICATION			RaiseEventNotification;
	EVENTS_RAISE_TARGETED				RaiseEventTargeted;
	EVENTS_RAISENOTIFICATION_TARGETED	RaiseEventNotificationTargeted;
	EVENTS_SUBSCRIBE					SubscribeEvent;
	EVENTS_SUBSCRIBE					UnsubscribeEvent;

	/* WndProc */
	WNDPROC_ADDREM						RegisterWndProc;
	WNDPROC_ADDREM						DeregisterWndProc;
	WNDPROC_SENDTOGAME					SendWndProcToGameOnly;

	/* Keybinds */
	KEYBINDS_REGISTERWITHSTRING			RegisterKeybindWithString;
	KEYBINDS_REGISTERWITHSTRUCT			RegisterKeybindWithStruct;
	KEYBINDS_DEREGISTER					DeregisterKeybind;

	/* DataLink */
	DATALINK_GETRESOURCE				GetResource;
	DATALINK_SHARERESOURCE				ShareResource;

	/* Textures */
	TEXTURES_GET						GetTexture;
	TEXTURES_GETORCREATEFROMFILE		GetTextureOrCreateFromFile;
	TEXTURES_GETORCREATEFROMRESOURCE	GetTextureOrCreateFromResource;
	TEXTURES_GETORCREATEFROMURL			GetTextureOrCreateFromURL;
	TEXTURES_GETORCREATEFROMMEMORY		GetTextureOrCreateFromMemory;
	TEXTURES_LOADFROMFILE				LoadTextureFromFile;
	TEXTURES_LOADFROMRESOURCE			LoadTextureFromResource;
	TEXTURES_LOADFROMURL				LoadTextureFromURL;
	TEXTURES_LOADFROMMEMORY				LoadTextureFromMemory;

	/* Shortcuts */
	QUICKACCESS_ADDSHORTCUT				AddShortcut;
	QUICKACCESS_GENERIC					RemoveShortcut;
	QUICKACCESS_GENERIC					NotifyShortcut;
	QUICKACCESS_ADDSIMPLE				AddSimpleShortcut;
	QUICKACCESS_GENERIC					RemoveSimpleShortcut;

	/* Localization */
	LOCALIZATION_TRANSLATE				Translate;
	LOCALIZATION_TRANSLATETO			TranslateTo;
};

// Revision 4
struct AddonAPI4 : AddonAPI
{
	/* Renderer */
	IDXGISwapChain*						SwapChain;
	ImGuiContext*						ImguiContext;
	void*								ImguiMalloc;
	void*								ImguiFree;
	GUI_ADDRENDER						RegisterRender;
	GUI_REMRENDER						DeregisterRender;

	/* Updater */
	UPDATER_REQUESTUPDATE				RequestUpdate;

	/* Paths */
	PATHS_GETGAMEDIR					GetGameDirectory;
	PATHS_GETADDONDIR					GetAddonDirectory;
	PATHS_GETCOMMONDIR					GetCommonDirectory;

	/* Minhook */
	MINHOOK_CREATE						CreateHook;
	MINHOOK_REMOVE						RemoveHook;
	MINHOOK_ENABLE						EnableHook;
	MINHOOK_DISABLE						DisableHook;

	/* Logging */
	LOGGER_LOG2							Log;

	/* GUI Alerts */
	ALERTS_NOTIFY						SendAlert;

	/* Events */
	EVENTS_RAISE						RaiseEvent;
	EVENTS_RAISENOTIFICATION			RaiseEventNotification;
	EVENTS_RAISE_TARGETED				RaiseEventTargeted;
	EVENTS_RAISENOTIFICATION_TARGETED	RaiseEventNotificationTargeted;
	EVENTS_SUBSCRIBE					SubscribeEvent;
	EVENTS_SUBSCRIBE					UnsubscribeEvent;

	/* WndProc */
	WNDPROC_ADDREM						RegisterWndProc;
	WNDPROC_ADDREM						DeregisterWndProc;
	WNDPROC_SENDTOGAME					SendWndProcToGameOnly;

	/* Keybinds */
	KEYBINDS_REGISTERWITHSTRING2		RegisterKeybindWithString;
	KEYBINDS_REGISTERWITHSTRUCT2		RegisterKeybindWithStruct;
	KEYBINDS_DEREGISTER					DeregisterKeybind;

	/* DataLink */
	DATALINK_GETRESOURCE				GetResource;
	DATALINK_SHARERESOURCE				ShareResource;

	/* Textures */
	TEXTURES_GET						GetTexture;
	TEXTURES_GETORCREATEFROMFILE		GetTextureOrCreateFromFile;
	TEXTURES_GETORCREATEFROMRESOURCE	GetTextureOrCreateFromResource;
	TEXTURES_GETORCREATEFROMURL			GetTextureOrCreateFromURL;
	TEXTURES_GETORCREATEFROMMEMORY		GetTextureOrCreateFromMemory;
	TEXTURES_LOADFROMFILE				LoadTextureFromFile;
	TEXTURES_LOADFROMRESOURCE			LoadTextureFromResource;
	TEXTURES_LOADFROMURL				LoadTextureFromURL;
	TEXTURES_LOADFROMMEMORY				LoadTextureFromMemory;

	/* Shortcuts */
	QUICKACCESS_ADDSHORTCUT				AddShortcut;
	QUICKACCESS_GENERIC					RemoveShortcut;
	QUICKACCESS_GENERIC					NotifyShortcut;
	QUICKACCESS_ADDSIMPLE				AddSimpleShortcut;
	QUICKACCESS_GENERIC					RemoveSimpleShortcut;

	/* Localization */
	LOCALIZATION_TRANSLATE				Translate;
	LOCALIZATION_TRANSLATETO			TranslateTo;

	/* Fonts */
	FONTS_GETRELEASE					GetFont;
	FONTS_GETRELEASE					ReleaseFont;
	FONTS_ADDFROMFILE					AddFontFromFile;
	FONTS_ADDFROMRESOURCE				AddFontFromResource;
	FONTS_ADDFROMMEMORY					AddFontFromMemory;
};

// Revision 5
struct AddonAPI5 : AddonAPI
{
	/* Renderer */
	IDXGISwapChain*						SwapChain;
	ImGuiContext*						ImguiContext;
	void*								ImguiMalloc;
	void*								ImguiFree;
	GUI_ADDRENDER						RegisterRender;
	GUI_REMRENDER						DeregisterRender;

	/* Updater */
	UPDATER_REQUESTUPDATE				RequestUpdate;

	/* Paths */
	PATHS_GETGAMEDIR					GetGameDirectory;
	PATHS_GETADDONDIR					GetAddonDirectory;
	PATHS_GETCOMMONDIR					GetCommonDirectory;

	/* Minhook */
	MINHOOK_CREATE						CreateHook;
	MINHOOK_REMOVE						RemoveHook;
	MINHOOK_ENABLE						EnableHook;
	MINHOOK_DISABLE						DisableHook;

	/* Logging */
	LOGGER_LOG2							Log;

	/* GUI Alerts */
	ALERTS_NOTIFY						SendAlert;

	/* Events */
	EVENTS_RAISE						RaiseEvent;
	EVENTS_RAISENOTIFICATION			RaiseEventNotification;
	EVENTS_RAISE_TARGETED				RaiseEventTargeted;
	EVENTS_RAISENOTIFICATION_TARGETED	RaiseEventNotificationTargeted;
	EVENTS_SUBSCRIBE					SubscribeEvent;
	EVENTS_SUBSCRIBE					UnsubscribeEvent;

	/* WndProc */
	WNDPROC_ADDREM						RegisterWndProc;
	WNDPROC_ADDREM						DeregisterWndProc;
	WNDPROC_SENDTOGAME					SendWndProcToGameOnly;

	/* Keybinds */
	KEYBINDS_INVOKE						InvokeKeybind;
	KEYBINDS_REGISTERWITHSTRING2		RegisterKeybindWithString;
	KEYBINDS_REGISTERWITHSTRUCT2		RegisterKeybindWithStruct;
	KEYBINDS_DEREGISTER					DeregisterKeybind;

	/* DataLink */
	DATALINK_GETRESOURCE				GetResource;
	DATALINK_SHARERESOURCE				ShareResource;

	/* Textures */
	TEXTURES_GET						GetTexture;
	TEXTURES_GETORCREATEFROMFILE		GetTextureOrCreateFromFile;
	TEXTURES_GETORCREATEFROMRESOURCE	GetTextureOrCreateFromResource;
	TEXTURES_GETORCREATEFROMURL			GetTextureOrCreateFromURL;
	TEXTURES_GETORCREATEFROMMEMORY		GetTextureOrCreateFromMemory;
	TEXTURES_LOADFROMFILE				LoadTextureFromFile;
	TEXTURES_LOADFROMRESOURCE			LoadTextureFromResource;
	TEXTURES_LOADFROMURL				LoadTextureFromURL;
	TEXTURES_LOADFROMMEMORY				LoadTextureFromMemory;

	/* Shortcuts */
	QUICKACCESS_ADDSHORTCUT				AddShortcut;
	QUICKACCESS_GENERIC					RemoveShortcut;
	QUICKACCESS_GENERIC					NotifyShortcut;
	QUICKACCESS_ADDSIMPLE				AddSimpleShortcut;
	QUICKACCESS_GENERIC					RemoveSimpleShortcut;

	/* Localization */
	LOCALIZATION_TRANSLATE				Translate;
	LOCALIZATION_TRANSLATETO			TranslateTo;
	LOCALIZATION_SET					SetTranslatedString;

	/* Fonts */
	FONTS_GETRELEASE					GetFont;
	FONTS_GETRELEASE					ReleaseFont;
	FONTS_ADDFROMFILE					AddFontFromFile;
	FONTS_ADDFROMRESOURCE				AddFontFromResource;
	FONTS_ADDFROMMEMORY					AddFontFromMemory;

	/* API */
		// GW2 API FUNCS
		// LOGITECH API FUNCS
};


#endif