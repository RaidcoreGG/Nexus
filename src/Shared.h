#ifndef SHARED_H
#define SHARED_H

#include <vector>
#include <string>

#include "Logging/LogHandler.h"
#include "Mumble/Mumble.h"
#include "Mumble/Identity.h"
#include "minhook/mh_hook.h"

using namespace Mumble;
using namespace LogHandler;

extern const wchar_t*               Version;
extern std::vector<std::wstring>    Parameters;

extern LinkedMem*                   MumbleLink;
extern Identity*                    MumbleIdentity;

#endif