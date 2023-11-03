#ifndef LOGGING_FUNCDEFS_H
#define LOGGING_FUNCDEFS_H

#include "ELogLevel.h"
#include "ILogger.h"

typedef void (*LOGGER_LOGA)(ELogLevel aLogLevel, const char* aStr);
typedef void (*LOGGER_ADDREM)(ILogger* aLogger);

#endif