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

extern CApiClient*					RaidcoreAPI;
extern CApiClient*					GitHubAPI;

extern std::string					ChangelogText;
extern bool							IsUpdateAvailable;

extern bool							IsGameLaunchSequence;

#endif