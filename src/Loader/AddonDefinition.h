#ifndef ADDONDEF_H
#define ADDONDEF_H

#include "AddonAPI.h"
#include "EUpdateProvider.h"

typedef void (*ADDON_LOAD)(AddonAPI aHostApi);
typedef void (*ADDON_UNLOAD)();
typedef void (*ADDON_RENDER)();
typedef void (*ADDON_OPTIONS)();

struct AddonDefinition
{
    /* required */
    signed int      Signature;      /* Raidcore Addon ID, set to random negative integer if not on Raidcore */
    const wchar_t*  Name;           /* Name of the addon as shown in the library */
    const wchar_t*  Version;        /* Leave as `__DATE__ L" " __TIME__` to maintain consistency */
    const wchar_t*  Author;         /* Author of the addon */
    const wchar_t*  Description;    /* Short description */
    ADDON_LOAD      Load;           /* Pointer to Load Function of the addon */
    ADDON_UNLOAD    Unload;         /* Pointer to Unload Function of the addon */

    /* optional */
    ADDON_RENDER    Render;         /* Present callback to render imgui */
    ADDON_OPTIONS   Options;        /* Options window callback, called when opening options for this addon */

    /* update fallback */
    EUpdateProvider Provider;       /* What platform is the the addon hosted on */
    const wchar_t*  UpdateLink;     /* Link to the update resource */

    bool HasMinimumRequirements();
};

#endif