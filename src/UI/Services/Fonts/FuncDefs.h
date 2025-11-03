#pragma once

#include <Windows.h>

#include "imgui/imgui.h"

typedef void (*FONTS_RECEIVECALLBACK)(const char* aIdentifier, ImFont* aFont);
typedef void (*FONTS_GETRELEASE)(const char* aIdentifier, FONTS_RECEIVECALLBACK aCallback);
typedef void (*FONTS_ADDFROMFILE)(const char* aIdentifier, float aFontSize, const char* aFilename, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig);
typedef void (*FONTS_ADDFROMRESOURCE)(const char* aIdentifier, float aFontSize, unsigned aResourceID, HMODULE aModule, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig);
typedef void (*FONTS_ADDFROMMEMORY)(const char* aIdentifier, float aFontSize, void* aData, size_t aSize, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig);
typedef void (*FONTS_RESIZE)(const char* aIdentifier, float aFontSize);
