#ifndef ADDONDEF_H
#define ADDONDEF_H

#include "AddonAPI.h"
#include "EUpdateProvider.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_extensions.h"

typedef void (*ADDON_LOAD)(AddonAPI aHostApi);
typedef void (*ADDON_UNLOAD)();
typedef void (*ADDON_RENDER)();
typedef void (*ADDON_OPTIONS)();

struct AddonDefinition
{
    /* required */
    signed int      Signature;      /* Raidcore Addon ID, set to random negative integer if not on Raidcore */
    const char*     Name;           /* Name of the addon as shown in the library */
    const char*     Version;        /* Leave as `__DATE__ L" " __TIME__` to maintain consistency */
    const char*     Author;         /* Author of the addon */
    const char*     Description;    /* Short description */
    ADDON_LOAD      Load;           /* Pointer to Load Function of the addon */
    ADDON_UNLOAD    Unload;         /* Pointer to Unload Function of the addon */

    /* optional */
    ADDON_RENDER    Render;         /* Present callback to render imgui */
    ADDON_OPTIONS   Options;        /* Options window callback, called when opening options for this addon */

    /* update fallback */
    EUpdateProvider Provider;       /* What platform is the the addon hosted on */
    const char*     UpdateLink;     /* Link to the update resource */

    bool HasMinimumRequirements();
    void RenderItem();
};

#endif