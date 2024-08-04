#ifndef GAMEBINDS_FUNCDEFS_H
#define GAMEBINDS_FUNCDEFS_H

#include "EGameBinds.h"

typedef void (*GAMEBINDS_PRESSASYNC)(EGameBinds aGameBind);
typedef void (*GAMEBINDS_RELEASEASYNC)(EGameBinds aGameBind);
typedef void (*GAMEBINDS_INVOKEASYNC)(EGameBinds aGameBind, int aDuration);
typedef void (*GAMEBINDS_PRESS)(EGameBinds aGameBind);
typedef void (*GAMEBINDS_RELEASE)(EGameBinds aGameBind);
typedef bool (*GAMEBINDS_ISBOUND)(EGameBinds aGameBind);

#endif