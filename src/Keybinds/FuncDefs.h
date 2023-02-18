#ifndef KEYBINDS_FUNCDEFS_H
#define KEYBINDS_FUNCDEFS_H

#include <string>

typedef void (*KEYBINDS_PROCESS)(std::string aIdentifier);
typedef void (*KEYBINDS_REGISTER)(std::string aIdentifier, KEYBINDS_PROCESS aKeybindHandler, std::string aKeybind);
typedef void (*KEYBINDS_UNREGISTER)(std::string aIdentifier);

#endif