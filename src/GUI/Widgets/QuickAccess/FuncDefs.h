#ifndef QUICKACCESS_FUNCDEFS_H
#define QUICKACCESS_FUNCDEFS_H

#include <string>

#include "../../../Textures/Texture.h"

typedef void (*QUICKACCESS_SHORTCUTRENDERCALLBACK)	();
typedef void (*QUICKACCESS_ADDSHORTCUT)				(const char* aIdentifier, const char* aTextureIdentifier, const char* aTextureHoverIdentifier, const char* aKeybindIdentifier, const char* aTooltipText);
typedef void (*QUICKACCESS_REMOVESHORTCUT)			(const char* aIdentifier);
typedef void (*QUICKACCESS_ADDSIMPLE)				(const char* aIdentifier, QUICKACCESS_SHORTCUTRENDERCALLBACK aShortcutRenderCallback);
typedef void (*QUICKACCESS_REMOVESIMPLE)			(const char* aIdentifier);

#endif