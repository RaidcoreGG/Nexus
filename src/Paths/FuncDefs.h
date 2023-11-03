#ifndef PATHS_FUNCDEFS_H
#define PATHS_FUNCDEFS_H

typedef const char* (*PATHS_GETGAMEDIR)();
typedef const char* (*PATHS_GETADDONDIR)(const char* aName);
typedef const char* (*PATHS_GETCOMMONDIR)();

#endif