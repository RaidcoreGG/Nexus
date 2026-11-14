///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  FontManager.h
/// Description  :  Handles fonts for the ImGui implementation of the UI.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <chrono>
#include <d3d11.h>
#include <filesystem>
#include <future>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <vector>
#include <Windows.h>

#include "imgui/imgui.h"

#include "Core/Logging/LogChannel.h"
#include "Core/Platform/Windows/Exe/ExePackFile.h"
#include "Core/Preferences/PrefSetting.h"
#include "Engine/Cleanup/RefCleanerBase.h"
#include "FontNotify.h"

typedef void (*FONTS_RECEIVECALLBACK)(const char* aIdentifier, ImFont* aFont);

///----------------------------------------------------------------------------------------------------
/// ManagedFont_t Struct
///----------------------------------------------------------------------------------------------------
struct ManagedFont_t
{
	std::string                                 Identifier;
	ImFont*                                     Pointer;
	std::vector<FONTS_RECEIVECALLBACK>          Subscribers;
	float                                       Size;
	std::shared_ptr<const std::vector<uint8_t>> Data;        /* Shared with the atlas builds that use it. */
	ImFontConfig*                               Config;
	std::shared_ptr<const std::vector<ImWchar>> GlyphRanges; /* Declared by an addon. Null for Nexus fonts. */
	bool                                        IsHostFont;  /* Added by Nexus, kept without subscribers. */
};

///----------------------------------------------------------------------------------------------------
/// FontAtlas_t Struct
/// 	One atlas: a font and the fonts merged into it. Keeps the font data and glyph ranges alive, the
/// 	atlas points into them. Later builds reuse it as long as its key, built from its inputs, is the same.
///----------------------------------------------------------------------------------------------------
struct FontAtlas_t
{
	std::string                                  Key;
	bool                                         IsDefault        = false; /* Becomes io.Fonts. */
	bool                                         IsOwnedByContext = false; /* Is io.Fonts, the ImGui context frees it. */
	ImFontAtlas*                                 Atlas            = nullptr;
	std::vector<std::pair<std::string, ImFont*>> Fonts;
	std::vector<std::shared_ptr<const void>>     KeepAlive;
	size_t                                       Placeholders     = 0;

	~FontAtlas_t();
};

///----------------------------------------------------------------------------------------------------
/// FontBuild_t Struct
/// 	The result of a build: the keys of all atlases in order, and the ones that had to be built.
///----------------------------------------------------------------------------------------------------
struct FontBuild_t
{
	std::vector<std::string>                  Keys;
	std::vector<std::unique_ptr<FontAtlas_t>> Built;
	std::chrono::milliseconds                 BuildTime{};
};

///----------------------------------------------------------------------------------------------------
/// EFontsUpdate Enumeration
///----------------------------------------------------------------------------------------------------
enum class EFontsUpdate
{
	None,
	Atlases,     /* Upload the new atlases with CreateDeviceObjects. */
	DefaultAtlas /* io.Fonts was replaced, the renderer backend has to upload it again. */
};

///----------------------------------------------------------------------------------------------------
/// CFontManager Class
///----------------------------------------------------------------------------------------------------
class CFontManager : public virtual IRefCleaner, public ILogChannel, public IFontUpdateNotifier
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CFontManager();
	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CFontManager();

	///----------------------------------------------------------------------------------------------------
	/// Advance:
	/// 	Starts a rebuild of the font atlases on worker threads if a font changed, and applies a finished
	/// 	one: swaps the atlases and notifies the callbacks of the fonts that changed and the font update
	/// 	subscribers. The previous atlases
	/// 	stay in use until then. Only atlases whose fonts, sizes, glyph ranges or fallbacks changed are built.
	/// 	Each font gets its own atlas, merged fonts join the atlas of the font before them.
	/// 	Glyphs: every font gets the glyph set, fonts added through AddFont also the ranges they declare.
	/// 	The glyph set is the base ranges, the texts of SetTexts and every glyph CollectMissingGlyphs found.
	/// 	The atlas of FONT_DEFAULT is io.Fonts, uploaded by the renderer backend; the others by CreateDeviceObjects.
	///----------------------------------------------------------------------------------------------------
	EFontsUpdate Advance();

	///----------------------------------------------------------------------------------------------------
	/// IsReady:
	/// 	Returns true once the first atlases were applied. No ImGui frame may start before.
	///----------------------------------------------------------------------------------------------------
	bool IsReady() const;

	///----------------------------------------------------------------------------------------------------
	/// SetTexts:
	/// 	Sets the texts whose glyphs belong to the glyph set, e.g. the active locale texts. Replaces the
	/// 	previous texts. Rebuilds the atlases with the next Advance only if the glyph set changed.
	/// 	Render thread only.
	///----------------------------------------------------------------------------------------------------
	void SetTexts(const std::vector<const char*>& aTexts);

	///----------------------------------------------------------------------------------------------------
	/// SetGlyphCache:
	/// 	Loads the glyphs collected in earlier sessions from the file and saves new ones to it.
	/// 	Render thread only.
	///----------------------------------------------------------------------------------------------------
	void SetGlyphCache(const std::filesystem::path& aPath);

	///----------------------------------------------------------------------------------------------------
	/// CollectMissingGlyphs:
	/// 	Adds the glyphs that were drawn as placeholders to the glyph set, they are built with the next
	/// 	Advance. The set only grows, also across sessions through the glyph cache. Render thread only.
	///----------------------------------------------------------------------------------------------------
	void CollectMissingGlyphs(const ImDrawData* aDrawData);

	///----------------------------------------------------------------------------------------------------
	/// AddFallbackFont:
	/// 	Merges the font into every font, for the glyphs neither it nor an earlier fallback has.
	/// 	Skipped if the file does not exist, e.g. a Windows font under Wine.
	///----------------------------------------------------------------------------------------------------
	void AddFallbackFont(const std::filesystem::path& aPath);

	///----------------------------------------------------------------------------------------------------
	/// CreateDeviceObjects:
	/// 	Uploads the textures of every atlas except io.Fonts and frees their pixels.
	/// 	The textures survive resizes; on another device the atlases are built again.
	///----------------------------------------------------------------------------------------------------
	void CreateDeviceObjects(ID3D11Device* aDevice);

	///----------------------------------------------------------------------------------------------------
	/// InvalidateDeviceObjects:
	/// 	Releases the textures created by CreateDeviceObjects. Only for replaced atlases or shutdown.
	///----------------------------------------------------------------------------------------------------
	void InvalidateDeviceObjects();

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
	/// 	Built with the glyph ranges declared in aConfig (ImGui's default ranges if none) and the locale texts.
	///----------------------------------------------------------------------------------------------------
	void AddFont(const char* aIdentifier, float aFontSize, const char* aFilename, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig);

	///----------------------------------------------------------------------------------------------------
	/// AddFont:
	/// 	Adds a font from an embedded resource and sends updates to the callback.
	/// 	Built with the glyph ranges declared in aConfig (ImGui's default ranges if none) and the locale texts.
	///----------------------------------------------------------------------------------------------------
	void AddFont(const char* aIdentifier, float aFontSize, unsigned aResourceID, HMODULE aModule, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig);

	///----------------------------------------------------------------------------------------------------
	/// AddFont:
	/// 	Adds a font from memory and sends updates to the callback.
	/// 	Built with the glyph ranges declared in aConfig (ImGui's default ranges if none) and the locale texts.
	///----------------------------------------------------------------------------------------------------
	void AddFont(const char* aIdentifier, float aFontSize, std::span<const uint8_t> aData, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig);

	///----------------------------------------------------------------------------------------------------
	/// ReplaceFont:
	/// 	Adds or replaces a Nexus font from disk, keeping its subscribers. Nexus fonts stay registered
	/// 	without subscribers; Nexus receives them through the font update notification.
	///----------------------------------------------------------------------------------------------------
	void ReplaceFont(const char* aIdentifier, float aFontSize, const char* aFilename, ImFontConfig* aConfig);

	///----------------------------------------------------------------------------------------------------
	/// ReplaceFont:
	/// 	Adds or replaces a Nexus font from a Nexus embedded file. See above.
	///----------------------------------------------------------------------------------------------------
	void ReplaceFont(const char* aIdentifier, float aFontSize, const Raidcore::Core::Platform::Windows::CPackFile& aPackFile, ImFontConfig* aConfig);

	///----------------------------------------------------------------------------------------------------
	/// ReplaceFont:
	/// 	Adds or replaces a Nexus font from memory. See above.
	///----------------------------------------------------------------------------------------------------
	void ReplaceFont(const char* aIdentifier, float aFontSize, std::span<const uint8_t> aData, ImFontConfig* aConfig);

	///----------------------------------------------------------------------------------------------------
	/// AddDefaultFont:
	/// 	Adds the default font as a Nexus font, unless FONT_DEFAULT already exists.
	///----------------------------------------------------------------------------------------------------
	void AddDefaultFont();

	///----------------------------------------------------------------------------------------------------
	/// ResizeFont:
	/// 	Changes the size of a given font.
	///----------------------------------------------------------------------------------------------------
	void ResizeFont(const char* aIdentifier, float aFontSize);

	///----------------------------------------------------------------------------------------------------
	/// CleanupRefs:
	/// 	Removes all unreleased references in the given address space.
	///----------------------------------------------------------------------------------------------------
	uint32_t CleanupRefs(void* aStartAddress, void* aEndAddress) override;

	private:
	mutable std::mutex         Mutex;
	std::vector<ManagedFont_t> Registry;
	bool                       IsFontAtlasBuilt = false;

	std::vector<std::shared_ptr<const std::vector<uint8_t>>> FallbackFonts;

	/* Render thread only. Ranges is replaced, never changed, so an unchanged pointer means unchanged ranges. */
	ImFontGlyphRangesBuilder                    TextGlyphs;
	ImFontGlyphRangesBuilder                    CollectedGlyphs;
	std::shared_ptr<const std::vector<ImWchar>> Ranges;
	std::filesystem::path                       GlyphCachePath;
	bool                                        IsGlyphCacheDirty = false;

	/* Render thread only. */
	std::future<FontBuild_t>                    Build;
	std::vector<std::unique_ptr<FontAtlas_t>>   Atlases;
	ID3D11Device*                               Device = nullptr; /* The textures of Atlases belong to it. */

	///----------------------------------------------------------------------------------------------------
	/// Apply:
	/// 	Replaces the atlases and io.Fonts with a finished build, reusing the unchanged ones, and
	/// 	notifies the callbacks of the fonts whose pointer changed.
	///----------------------------------------------------------------------------------------------------
	EFontsUpdate Apply(FontBuild_t aBuild);

	///----------------------------------------------------------------------------------------------------
	/// UpdateRanges:
	/// 	Builds the glyph set and rebuilds the atlases with the next Advance if it changed.
	///----------------------------------------------------------------------------------------------------
	void UpdateRanges();

	///----------------------------------------------------------------------------------------------------
	/// LogAtlases:
	/// 	Logs the built atlases and the totals of all atlases.
	///----------------------------------------------------------------------------------------------------
	void LogAtlases(const std::vector<FontAtlas_t*>& aBuilt, std::chrono::milliseconds aBuildTime) const;

	///----------------------------------------------------------------------------------------------------
	/// AddFontInternal:
	/// 	Adds a font from memory and registers the callback.
	/// 	aUseDeclaredRanges: build with the ranges of aConfig instead of Nexus' ranges.
	///----------------------------------------------------------------------------------------------------
	void AddFontInternal(const char* aIdentifier, float aFontSize, std::span<const uint8_t> aData, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig, bool aUseDeclaredRanges);

	///----------------------------------------------------------------------------------------------------
	/// CreateManagedFont:
	/// 	Creates a new ManagedFont_t for the Registry.
	///----------------------------------------------------------------------------------------------------
	ManagedFont_t CreateManagedFont(std::string aIdentifier, float aFontSize, std::span<const uint8_t> aData, ImFontConfig* aConfig, bool aUseDeclaredRanges);

	public:
	CSetting<float>       FontSize{ "FontSize", 15.0f };
	CSetting<std::string> UserFont{ "UserFont", "" };
};
