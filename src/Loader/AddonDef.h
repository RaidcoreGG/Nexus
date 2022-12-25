#ifndef ADDONDEF_H
#define ADDONDEF_H

#include "EUpdateProvider.h"

typedef struct AddonDef
{
    signed int      Signature;      /* Raidcore Addon ID, set to -1 if not on Raidcore */
    const wchar_t*  Name;           /* Name of the addon as shown in the library */
    const wchar_t*  Build;          /* Leave as `L"" __DATE__ " " __TIME__` to maintain consistency */
    void*           Load;           /* Pointer to Load Function of the addon */
    void*           Unload;         /* Pointer to Unload Function of the addon */
    void*           WndProc;        /* WndProc callback, only use if necessary, prefer using raidcore keybind registry */
    void*           Present;        /* Present callback to render imgui */
    void*           Options;        /* Options window callback, called when opening options for this addon */
    EUpdateProvider Provider;       /* What platform is the the addon hosted on */
    const wchar_t*  UpdateLink;     /* Link to the update resource */
};

#endif