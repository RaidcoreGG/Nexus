#ifndef SHARED_H
#define SHARED_H

#include <Windows.h>
#include <vector>
#include <string>

#include "Loader/AddonDefinition.h"
#include "Logging/LogHandler.h"
#include "API/ApiClient.h"
#include "Localization/Localization.h"
#include "Services/Updater/Updater.h"

#include "imgui.h"

using namespace LogHandler;

extern DWORD						NexusModuleSize;
extern HMODULE						NexusHandle;
extern HMODULE						GameHandle;
extern HMODULE						D3D11Handle;
extern HMODULE						D3D11SystemHandle;

extern AddonVersion					Version;
extern std::vector<std::string>		Parameters;
extern std::vector<signed int>		RequestedAddons;

/* FIXME: these need to be dependency injected. Fix before 2024/06/30. */
extern CLocalization*				Language;
extern CUpdater*					Updater;

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