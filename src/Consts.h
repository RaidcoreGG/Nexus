#ifndef CONSTS_H
#define CONSTS_H

#include <Windows.h>

/* UI Scale */
extern const float SC_SMALL;
extern const float SC_NORMAL;
extern const float SC_LARGE;
extern const float SC_LARGER;

/* Channels */
extern const char* CH_CORE;
extern const char* CH_EVENTS;
extern const char* CH_TEXTURES;
extern const char* CH_QUICKACCESS;
extern const char* CH_LOADER;
extern const char* CH_DATALINK;
extern const char* CH_KEYBINDS;
extern const char* CH_LOCALIZATION;

/* Keybinds */
extern const char* KB_MENU;
extern const char* KB_ADDONS;
extern const char* KB_OPTIONS;
extern const char* KB_CHANGELOG;
extern const char* KB_LOG;
extern const char* KB_DEBUG;
extern const char* KB_MUMBLEOVERLAY;
extern const char* KB_TOGGLEHIDEUI;

/* Textures & Icons */
extern const char* ICON_NEXUS;
extern const char* ICON_NEXUS_HOVER;
extern const char* ICON_GENERIC;
extern const char* ICON_GENERIC_HOVER;
extern const char* ICON_NOTIFICATION;
extern const char* ICON_ADDONS;
extern const char* ICON_OPTIONS;
extern const char* ICON_OPTIONS_HOVER;
extern const char* ICON_CHANGELOG;
extern const char* ICON_LOG;
extern const char* ICON_DEBUG;
extern const char* ICON_ABOUT;
extern const char* TEX_MENU_BACKGROUND;
extern const char* TEX_MENU_BUTTON;
extern const char* TEX_MENU_BUTTON_HOVER;

/* Quick Access*/
extern const char* QA_MENU;

/* Events */
extern const char* EV_WINDOW_RESIZED;
extern const char* EV_MUMBLE_IDENTITY_UPDATED;
extern const char* EV_ADDON_LOADED;
extern const char* EV_ADDON_UNLOADED;
extern const char* EV_VOLATILE_ADDONS_DISABLED;

/* DataLink */
extern const char* DL_MUMBLE_LINK;
extern const char* DL_NEXUS_LINK;

/* Loader */
extern const UINT WM_ADDONDIRUPDATE;

/* API */
extern const char* API_RAIDCORE;
extern const char* API_GITHUB;
extern const char* API_GW2;

#endif