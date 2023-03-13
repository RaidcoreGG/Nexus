#ifndef LOGGING_FUNCDEFS_H
#define LOGGING_FUNCDEFS_H

#include <string>

#include "ELogLevel.h"
#include "ILogger.h"

typedef void (*LOGGER_LOGA)(ELogLevel aLogLevel, std::string aChannel, const char* aFmt, ...);
typedef void (*LOGGER_ADDREM)(ILogger* aLogger);

#endif