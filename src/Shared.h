#ifndef SHARED_H
#define SHARED_H

#include <Windows.h>
#include <vector>
#include <string>

#include "Loader/AddonVersion.h"
#include "Services/Logging/LogHandler.h"
#include "Services/API/ApiClient.h"
#include "Services/Localization/Localization.h"
#include "Services/Updater/Updater.h"
#include "Services/Textures/TextureLoader.h"
#include "Services/DataLink/DataLink.h"
#include "Events/EventHandler.h"
#include "Inputs/RawInput/RawInputApi.h"
#include "Inputs/InputBinds/InputBindHandler.h"
#include "Inputs/GameBinds/GameBindsHandler.h"
#include "UI/UiContext.h"

#include "imgui/imgui.h"

extern DWORD						NexusModuleSize;
extern HMODULE						NexusHandle;
extern HMODULE						GameHandle;
extern HMODULE						D3D11Handle;
extern HMODULE						D3D11SystemHandle;

extern AddonVersion					Version;
extern std::vector<std::string>		Parameters;
extern std::vector<signed int>		RequestedAddons;

/* FIXME: these need to be dependency injected. Fix before 2024/06/30. */
extern CLogHandler*					Logger;
extern CLocalization*				Language;
extern CUpdater*					UpdateService;
extern CTextureLoader*				TextureService;
extern CDataLink*					DataLinkService;
extern CEventApi*					EventApi;
extern CRawInputApi*				RawInputApi;
extern CInputBindApi*				InputBindApi;
extern CGameBindsApi*				GameBindsApi;
extern CUiContext*					UIContext;

extern ImGuiWindowFlags				WindowFlags_Default;
extern ImGuiWindowFlags				WindowFlags_Overlay;
extern ImGuiWindowFlags				WindowFlags_Watermark;

extern CApiClient*					RaidcoreAPI;
extern CApiClient*					GitHubAPI;

extern std::string					ChangelogText;
extern bool							IsUpdateAvailable;

extern bool							IsGameLaunchSequence;

#endif