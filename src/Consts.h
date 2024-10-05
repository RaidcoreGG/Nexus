#ifndef CONSTS_H
#define CONSTS_H

#include <Windows.h>

constexpr const char* NULLSTR = "(null)";

/* Channels */
extern const char* CH_CORE;
extern const char* CH_LOADER;

/* Textures & Icons */
extern const char* ICON_NOTIFICATION;
extern const char* ICON_WARNING;
extern const char* ICON_RETURN;
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

/* Events */
extern const char* EV_WINDOW_RESIZED;
extern const char* EV_MUMBLE_IDENTITY_UPDATED;
extern const char* EV_ADDON_LOADED;
extern const char* EV_ADDON_UNLOADED;
extern const char* EV_VOLATILE_ADDON_DISABLED;

/* Loader */
extern const UINT WM_ADDONDIRUPDATE;

/* API */
extern const char* API_RAIDCORE;
extern const char* API_GITHUB;
extern const char* API_GW2;

#endif