#ifndef SHARED_H
#define SHARED_H

#include <Windows.h>
#include <vector>
#include <string>

#include "Engine/Loader/AddonVersion.h"
#include "Engine/Logging/LogApi.h"
#include "Engine/Networking/WebRequests/WreClient.h"
#include "UI/Services/Localization/LoclApi.h"
#include "Engine/Updater/Updater.h"
#include "Engine/Textures/TxLoader.h"
#include "Engine/DataLink/DlApi.h"
#include "Engine/Events/EvtApi.h"
#include "Engine/Inputs/RawInput/RiApi.h"
#include "Engine/Inputs/InputBinds/IbApi.h"
#include "GW2/Inputs/GameBinds/GbApi.h"
#include "UI/UiContext.h"

#include "imgui/imgui.h"

extern bool							IsGameLaunchSequence;

#endif