///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Resources.inc
/// Description  :  Single list of embedded files, included by Nexus.rc and Resources.h.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

/* X-macro list: no include guard, every includer defines PACK_FILE(name, type, path) before including. */
/* Paths are relative to the project directory. Names are case-insensitive and must be unique. */
/* Type is an EPackFileType category, not a file format. */

PACK_FILE(ThirdPartyNotices,         Text,  "THIRDPARTYSOFTWAREREADME.TXT")

/* Seasonal */
PACK_FILE(IconSnowflake,             Image, "res\\Images\\MainWindow\\SnowflakeShaded.png")
PACK_FILE(IconLeafVariant1,          Image, "res\\Images\\MainWindow\\LeafShaded_Variant1.png")
PACK_FILE(IconLeafVariant2,          Image, "res\\Images\\MainWindow\\LeafShaded_Variant2.png")
PACK_FILE(IconLeafVariant3,          Image, "res\\Images\\MainWindow\\LeafShaded_Variant3.png")

/* Fonts */
PACK_FILE(FontMenomonia,             Font,  "res\\Fonts\\Menomonia.ttf")
PACK_FILE(FontFiraSans,              Font,  "res\\Fonts\\FiraSans.ttf")
PACK_FILE(FontInter,                 Font,  "res\\Fonts\\Inter.ttf")

/* QuickAccess */
PACK_FILE(IconNexus,                 Image, "res\\Images\\QuickAccess\\Nexus.png")
PACK_FILE(IconNexusHover,            Image, "res\\Images\\QuickAccess\\Nexus_Hover.png")
PACK_FILE(IconNexusHalloween,        Image, "res\\Images\\QuickAccess\\NexusHalloween.png")
PACK_FILE(IconNexusHalloweenHover,   Image, "res\\Images\\QuickAccess\\NexusHalloween_Hover.png")
PACK_FILE(IconNexusXmas,             Image, "res\\Images\\QuickAccess\\NexusXmas.png")
PACK_FILE(IconNexusXmasHover,        Image, "res\\Images\\QuickAccess\\NexusXmas_Hover.png")

PACK_FILE(IconNotification1,         Image, "res\\Images\\QuickAccess\\Notification1.png")
PACK_FILE(IconNotification2,         Image, "res\\Images\\QuickAccess\\Notification2.png")
PACK_FILE(IconNotification3,         Image, "res\\Images\\QuickAccess\\Notification3.png")
PACK_FILE(IconNotification4,         Image, "res\\Images\\QuickAccess\\Notification4.png")
PACK_FILE(IconNotification5,         Image, "res\\Images\\QuickAccess\\Notification5.png")
PACK_FILE(IconNotification6,         Image, "res\\Images\\QuickAccess\\Notification6.png")
PACK_FILE(IconNotification7,         Image, "res\\Images\\QuickAccess\\Notification7.png")
PACK_FILE(IconNotification8,         Image, "res\\Images\\QuickAccess\\Notification8.png")
PACK_FILE(IconNotification9,         Image, "res\\Images\\QuickAccess\\Notification9.png")
PACK_FILE(IconNotificationX,         Image, "res\\Images\\QuickAccess\\NotificationTooMany.png")
PACK_FILE(IconContextMenuAvailable,  Image, "res\\Images\\QuickAccess\\ContextMenu.png")

/* MainWindow */
PACK_FILE(IconAddons,                Image, "res\\Images\\MainWindow\\Pages\\AddonsShaded.png")
PACK_FILE(IconOptions,               Image, "res\\Images\\MainWindow\\Pages\\SettingsShaded.png")
PACK_FILE(IconBinds,                 Image, "res\\Images\\MainWindow\\Pages\\BindsShaded.png")
PACK_FILE(IconLog,                   Image, "res\\Images\\MainWindow\\Pages\\LogShaded.png")
PACK_FILE(IconDebug,                 Image, "res\\Images\\MainWindow\\Pages\\DebugShaded.png")
PACK_FILE(IconDashboard,             Image, "res\\Images\\MainWindow\\Pages\\HomeShaded.png")
PACK_FILE(IconAbout,                 Image, "res\\Images\\MainWindow\\Pages\\AboutShaded.png")

/* Addon Policy Tiers */
PACK_FILE(IconTierUnknown,           Image, "res\\Images\\MainWindow\\Pages\\Addons\\PolicyUnknown.png")
PACK_FILE(IconTier1,                 Image, "res\\Images\\MainWindow\\Pages\\Addons\\PolicyTier1.png")
PACK_FILE(IconTier2,                 Image, "res\\Images\\MainWindow\\Pages\\Addons\\PolicyTier2.png")
PACK_FILE(IconTier3,                 Image, "res\\Images\\MainWindow\\Pages\\Addons\\PolicyTier3.png")

/* Generic */
PACK_FILE(IconRaidcore,              Image, "res\\Images\\MainWindow\\RaidcoreTag.png")
PACK_FILE(IconPopout,                Image, "res\\Images\\MainWindow\\ExternalShaded.png")
PACK_FILE(IconClose,                 Image, "res\\Images\\MainWindow\\CloseShaded.png")
PACK_FILE(IconChevronRt,             Image, "res\\Images\\MainWindow\\ChevronRtShaded.png")
PACK_FILE(IconList,                  Image, "res\\Images\\MainWindow\\ListShaded.png")
PACK_FILE(IconTiles,                 Image, "res\\Images\\MainWindow\\TilesShaded.png")
PACK_FILE(IconCanFavorite,           Image, "res\\Images\\MainWindow\\CanFavoriteShaded.png")
PACK_FILE(IconFavorite,              Image, "res\\Images\\MainWindow\\FavoriteShaded.png")
PACK_FILE(IconRefresh,               Image, "res\\Images\\MainWindow\\RefreshShaded.png")
PACK_FILE(IconFilter,                Image, "res\\Images\\MainWindow\\FilterShaded.png")
PACK_FILE(IconUpdate,                Image, "res\\Images\\MainWindow\\UpdateShaded.png")
PACK_FILE(IconGitHub,                Image, "res\\Images\\MainWindow\\GitHubShaded.png")
PACK_FILE(BannerDiscord,             Image, "res\\Images\\Social\\Banners\\Discord.png")

/* Locales (primary) */
PACK_FILE(LocaleDisplayStringsEN,    Text,  "res\\Locales\\en\\DisplayStrings.json")
PACK_FILE(LocaleGameBindsEN,         Text,  "res\\Locales\\en\\GameBinds.json")
PACK_FILE(LocaleInputBindsEN,        Text,  "res\\Locales\\en\\InputBinds.json")
PACK_FILE(LocaleDisplayStringsDE,    Text,  "res\\Locales\\de\\DisplayStrings.json")
PACK_FILE(LocaleGameBindsDE,         Text,  "res\\Locales\\de\\GameBinds.json")
PACK_FILE(LocaleInputBindsDE,        Text,  "res\\Locales\\de\\InputBinds.json")
PACK_FILE(LocaleDisplayStringsFR,    Text,  "res\\Locales\\fr\\DisplayStrings.json")
PACK_FILE(LocaleGameBindsFR,         Text,  "res\\Locales\\fr\\GameBinds.json")
PACK_FILE(LocaleInputBindsFR,        Text,  "res\\Locales\\fr\\InputBinds.json")
PACK_FILE(LocaleDisplayStringsES,    Text,  "res\\Locales\\es\\DisplayStrings.json")
PACK_FILE(LocaleGameBindsES,         Text,  "res\\Locales\\es\\GameBinds.json")
PACK_FILE(LocaleInputBindsES,        Text,  "res\\Locales\\es\\InputBinds.json")
PACK_FILE(LocaleDisplayStringsCN,    Text,  "res\\Locales\\cn\\DisplayStrings.json")
PACK_FILE(LocaleGameBindsCN,         Text,  "res\\Locales\\cn\\GameBinds.json")
PACK_FILE(LocaleInputBindsCN,        Text,  "res\\Locales\\cn\\InputBinds.json")
PACK_FILE(LocaleDisplayStringsKR,    Text,  "res\\Locales\\kr\\DisplayStrings.json")
PACK_FILE(LocaleGameBindsKR,         Text,  "res\\Locales\\kr\\GameBinds.json")
PACK_FILE(LocaleInputBindsKR,        Text,  "res\\Locales\\kr\\InputBinds.json")

/* Locales (secondary) */
PACK_FILE(LocaleDisplayStringsBR,    Text,  "res\\Locales\\br\\DisplayStrings.json")
PACK_FILE(LocaleGameBindsBR,         Text,  "res\\Locales\\br\\GameBinds.json")
PACK_FILE(LocaleInputBindsBR,        Text,  "res\\Locales\\br\\InputBinds.json")
PACK_FILE(LocaleDisplayStringsCZ,    Text,  "res\\Locales\\cz\\DisplayStrings.json")
PACK_FILE(LocaleGameBindsCZ,         Text,  "res\\Locales\\cz\\GameBinds.json")
PACK_FILE(LocaleInputBindsCZ,        Text,  "res\\Locales\\cz\\InputBinds.json")
PACK_FILE(LocaleDisplayStringsIT,    Text,  "res\\Locales\\it\\DisplayStrings.json")
PACK_FILE(LocaleGameBindsIT,         Text,  "res\\Locales\\it\\GameBinds.json")
PACK_FILE(LocaleInputBindsIT,        Text,  "res\\Locales\\it\\InputBinds.json")
PACK_FILE(LocaleDisplayStringsPL,    Text,  "res\\Locales\\pl\\DisplayStrings.json")
PACK_FILE(LocaleGameBindsPL,         Text,  "res\\Locales\\pl\\GameBinds.json")
PACK_FILE(LocaleInputBindsPL,        Text,  "res\\Locales\\pl\\InputBinds.json")
PACK_FILE(LocaleDisplayStringsRU,    Text,  "res\\Locales\\ru\\DisplayStrings.json")
PACK_FILE(LocaleGameBindsRU,         Text,  "res\\Locales\\ru\\GameBinds.json")
PACK_FILE(LocaleInputBindsRU,        Text,  "res\\Locales\\ru\\InputBinds.json")
