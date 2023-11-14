#ifndef LOGGING_FUNCDEFS_H
#define LOGGING_FUNCDEFS_H

#include "ELogLevel.h"

typedef void (*LOGGER_LOGA)(ELogLevel aLogLevel, const char* aStr);

#endif