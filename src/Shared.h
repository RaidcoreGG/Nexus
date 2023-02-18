#ifndef SHARED_H
#define SHARED_H

#include <vector>
#include <string>

#include "Mumble/LinkedMem.h"
#include "Mumble/Identity.h"
#include "Logging/LogHandler.h"

using namespace Mumble;
using namespace LogHandler;

extern const char*					Version;
extern std::vector<std::wstring>    Parameters;

extern LinkedMem*                   MumbleLink;
extern Identity*                    MumbleIdentity;

#endif