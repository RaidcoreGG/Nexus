#ifndef SHARED_H
#define SHARED_H

#include <Windows.h>
#include <vector>
#include <string>

#include "Loader/AddonDefinition.h"
#include "Loader/NexusLinkData.h"
#include "Mumble/LinkedMem.h"
#include "Mumble/Identity.h"
#include "Logging/LogHandler.h"
#include "API/ApiClient.h"
#include "Localization/Localization.h"

#include "imgui.h"

using namespace Mumble;
using namespace LogHandler;

extern DWORD						NexusModuleSize;
extern HMODULE						NexusHandle;
extern HMODULE						GameHandle;
extern HMODULE						D3D11Handle;
extern HMODULE						D3D11SystemHandle;

extern AddonVersion					Version;
extern std::vector<std::string>		Parameters;
extern std::vector<signed int>		RequestedAddons;

extern long long					FrameCounter;

extern std::string					MumbleLinkName;
extern LinkedMem*                   MumbleLink;
extern Identity*                    MumbleIdentity;
extern bool							IsMoving;
extern bool							IsCameraMoving;
extern bool							IsGameplay;

extern NexusLinkData*				NexusLink;

extern CLocalization&				Language;

extern ImFont*						MonospaceFont;	/* default/monospace/console font */
extern ImFont*						UserFont;		/* custom user font */
extern ImFont*						Font;			/* Menomonia */
extern ImFont*						FontBig;		/* Menomonia, but slightly bigger */
extern ImFont*						FontUI;			/* Trebuchet */

extern ImGuiWindowFlags				WindowFlags_Default;
extern ImGuiWindowFlags				WindowFlags_Overlay;
extern ImGuiWindowFlags				WindowFlags_Custom;
extern ImGuiWindowFlags				WindowFlags_Watermark;

extern CApiClient*					RaidcoreAPI;
extern CApiClient*					GitHubAPI;

extern std::string					ChangelogText;
extern bool							IsUpdateAvailable;

extern bool							IsGameLaunchSequence;

#endif