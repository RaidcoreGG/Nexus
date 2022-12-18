#ifndef LOGLEVEL_H
#define LOGLEVEL_H

typedef enum class LogLevel : signed char
{
    OFF         = -1,
    CRITICAL    = 0x00,
    WARNING     = 0x01,
    INFO        = 0x02,
    DEBUG       = 0x04,
    TRACE       = 0x08,
    ALL         = CRITICAL | WARNING | INFO | DEBUG | TRACE
} LogLevel_t;

#endif