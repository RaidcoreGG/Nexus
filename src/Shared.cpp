#include "Shared.h"

const wchar_t*				Version			= __DATE__ L" " __TIME__;
std::vector<std::wstring>	Parameters		= {};

LogHandler*					Logger			= LogHandler::GetInstance();
LinkedMem*					MumbleLink		= nullptr;
Identity*					MumbleIdentity  = nullptr;