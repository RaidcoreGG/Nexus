#ifndef DATALINK_FUNCDEFS_H
#define DATALINK_FUNCDEFS_H

#include <string>

typedef void* (*DATALINK_GETRESOURCE)(const char* aIdentifier);
typedef void* (*DATALINK_SHARERESOURCE)(const char* aIdentifier, size_t aResourceSize);

#endif