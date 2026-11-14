///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  FontGlyphs.cpp
/// Description  :  Glyph sets of the fonts and the placeholder glyphs that find the missing ones.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "FontGlyphs.h"

#include <fstream>
#include <iterator>
#include <string>
#include <Windows.h>

#include "imgui/imgui_internal.h"

/* ImGui compiles its stb_truetype statically into imgui_draw.cpp, this is a private copy. */
#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "imgui/imstb_truetype.h"

/* Real texture coordinates are within 0 to 1, the placeholders store the codepoint as -U. */
static constexpr float PLACEHOLDER_V = -12345.0f;

static constexpr unsigned int FIRST_CODEPOINT = 0x20;
static constexpr unsigned int LAST_CODEPOINT  = 0xFFFF;

static bool IsCodepoint(unsigned int aCodepoint)
{
	return aCodepoint >= FIRST_CODEPOINT && aCodepoint <= LAST_CODEPOINT && (aCodepoint < 0xD800 || aCodepoint > 0xDFFF);
}

void AddBaseGlyphRanges(ImFontGlyphRangesBuilder& aBuilder)
{
	static const ImWchar s_Ranges[] =
	{
		0x0020, 0x00FF, /* Basic Latin, Latin-1 Supplement */
		0x0100, 0x024F, /* Latin Extended-A and -B */
		0,
	};

	aBuilder.AddRanges(s_Ranges);
}

std::vector<uint64_t> GetRenderableGlyphs(const uint8_t* aData, int aFontNo)
{
	std::vector<uint64_t> renderable((LAST_CODEPOINT + 1) / 64, 0);

	stbtt_fontinfo info{};
	int offset = stbtt_GetFontOffsetForIndex(aData, aFontNo);
	if (offset < 0 || !stbtt_InitFont(&info, aData, offset)) { return renderable; }

	for (unsigned int codepoint = FIRST_CODEPOINT; codepoint <= LAST_CODEPOINT; codepoint++)
	{
		if (IsCodepoint(codepoint) && stbtt_FindGlyphIndex(&info, static_cast<int>(codepoint)))
		{
			renderable[codepoint >> 6] |= 1ull << (codepoint & 63);
		}
	}

	return renderable;
}

size_t AddPlaceholderGlyphs(ImFont* aFont, const std::vector<uint64_t>& aRenderable)
{
	if (!aFont || aRenderable.empty()) { return 0; }

	std::vector<ImWchar> missing;
	for (unsigned int codepoint = FIRST_CODEPOINT; codepoint <= LAST_CODEPOINT; codepoint++)
	{
		if ((aRenderable[codepoint >> 6] >> (codepoint & 63)) & 1 && !aFont->FindGlyphNoFallback(static_cast<ImWchar>(codepoint)))
		{
			missing.push_back(static_cast<ImWchar>(codepoint));
		}
	}

	/* Glyph indices are 16 bit and 0xFFFF means none. */
	size_t room = 0xFFFE - static_cast<size_t>(aFont->Glyphs.Size);
	if (missing.size() > room) { missing.resize(room); }
	if (missing.empty()) { return 0; }

	aFont->GrowIndex(LAST_CODEPOINT + 1);
	for (float& advance : aFont->IndexAdvanceX)
	{
		if (advance < 0.0f) { advance = aFont->FallbackAdvanceX; }
	}

	aFont->Glyphs.reserve(aFont->Glyphs.Size + static_cast<int>(missing.size()));
	for (ImWchar codepoint : missing)
	{
		ImFontGlyph glyph{};
		glyph.Codepoint = codepoint;
		glyph.Visible   = 1; /* Zero width draws nothing, but the quad still reaches the draw data. */
		glyph.AdvanceX  = aFont->FallbackAdvanceX;
		glyph.Y1        = aFont->FontSize;
		glyph.U0        = -static_cast<float>(codepoint);
		glyph.U1        = glyph.U0;
		glyph.V0        = PLACEHOLDER_V;
		glyph.V1        = PLACEHOLDER_V;

		aFont->IndexLookup[codepoint]   = static_cast<ImWchar>(aFont->Glyphs.Size);
		aFont->IndexAdvanceX[codepoint] = aFont->FallbackAdvanceX;
		aFont->Glyphs.push_back(glyph);

		unsigned int page = codepoint / 4096;
		aFont->Used4kPagesMap[page >> 3] |= static_cast<ImU8>(1 << (page & 7));
	}

	/* The glyphs may have moved. */
	aFont->FallbackGlyph = aFont->FindGlyphNoFallback(aFont->FallbackChar);

	return missing.size();
}

bool CollectPlaceholderGlyphs(const ImDrawData* aDrawData, ImFontGlyphRangesBuilder& aBuilder)
{
	if (!aDrawData) { return false; }

	bool isNew = false;
	for (int i = 0; i < aDrawData->CmdListsCount; i++)
	{
		const ImVector<ImDrawVert>& vertices = aDrawData->CmdLists[i]->VtxBuffer;
		for (const ImDrawVert& vertex : vertices)
		{
			if (vertex.uv.y != PLACEHOLDER_V) { continue; }

			float value = -vertex.uv.x;
			if (value < FIRST_CODEPOINT || value > LAST_CODEPOINT) { continue; }

			unsigned int codepoint = static_cast<unsigned int>(value);
			if (!IsCodepoint(codepoint) || aBuilder.GetBit(codepoint)) { continue; }

			aBuilder.AddChar(static_cast<ImWchar>(codepoint));
			isNew = true;
		}
	}

	return isNew;
}

void LoadGlyphCache(const std::filesystem::path& aPath, ImFontGlyphRangesBuilder& aBuilder)
{
	std::ifstream file(aPath, std::ios::binary);
	if (!file) { return; }

	std::string text{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
	const char* it  = text.c_str();
	const char* end = it + text.size();

	while (it < end)
	{
		unsigned int codepoint = 0;
		int length = ImTextCharFromUtf8(&codepoint, it, end);
		if (length <= 0) { break; }
		it += length;

		if (IsCodepoint(codepoint))
		{
			aBuilder.AddChar(static_cast<ImWchar>(codepoint));
		}
	}
}

void SaveGlyphCache(const std::filesystem::path& aPath, const std::vector<ImWchar>& aCodepoints)
{
	std::vector<ImWchar> codepoints = aCodepoints;
	codepoints.push_back(0);

	std::string text(aCodepoints.size() * 3 + 1, '\0');
	int length = ImTextStrToUtf8(text.data(), static_cast<int>(text.size()), codepoints.data(), nullptr);
	text.resize(static_cast<size_t>(length));

	std::filesystem::path temporary = aPath;
	temporary += ".tmp";

	{
		std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
		if (!file) { return; }
		file.write(text.data(), static_cast<std::streamsize>(text.size()));
		if (!file) { return; }
	}

	MoveFileExW(temporary.c_str(), aPath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
}
