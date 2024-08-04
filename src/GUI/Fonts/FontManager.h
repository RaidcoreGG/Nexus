///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  FontManager.h
/// Description  :  Handles fonts for the ImGui implementation of the GUI.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include <Windows.h>
#include <mutex>
#include <string>
#include <vector>

#include "FuncDefs.h"

#include "imgui/imgui.h"

constexpr const char* CH_FONTMANAGER = "CFontManager";

///----------------------------------------------------------------------------------------------------
/// ManagedFont Struct
///----------------------------------------------------------------------------------------------------
struct ManagedFont
{
	std::string							Identifier;
	ImFont*								Pointer;
	std::vector<FONTS_RECEIVECALLBACK>	Subscribers;
	float								Size;
	void*								Data;
	size_t								DataSize;
	ImFontConfig*						Config;
};

///----------------------------------------------------------------------------------------------------
/// FontManager Namespace
///----------------------------------------------------------------------------------------------------
namespace FontManager
{
	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_Get:
	/// 	Addon API wrapper to get a font sent to the callback.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_Get(const char* aIdentifier, FONTS_RECEIVECALLBACK aCallback);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_Get2:
	/// 	Addon API wrapper to get a font sent to the callback.
	///----------------------------------------------------------------------------------------------------
	ImFont* ADDONAPI_Get2(const char* aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_Release:
	/// 	Addon API wrapper to release a receiver/callback from a specific font.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_Release(const char* aIdentifier, FONTS_RECEIVECALLBACK aCallback);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_AddFontFromFile:
	/// 	Addon API wrapper to add a font from file.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_AddFontFromFile(const char* aIdentifier, float aFontSize, const char* aFilename, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_AddFontFromResource:
	/// 	Addon API wrapper to add a font from an embedded resource.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_AddFontFromResource(const char* aIdentifier, float aFontSize, unsigned aResourceID, HMODULE aModule, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_AddFontFromMemory:
	/// 	Addon API wrapper to add a font from memory.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_AddFontFromMemory(const char* aIdentifier, float aFontSize, void* aData, size_t aSize, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_ResizeFont:
	/// 	Changes the size of a given font.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_ResizeFont(const char* aIdentifier, float aFontSize);
}

///----------------------------------------------------------------------------------------------------
/// CFontManager Singleton
///----------------------------------------------------------------------------------------------------
class CFontManager
{
public:
	///----------------------------------------------------------------------------------------------------
	/// GetInstance:
	/// 	Returns a reference to an instance of CFontManager.
	///----------------------------------------------------------------------------------------------------
	static CFontManager& GetInstance();

	///----------------------------------------------------------------------------------------------------
	/// Reload:
	/// 	Forces the font atlas to rebuild.
	///----------------------------------------------------------------------------------------------------
	void Reload();

	///----------------------------------------------------------------------------------------------------
	/// Advance:
	/// 	Processes fonts and notifies callbacks.
	///----------------------------------------------------------------------------------------------------
	bool Advance();

	///----------------------------------------------------------------------------------------------------
	/// Get:
	/// 	Returns a font if it exists or nullptr.
	///----------------------------------------------------------------------------------------------------
	ManagedFont* Get(const char* aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// Release:
	/// 	Releases a callback/receiver from a specific font.
	///----------------------------------------------------------------------------------------------------
	void Release(const char* aIdentifier, FONTS_RECEIVECALLBACK aCallback);

	///----------------------------------------------------------------------------------------------------
	/// AddFont:
	/// 	Adds a font from disk and sends updates to the callback.
	///----------------------------------------------------------------------------------------------------
	void AddFont(const char* aIdentifier, float aFontSize, const char* aFilename, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig);

	///----------------------------------------------------------------------------------------------------
	/// AddFont:
	/// 	Adds a font from an embedded resource and sends updates to the callback.
	///----------------------------------------------------------------------------------------------------
	void AddFont(const char* aIdentifier, float aFontSize, unsigned aResourceID, HMODULE aModule, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig);

	///----------------------------------------------------------------------------------------------------
	/// AddFont:
	/// 	Adds a font from memory and sends updates to the callback.
	///----------------------------------------------------------------------------------------------------
	void AddFont(const char* aIdentifier, float aFontSize, void* aData, size_t aSize, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig);

	///----------------------------------------------------------------------------------------------------
	/// ReplaceFont:
	/// 	Replaces a font from disk and sends updates to the callback.
	///----------------------------------------------------------------------------------------------------
	void ReplaceFont(const char* aIdentifier, float aFontSize, const char* aFilename, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig);

	///----------------------------------------------------------------------------------------------------
	/// ReplaceFont:
	/// 	Replaces a font from an embedded resource and sends updates to the callback.
	///----------------------------------------------------------------------------------------------------
	void ReplaceFont(const char* aIdentifier, float aFontSize, unsigned aResourceID, HMODULE aModule, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig);

	///----------------------------------------------------------------------------------------------------
	/// ReplaceFont:
	/// 	Replaces a font from memory and sends updates to the callback.
	///----------------------------------------------------------------------------------------------------
	void ReplaceFont(const char* aIdentifier, float aFontSize, void* aData, size_t aSize, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig);

	///----------------------------------------------------------------------------------------------------
	/// AddDefaultFont:
	/// 	Adds a the default ImGui font.
	///----------------------------------------------------------------------------------------------------
	void AddDefaultFont(FONTS_RECEIVECALLBACK aCallback = nullptr);

	///----------------------------------------------------------------------------------------------------
	/// ResizeFont:
	/// 	Changes the size of a given font.
	///----------------------------------------------------------------------------------------------------
	void ResizeFont(const char* aIdentifier, float aFontSize);

	///----------------------------------------------------------------------------------------------------
	/// Verify:
	/// 	Removes all unreleased references in the given address space.
	///----------------------------------------------------------------------------------------------------
	int Verify(void* aStartAddress, void* aEndAddress);

	CFontManager(CFontManager const&) = delete;
	void operator=(CFontManager const&) = delete;

private:
	std::mutex					Mutex;
	std::vector<ManagedFont>	Registry;
	bool						IsFontAtlasBuilt = false;

	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CFontManager() = default;
	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CFontManager() = default;

	///----------------------------------------------------------------------------------------------------
	/// AddFontInternal:
	/// 	Adds a font from memory and registers the callback.
	///----------------------------------------------------------------------------------------------------
	void AddFontInternal(const char* aIdentifier, float aFontSize, void* aData, size_t aSize, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig);

	///----------------------------------------------------------------------------------------------------
	/// NotifyCallbacks:
	/// 	Notifies all callbacks with the new font.
	/// 	aNotifyNull will send a nullptr, effectively "clearing" the subscriber.
	///----------------------------------------------------------------------------------------------------
	void NotifyCallbacks(bool aNotifyNull = false);

	///----------------------------------------------------------------------------------------------------
	/// CreateManagedFont:
	/// 	Creates a new ManagedFont for the Registry.
	///----------------------------------------------------------------------------------------------------
	ManagedFont CreateManagedFont(std::string aIdentifier, float aFontSize, void* aData, size_t aSize, ImFontConfig* aConfig);
};

#endif
