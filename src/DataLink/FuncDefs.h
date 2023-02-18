#ifndef DATALINK_FUNCDEFS_H
#define DATALINK_FUNCDEFS_H

#include <string>

typedef void* (*DATALINK_GETRESOURCE)(std::string aIdentifier);
typedef void* (*DATALINK_SHARERESOURCE)(std::string aIdentifier, size_t aResourceSize);

#endif