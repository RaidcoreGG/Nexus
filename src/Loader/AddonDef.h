#ifndef ADDONDEF_H
#define ADDONDEF_H

#include "HostAPI.h"
#include "EUpdateProvider.h"

#include "../imgui/imgui.h"
#include "../Mumble/Mumble.h"

#include "../Shared.h"

typedef void (*LoadSig)(HostAPI aHostApi, ImGuiContext aImguiContext, Minhook aMinhook, LinkedMem* aMumble);
typedef void (*UnloadSig)();
typedef void (*RenderSig)();
typedef void (*OptionsSig)();

typedef struct AddonDef
{
    signed int      Signature;      /* Raidcore Addon ID, set to -1 if not on Raidcore */
    const wchar_t*  Name;           /* Name of the addon as shown in the library */
    const wchar_t*  Version;        /* Leave as `__DATE__ L" " __TIME__` to maintain consistency */
    const wchar_t*  Author;         /* Author of the addon */
    const wchar_t*  Description;    /* Short description */
    LoadSig         Load;           /* Pointer to Load Function of the addon */
    UnloadSig       Unload;         /* Pointer to Unload Function of the addon */
    RenderSig       Render;         /* Present callback to render imgui */
    OptionsSig      Options;        /* Options window callback, called when opening options for this addon */
    EUpdateProvider Provider;       /* What platform is the the addon hosted on */
    const wchar_t*  UpdateLink;     /* Link to the update resource */
};

#endif