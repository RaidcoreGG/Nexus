#ifndef INPUTBINDS_FUNCDEFS_H
#define INPUTBINDS_FUNCDEFS_H

#include "InputBind.h"

typedef void (*INPUTBINDS_PROCESS)(const char* aIdentifier);
typedef void (*INPUTBINDS_REGISTERWITHSTRING)(const char* aIdentifier, INPUTBINDS_PROCESS aInputBindHandler, const char* aInputBind);
typedef void (*INPUTBINDS_REGISTERWITHSTRUCT)(const char* aIdentifier, INPUTBINDS_PROCESS aInputBindHandler, LegacyInputBind aInputBind);

typedef void (*INPUTBINDS_PROCESS2)(const char* aIdentifier, bool aIsRelease);
typedef void (*INPUTBINDS_REGISTERWITHSTRING2)(const char* aIdentifier, INPUTBINDS_PROCESS2 aInputBindHandler, const char* aInputBind);
typedef void (*INPUTBINDS_REGISTERWITHSTRUCT2)(const char* aIdentifier, INPUTBINDS_PROCESS2 aInputBindHandler, LegacyInputBind aInputBind);

typedef void (*INPUTBINDS_INVOKE)(const char* aIdentifier, bool aIsRelease);

typedef void (*INPUTBINDS_DEREGISTER)(const char* aIdentifier);

#endif