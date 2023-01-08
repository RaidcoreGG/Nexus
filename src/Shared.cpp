#include "Shared.h"

const wchar_t*	Version			= L"Version: " __DATE__ " " __TIME__;
wchar_t*		CommandLine		= nullptr;
wchar_t			Parameters		[MAX_PATH]{};

LogHandler*		Logger			= LogHandler::GetInstance();
LinkedMem*		MumbleLink		= nullptr;
Minhook			MinhookTable	= {};