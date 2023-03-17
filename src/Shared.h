#ifndef SHARED_H
#define SHARED_H

#include <Windows.h>
#include <vector>
#include <string>

#include "imgui/imgui.h"
#include "Mumble/LinkedMem.h"
#include "Mumble/Identity.h"
#include "Logging/LogHandler.h"

using namespace Mumble;
using namespace LogHandler;

extern HMODULE						AddonHostModule;

extern const char*					Version;
extern std::vector<std::wstring>    Parameters;

extern LinkedMem*                   MumbleLink;
extern Identity*                    MumbleIdentity;
extern bool							IsMoving;
extern bool							IsCameraMoving;
extern bool							IsGameplay;

extern ImFont*						Font;
extern ImFont*						FontBig;
extern ImFont*						FontUI;

#endif