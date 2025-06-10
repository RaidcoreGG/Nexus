///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  FontManager.h
/// Description  :  Handles fonts for the ImGui implementation of the UI.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include <mutex>
#include <string>
#include <vector>
#include <Windows.h>

#include "imgui/imgui.h"

#include "FuncDefs.h"
#include "Engine/Localization/LoclApi.h"

constexpr const char* CH_FONTMANAGER = "CFontManager";

///----------------------------------------------------------------------------------------------------
/// ManagedFont_t Struct
///----------------------------------------------------------------------------------------------------
struct ManagedFont_t
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
/// CFontManager Class
///----------------------------------------------------------------------------------------------------
class CFontManager
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CFontManager(CLocalization* aLocalization);
	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CFontManager();

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
	ManagedFont_t* Get(const char* aIdentifier);

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

	private:
	CLocalization* Language;

	mutable std::mutex			Mutex;
	std::vector<ManagedFont_t>	Registry;
	bool						IsFontAtlasBuilt = false;

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
	/// 	Creates a new ManagedFont_t for the Registry.
	///----------------------------------------------------------------------------------------------------
	ManagedFont_t CreateManagedFont(std::string aIdentifier, float aFontSize, void* aData, size_t aSize, ImFontConfig* aConfig);
};

#endif
