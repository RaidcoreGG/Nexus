///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  FontGlyphs.h
/// Description  :  Glyph sets of the fonts and the placeholder glyphs that find the missing ones.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

#include "imgui/imgui.h"

///----------------------------------------------------------------------------------------------------
/// AddBaseGlyphRanges:
/// 	Adds the glyphs every font has regardless of use: Basic Latin, Latin-1, Latin Extended-A and -B.
///----------------------------------------------------------------------------------------------------
void AddBaseGlyphRanges(ImFontGlyphRangesBuilder& aBuilder);

///----------------------------------------------------------------------------------------------------
/// GetRenderableGlyphs:
/// 	Returns one bit per codepoint of the Basic Multilingual Plane that the font has a glyph for.
///----------------------------------------------------------------------------------------------------
std::vector<uint64_t> GetRenderableGlyphs(const uint8_t* aData, int aFontNo);

///----------------------------------------------------------------------------------------------------
/// AddPlaceholderGlyphs:
/// 	Gives every renderable codepoint the font has no glyph for an invisible placeholder glyph that
/// 	encodes the codepoint in its texture coordinates, so drawing it can be found in the draw data.
/// 	The placeholder advances like the fallback glyph. Returns the number of placeholders added.
///----------------------------------------------------------------------------------------------------
size_t AddPlaceholderGlyphs(ImFont* aFont, const std::vector<uint64_t>& aRenderable);

///----------------------------------------------------------------------------------------------------
/// CollectPlaceholderGlyphs:
/// 	Adds the codepoints of the placeholder glyphs in the draw data to the builder.
/// 	Returns true if one was not in it yet.
///----------------------------------------------------------------------------------------------------
bool CollectPlaceholderGlyphs(const ImDrawData* aDrawData, ImFontGlyphRangesBuilder& aBuilder);

///----------------------------------------------------------------------------------------------------
/// LoadGlyphCache:
/// 	Adds the codepoints of the cache file (UTF-8 text) to the builder.
///----------------------------------------------------------------------------------------------------
void LoadGlyphCache(const std::filesystem::path& aPath, ImFontGlyphRangesBuilder& aBuilder);

///----------------------------------------------------------------------------------------------------
/// SaveGlyphCache:
/// 	Writes the codepoints as UTF-8 text, replacing the file only once it is complete.
///----------------------------------------------------------------------------------------------------
void SaveGlyphCache(const std::filesystem::path& aPath, const std::vector<ImWchar>& aCodepoints);
