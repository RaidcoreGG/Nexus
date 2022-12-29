#include "core.h"

BOOL FindFunction(HMODULE mod, LPVOID func, LPCSTR name)
{
	FARPROC* fp = (FARPROC*)func;
	*fp = mod ? GetProcAddress(mod, name) : 0;
	return (fp != 0);
}

std::string WStrToStr(std::wstring& wide_string)
{
	static std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
	return utf8_conv.to_bytes(wide_string);
}