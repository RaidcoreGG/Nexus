#include "Shared.h"

HMODULE						AddonHostModule = nullptr;

const char*					Version			= __DATE__ " " __TIME__;
std::vector<std::wstring>	Parameters		= {};

LinkedMem*					MumbleLink		= nullptr;
Identity*					MumbleIdentity  = nullptr;