#ifndef GUI_FUNCDEFS_H
#define GUI_FUNCDEFS_H

typedef void (*ADDON_RENDER)(bool aIsUIVisible);
typedef void (*GUI_REGISTER)(ADDON_RENDER aRenderCallback);
typedef void (*GUI_UNREGISTER)(ADDON_RENDER aRenderCallback);

#endif