#ifndef UI_SERVICES_QOL_FUNCDEFS_H
#define UI_SERVICES_QOL_FUNCDEFS_H

typedef void (*GUI_REGISTERCLOSEONESCAPE)(const char* aWindowName, bool* aIsVisible);
typedef void (*GUI_DEREGISTERCLOSEONESCAPE)(const char* aWindowName);

#endif
