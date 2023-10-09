#ifndef SHARED_H
#define SHARED_H

#include <Windows.h>
#include <vector>
#include <string>

#include "imgui/imgui.h"
#include "Mumble/LinkedMem.h"
#include "Mumble/Identity.h"
#include "Logging/LogHandler.h"
#include "Loader/AddonDefinition.h"

using namespace Mumble;
using namespace LogHandler;

extern HMODULE						NexusHandle;
extern HMODULE						GameHandle;
extern HMODULE						D3D11Handle;
extern HMODULE						D3D11SystemHandle;

extern AddonVersion*				Version;
extern std::vector<std::string>		Parameters;

extern LinkedMem*                   MumbleLink;
extern Identity*                    MumbleIdentity;
extern bool							IsMoving;
extern bool							IsCameraMoving;
extern bool							IsGameplay;

extern NexusLinkData*				NexusLink;

extern ImFont*						Font;
extern ImFont*						FontBig;
extern ImFont*						FontUI;

extern ImGuiWindowFlags				WindowFlags_Default;
extern ImGuiWindowFlags				WindowFlags_Overlay;
extern ImGuiWindowFlags				WindowFlags_Custom;

#endif