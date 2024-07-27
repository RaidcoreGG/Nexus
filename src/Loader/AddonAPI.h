#ifndef ADDONAPI_H
#define ADDONAPI_H

#include "imgui/imgui.h"
#include <dxgi.h>

#include "Events/FuncDefs.h"
#include "GUI/Fonts/FuncDefs.h"
#include "GUI/FuncDefs.h"
#include "GUI/Widgets/Alerts/FuncDefs.h"
#include "GUI/Widgets/QuickAccess/FuncDefs.h"
#include "Inputs/GameBinds/FuncDefs.h"
#include "Inputs/Keybinds/FuncDefs.h"
#include "Inputs/RawInput/FuncDefs.h"
#include "minhook/mh_hook.h"
#include "Paths/FuncDefs.h"
#include "Services/DataLink/FuncDefs.h"
#include "Services/Localization/FuncDefs.h"
#include "Services/Logging/FuncDefs.h"
#include "Services/Textures/FuncDefs.h"
#include "Services/Updater/FuncDefs.h"

// Base
struct AddonAPI {};

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
};

struct AddonAPI6 : AddonAPI
{
	/* Renderer */
	IDXGISwapChain*							SwapChain;
	ImGuiContext*							ImguiContext;
	void*									ImguiMalloc;
	void*									ImguiFree;

	struct RendererVT
	{
		GUI_ADDRENDER						Register;
		GUI_REMRENDER						Deregister;
	};
	RendererVT								Renderer;

	/* Updater */
	UPDATER_REQUESTUPDATE					RequestUpdate;

	/* Logging */
	LOGGER_LOG2								Log;

	/* User Interface */
	struct UIVT
	{
		ALERTS_NOTIFY						SendAlert;
		GUI_REGISTERCLOSEONESCAPE			RegisterCloseOnEscape;
		GUI_DEREGISTERCLOSEONESCAPE			DeregisterCloseOnEscape;
	};
	UIVT									UI;

	/* Paths */
	struct PathsVT
	{
		PATHS_GETGAMEDIR					GetGameDirectory;
		PATHS_GETADDONDIR					GetAddonDirectory;
		PATHS_GETCOMMONDIR					GetCommonDirectory;
	};
	PathsVT									Paths;

	/* Minhook */
	struct MinHookVT
	{
		MINHOOK_CREATE						Create;
		MINHOOK_REMOVE						Remove;
		MINHOOK_ENABLE						Enable;
		MINHOOK_DISABLE						Disable;
	};
	MinHookVT								MinHook;

	/* Events */
	struct EventsVT
	{
		EVENTS_RAISE						Raise;
		EVENTS_RAISENOTIFICATION			RaiseNotification;
		EVENTS_RAISE_TARGETED				RaiseTargeted;
		EVENTS_RAISENOTIFICATION_TARGETED	RaiseNotificationTargeted;
		EVENTS_SUBSCRIBE					Subscribe;
		EVENTS_SUBSCRIBE					Unsubscribe;
	};
	EventsVT								Events;

	/* WndProc */
	struct WndProcVT
	{
		WNDPROC_ADDREM						Register;
		WNDPROC_ADDREM						Deregister;
		WNDPROC_SENDTOGAME					SendToGameOnly;
	};
	WndProcVT								WndProc;

	/* InputBinds */
	struct InputBindsVT
	{
		KEYBINDS_INVOKE						Invoke;
		KEYBINDS_REGISTERWITHSTRING2		RegisterWithString;
		KEYBINDS_REGISTERWITHSTRUCT2		RegisterWithStruct;
		KEYBINDS_DEREGISTER					Deregister;
	};
	InputBindsVT							InputBinds;

	/* GameBinds */
	struct GameBindsVT
	{
		GAMEBINDS_PRESSASYNC				PressAsync;
		GAMEBINDS_RELEASEASYNC				ReleaseAsync;
		GAMEBINDS_INVOKEASYNC				InvokeAsync;
		GAMEBINDS_PRESS						Press;
		GAMEBINDS_RELEASE					Release;
		GAMEBINDS_ISBOUND					IsBound;
	};
	GameBindsVT								GameBinds;

	/* DataLink */
	struct DataLinkVT
	{
		DATALINK_GETRESOURCE				Get;
		DATALINK_SHARERESOURCE				Share;
	};
	DataLinkVT								DataLink;

	/* Textures */
	struct TexturesVT
	{
		TEXTURES_GET						Get;
		TEXTURES_GETORCREATEFROMFILE		GetOrCreateFromFile;
		TEXTURES_GETORCREATEFROMRESOURCE	GetOrCreateFromResource;
		TEXTURES_GETORCREATEFROMURL			GetOrCreateFromURL;
		TEXTURES_GETORCREATEFROMMEMORY		GetOrCreateFromMemory;
		TEXTURES_LOADFROMFILE				LoadFromFile;
		TEXTURES_LOADFROMRESOURCE			LoadFromResource;
		TEXTURES_LOADFROMURL				LoadFromURL;
		TEXTURES_LOADFROMMEMORY				LoadFromMemory;
	};
	TexturesVT								Textures;

	/* Shortcuts */
	struct QuickAccessVT
	{
		QUICKACCESS_ADDSHORTCUT				Add;
		QUICKACCESS_GENERIC					Remove;
		QUICKACCESS_GENERIC					Notify;
		QUICKACCESS_ADDSIMPLE2				AddContextMenu;
		QUICKACCESS_GENERIC					RemoveContextMenu;
	};
	QuickAccessVT							QuickAccess;

	/* Localization */
	struct LocalizationVT
	{
		LOCALIZATION_TRANSLATE				Translate;
		LOCALIZATION_TRANSLATETO			TranslateTo;
		LOCALIZATION_SET					SetTranslatedString;
	};
	LocalizationVT							Localization;

	/* Fonts */
	struct FontsVT
	{
		FONTS_GETRELEASE					Get;
		FONTS_GETRELEASE					Release;
		FONTS_ADDFROMFILE					AddFromFile;
		FONTS_ADDFROMRESOURCE				AddFromResource;
		FONTS_ADDFROMMEMORY					AddFromMemory;
		FONTS_RESIZE						Resize;
	};
	FontsVT									Fonts;
};

#endif
