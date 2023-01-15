#ifndef SHARED_H
#define SHARED_H

#include "Logging/LogHandler.h"
#include "Mumble/Mumble.h"
#include "Mumble/Identity.h"
#include "minhook/mh_hook.h"

using namespace Mumble;

typedef MH_STATUS(WINAPI* MHCreateSig)(LPVOID pTarget, LPVOID pDetour, LPVOID *ppOriginal);
typedef MH_STATUS(WINAPI* MHRemoveSig)(LPVOID pTarget);
typedef MH_STATUS(WINAPI* MHEnableSig)(LPVOID pTarget);
typedef MH_STATUS(WINAPI* MHDisableSig)(LPVOID pTarget);

struct Minhook
{
    MHCreateSig     CreateHook;
    MHRemoveSig     RemoveHook;
    MHEnableSig     EnableHook;
    MHDisableSig    DisableHook;
};

extern const wchar_t*   Version;
extern wchar_t*         CommandLine;
extern wchar_t          Parameters[];

extern LogHandler*	    Logger;
extern LinkedMem*       MumbleLink;
extern Identity*        MumbleIdentity;
extern Minhook          MinhookTable;

#endif