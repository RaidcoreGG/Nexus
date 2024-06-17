#ifndef CORE_H
#define CORE_H

#include <filesystem>
#include <string>
#include <vector>
#include <Windows.h>

#include "Loader/EUpdateProvider.h"

#include "Util/Base64.h"
#include "Util/MD5.h"
#include "Util/Strings.h"
#include "Util/Url.h"

bool FindFunction(HMODULE aModule, LPVOID aFunction, LPCSTR aName);

const char* ConvertToUTF8(const char* multibyteStr);

bool GetResource(HMODULE aModule, LPCSTR aResourceName, LPCSTR aResourceType, LPVOID* aOutLockedResource, DWORD* aOutResourceSize);

#endif
