#ifndef GUI_FUNCDEFS_H
#define GUI_FUNCDEFS_H

#include "ERenderType.h"

typedef void (*GUI_RENDER)(bool aIsUIVisible);
typedef void (*GUI_ADDRENDER)(ERenderType aRenderType, GUI_RENDER aRenderCallback);
typedef void (*GUI_REMRENDER)(GUI_RENDER aRenderCallback);

#endif