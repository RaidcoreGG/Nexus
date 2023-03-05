#ifndef QUICKACCESS_FUNCDEFS_H
#define QUICKACCESS_FUNCDEFS_H

#include <string>

#include "../../../Textures/Texture.h"

typedef void (*QUICKACCESS_SHORTCUTRENDERCALLBACK)	();
typedef void (*QUICKACCESS_ADDSHORTCUT)				(std::string aIdentifier, std::string aTextureIdentifier, std::string aTextureHoverIdentifier, std::string aKeybindIdentifier, std::string aTooltipText);
typedef void (*QUICKACCESS_REMOVESHORTCUT)			(std::string aIdentifier);
typedef void (*QUICKACCESS_ADDSIMPLE)				(std::string aIdentifier, QUICKACCESS_SHORTCUTRENDERCALLBACK aShortcutRenderCallback);
typedef void (*QUICKACCESS_REMOVESIMPLE)			(std::string aIdentifier);

#endif