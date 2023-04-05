#include "Shared.h"

HMODULE						AddonHostModule = nullptr;

const char*					Version			= __DATE__ " " __TIME__;
std::vector<std::string>	Parameters		= {};

LinkedMem*					MumbleLink		= nullptr;
Identity*					MumbleIdentity  = new Identity{};
bool						IsMoving		= false;
bool						IsCameraMoving	= false;
bool						IsGameplay		= false;

ImFont*						Font			= nullptr;
ImFont*						FontBig			= nullptr;
ImFont*						FontUI			= nullptr;