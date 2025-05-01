#ifndef CONSTS_H
#define CONSTS_H

constexpr const char* NULLSTR = "(null)";

/* Options */
constexpr const char* OPT_LASTGAMEBUILD            = "LastGameBuild";
constexpr const char* OPT_ACCEPTEULA               = "AcceptEULA";
constexpr const char* OPT_NOTIFYCHANGELOG          = "NotifyChangelog";
constexpr const char* OPT_CLOSEESCAPE              = "CloseOnEscape";
constexpr const char* OPT_LASTUISCALE              = "LastUIScale";
constexpr const char* OPT_FONTSIZE                 = "FontSize";
constexpr const char* OPT_QAVERTICAL               = "QAVertical";
constexpr const char* OPT_QAVISIBILITY             = "QAVisibility";
constexpr const char* OPT_QALOCATION               = "QALocation";
constexpr const char* OPT_QAOFFSETX                = "QAOffsetX";
constexpr const char* OPT_QAOFFSETY                = "QAOffsetY";
constexpr const char* OPT_QASHOWARCDPS             = "QAShowArcDPS";
constexpr const char* OPT_IMGUISTYLE               = "ImGuiStyle";
constexpr const char* OPT_IMGUICOLORS              = "ImGuiColors";
constexpr const char* OPT_LANGUAGE                 = "Language";
constexpr const char* OPT_GLOBALSCALE              = "GlobalScale";
constexpr const char* OPT_SHOWADDONSWINDOWAFTERDUU = "ShowAddonsWindowAfterDisableUntilUpdate";
constexpr const char* OPT_USERFONT                 = "UserFont";
constexpr const char* OPT_DISABLEFESTIVEFLAIR      = "DisableFestiveFlair";
constexpr const char* OPT_DPISCALING               = "DPIScaling";
constexpr const char* OPT_CAMCTRL_LOCKCURSOR       = "CameraControl_LockCursor";
constexpr const char* OPT_CAMCTRL_RESETCURSOR      = "CameraControl_ResetCursor";

/* Channels */
constexpr const char* CH_CORE = "Core";
constexpr const char* CH_LOADER = "Loader";

/* Events */
constexpr const char* EV_WINDOW_RESIZED = "EV_WINDOW_RESIZED";
constexpr const char* EV_ADDON_LOADED = "EV_ADDON_LOADED";
constexpr const char* EV_ADDON_UNLOADED = "EV_ADDON_UNLOADED";
constexpr const char* EV_VOLATILE_ADDON_DISABLED = "EV_VOLATILE_ADDON_DISABLED";

/* API */
constexpr const char* API_RAIDCORE = "https://api.raidcore.gg";
constexpr const char* API_GITHUB = "https://api.github.com";
constexpr const char* API_GW2 = "https://api.guildwars2.com";

#endif