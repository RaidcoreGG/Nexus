#ifndef ELOGLEVEL_H
#define ELOGLEVEL_H

enum class ELogLevel : unsigned char
{
    OFF         = 0x00,
    CRITICAL    = 0x01,
    WARNING     = 0x02,
    INFO        = 0x04,
    DEBUG       = 0x08,
    TRACE       = 0x10,
    ALL
};

#endif