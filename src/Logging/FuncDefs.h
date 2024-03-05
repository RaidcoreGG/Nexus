#ifndef LOGGING_FUNCDEFS_H
#define LOGGING_FUNCDEFS_H

#include "ELogLevel.h"

typedef void (*LOGGER_LOG)(ELogLevel aLogLevel, const char* aStr);
typedef void (*LOGGER_LOG2)(ELogLevel aLogLevel, const char* aChannel, const char* aStr);

#endif