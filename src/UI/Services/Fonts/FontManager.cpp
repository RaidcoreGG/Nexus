///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  FontManager.cpp
/// Description  :  Handles fonts for the ImGui implementation of the UI.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "FontManager.h"

#include <algorithm>
#include <chrono>
#include <map>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>

#include "imgui/imgui_internal.h"

#include "FontGlyphs.h"
#include "res/Resources.h"

using namespace Raidcore::Core::Platform::Windows;

CFontManager::CFontManager() : IRefCleaner("FontManager"), ILogChannel("Fonts")
{
	float storedFontSz = this->FontSize.Get();
	if (storedFontSz <= 0)
	{
		storedFontSz = min(max(storedFontSz, 1.0f), 50.0f);
		this->FontSize.Set(storedFontSz);
	}
	ImGui::GetCurrentContext()->FontSize = storedFontSz;

	this->SetTexts({});
}

FontAtlas_t::~FontAtlas_t()
{
	if (this->Atlas && !this->IsOwnedByContext)
	{
		IM_DELETE(this->Atlas);
	}
}

/* Glyphs per font file and face, shared by the atlases of one build. */
using RenderableGlyphs_t = std::map<std::pair<const void*, int>, std::vector<uint64_t>>;

///----------------------------------------------------------------------------------------------------
/// FontPlanFont_t Struct
///----------------------------------------------------------------------------------------------------
struct FontPlanFont_t
{
	std::string                                 Identifier;
	float                                       Size;
	ImFontConfig                                Config;
	std::shared_ptr<const std::vector<uint8_t>> Data;
	std::shared_ptr<const std::vector<ImWchar>> DeclaredRanges; /* Addon fonts, merged with the glyph set. */
};

///----------------------------------------------------------------------------------------------------
/// FontPlan_t Struct
/// 	One atlas to build: a font and the fonts merged into it.
///----------------------------------------------------------------------------------------------------
struct FontPlan_t
{
	std::string                                 Key;
	bool                                        IsDefault = false;
	std::shared_ptr<const std::vector<ImWchar>> Ranges;   /* The glyph set. */
	std::vector<FontPlanFont_t>                 Fonts;
};

///----------------------------------------------------------------------------------------------------
/// AppendKey:
/// 	Appends the bytes of a value to an atlas key.
///----------------------------------------------------------------------------------------------------
template <typename T>
static void AppendKey(std::string& aKey, const T& aValue)
{
	aKey.append(reinterpret_cast<const char*>(&aValue), sizeof(T));
}

///----------------------------------------------------------------------------------------------------
/// AppendKey:
/// 	Appends what affects the build of a font to an atlas key. Pointers identify immutable shared data.
///----------------------------------------------------------------------------------------------------
static void AppendKey(std::string& aKey, const FontPlanFont_t& aFont)
{
	aKey.append(aFont.Identifier);
	aKey.push_back('\0');
	AppendKey(aKey, aFont.Size);
	AppendKey(aKey, aFont.Data.get());
	AppendKey(aKey, aFont.DeclaredRanges.get());
	AppendKey(aKey, aFont.Config.FontNo);
	AppendKey(aKey, aFont.Config.OversampleH);
	AppendKey(aKey, aFont.Config.OversampleV);
	AppendKey(aKey, aFont.Config.PixelSnapH);
	AppendKey(aKey, aFont.Config.GlyphExtraSpacing);
	AppendKey(aKey, aFont.Config.GlyphOffset);
	AppendKey(aKey, aFont.Config.GlyphMinAdvanceX);
	AppendKey(aKey, aFont.Config.GlyphMaxAdvanceX);
	AppendKey(aKey, aFont.Config.MergeMode);
	AppendKey(aKey, aFont.Config.RasterizerFlags);
	AppendKey(aKey, aFont.Config.RasterizerMultiply);
	AppendKey(aKey, aFont.Config.EllipsisChar);
}

///----------------------------------------------------------------------------------------------------
/// BuildAtlas:
/// 	Builds the atlas of one plan. Touches no ImGui context state, runs on a worker.
/// 	The fallback fonts are merged last, so the glyphs of the font and its own merges take precedence.
/// 	Every glyph the fonts could render but that is not in the set gets a placeholder.
///----------------------------------------------------------------------------------------------------
static std::unique_ptr<FontAtlas_t> BuildAtlas(
	const FontPlan_t&                                               aPlan,
	const std::vector<std::shared_ptr<const std::vector<uint8_t>>>& aFallbacks,
	const RenderableGlyphs_t&                                       aRenderable
)
{
	std::unique_ptr<FontAtlas_t> entry = std::make_unique<FontAtlas_t>();
	entry->Key       = aPlan.Key;
	entry->IsDefault = aPlan.IsDefault;
	entry->Atlas     = IM_NEW(ImFontAtlas)();
	entry->Atlas->Flags |= ImFontAtlasFlags_NoPowerOfTwoHeight;
	entry->KeepAlive.push_back(aPlan.Ranges);

	auto resolveRanges = [&entry, &aPlan](const FontPlanFont_t& aFont) -> const ImWchar*
	{
		if (!aFont.DeclaredRanges) { return aPlan.Ranges->data(); }

		ImFontGlyphRangesBuilder rb{};
		rb.AddRanges(aFont.DeclaredRanges->data());
		rb.AddRanges(aPlan.Ranges->data());

		ImVector<ImWchar> ranges;
		rb.BuildRanges(&ranges);

		std::shared_ptr<const std::vector<ImWchar>> merged = std::make_shared<const std::vector<ImWchar>>(ranges.begin(), ranges.end());
		entry->KeepAlive.push_back(merged);
		return merged->data();
	};

	const ImWchar* groupRanges = nullptr;
	for (const FontPlanFont_t& font : aPlan.Fonts)
	{
		const ImWchar* ranges = resolveRanges(font);
		if (!groupRanges) { groupRanges = ranges; }

		ImFontConfig config = font.Config;
		ImFont* pointer = entry->Atlas->AddFontFromMemoryTTF(const_cast<uint8_t*>(font.Data->data()), static_cast<int>(font.Data->size()), font.Size, &config, ranges);

		entry->Fonts.emplace_back(font.Identifier, pointer);
		entry->KeepAlive.push_back(font.Data);
	}

	if (!aPlan.Fonts.empty())
	{
		for (const std::shared_ptr<const std::vector<uint8_t>>& fallback : aFallbacks)
		{
			ImFontConfig config{};
			config.MergeMode            = true;
			config.FontDataOwnedByAtlas = false;
			config.OversampleH          = 1; /* CJK at UI sizes gains nothing from it, it triples build time and atlas size. */
			entry->Atlas->AddFontFromMemoryTTF(const_cast<uint8_t*>(fallback->data()), static_cast<int>(fallback->size()), aPlan.Fonts.front().Size, &config, groupRanges);
			entry->KeepAlive.push_back(fallback);
		}
	}
	else
	{
		/* NewFrame requires io.Fonts to be built and not empty. */
		entry->Atlas->AddFontDefault();
	}

	entry->Atlas->Build();

	if (!aPlan.Fonts.empty() && !entry->Atlas->Fonts.empty())
	{
		std::vector<uint64_t> renderable;
		auto addRenderable = [&aRenderable, &renderable](const void* aData, int aFontNo)
		{
			auto it = aRenderable.find({ aData, aFontNo });
			if (it == aRenderable.end()) { return; }

			if (renderable.empty()) { renderable.resize(it->second.size(), 0); }
			for (size_t i = 0; i < renderable.size(); i++)
			{
				renderable[i] |= it->second[i];
			}
		};

		for (const FontPlanFont_t& font : aPlan.Fonts)
		{
			addRenderable(font.Data.get(), font.Config.FontNo);
		}
		for (const std::shared_ptr<const std::vector<uint8_t>>& fallback : aFallbacks)
		{
			addRenderable(fallback.get(), 0);
		}

		entry->Placeholders = AddPlaceholderGlyphs(entry->Atlas->Fonts[0], renderable);
	}

	return entry;
}

///----------------------------------------------------------------------------------------------------
/// BuildAtlases:
/// 	Builds the plans in parallel. Saves the collected glyphs first if aCachePath is set.
///----------------------------------------------------------------------------------------------------
static FontBuild_t BuildAtlases(
	std::vector<std::string>                                 aKeys,
	std::vector<FontPlan_t>                                  aPlans,
	std::vector<std::shared_ptr<const std::vector<uint8_t>>> aFallbacks,
	std::filesystem::path                                    aCachePath,
	std::vector<ImWchar>                                     aCollected
)
{
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

	if (!aCachePath.empty())
	{
		SaveGlyphCache(aCachePath, aCollected);
	}

	RenderableGlyphs_t renderable;
	auto addRenderable = [&renderable](const std::shared_ptr<const std::vector<uint8_t>>& aData, int aFontNo)
	{
		if (renderable.find({ aData.get(), aFontNo }) != renderable.end()) { return; }
		renderable.emplace(std::make_pair(aData.get(), aFontNo), GetRenderableGlyphs(aData->data(), aFontNo));
	};
	for (const FontPlan_t& plan : aPlans)
	{
		for (const FontPlanFont_t& font : plan.Fonts)
		{
			addRenderable(font.Data, font.Config.FontNo);
		}
	}
	if (!aPlans.empty())
	{
		for (const std::shared_ptr<const std::vector<uint8_t>>& fallback : aFallbacks)
		{
			addRenderable(fallback, 0);
		}
	}

	std::vector<std::future<std::unique_ptr<FontAtlas_t>>> builds;
	for (const FontPlan_t& plan : aPlans)
	{
		builds.push_back(std::async(std::launch::async, BuildAtlas, std::cref(plan), std::cref(aFallbacks), std::cref(renderable)));
	}

	FontBuild_t result{};
	result.Keys = std::move(aKeys);
	for (std::future<std::unique_ptr<FontAtlas_t>>& build : builds)
	{
		result.Built.push_back(build.get());
	}
	result.BuildTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);

	return result;
}

CFontManager::~CFontManager()
{
	if (this->Build.valid())
	{
		this->Build.wait();
	}

	this->InvalidateDeviceObjects();
}

EFontsUpdate CFontManager::Advance()
{
	if (this->Build.valid())
	{
		if (this->Build.wait_for(std::chrono::seconds(0)) != std::future_status::ready) { return EFontsUpdate::None; }

		return this->Apply(this->Build.get());
	}

	std::vector<FontPlanFont_t> fonts;
	std::vector<std::shared_ptr<const std::vector<uint8_t>>> fallbacks;

	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		if (this->IsFontAtlasBuilt) { return EFontsUpdate::None; }
		this->IsFontAtlasBuilt = true;

		for (const ManagedFont_t& font : this->Registry)
		{
			FontPlanFont_t& input = fonts.emplace_back();
			input.Identifier     = font.Identifier;
			input.Size           = font.Size;
			input.Data           = !font.Config ? nullptr : font.Data;
			input.DeclaredRanges = font.GlyphRanges;
			if (font.Config) { input.Config = *font.Config; }
		}

		fallbacks = this->FallbackFonts;
	}

	/* Groups: a font and the fonts merged into it share one atlas. */
	std::vector<FontPlan_t> plans;
	FontPlan_t* group = nullptr;
	for (size_t i = 0; i < fonts.size(); i++)
	{
		FontPlanFont_t& font = fonts[i];
		bool isMerged = font.Config.MergeMode && group;

		if (!font.Data)
		{
			if (!font.Config.MergeMode) { group = nullptr; }
			continue;
		}

		if (!isMerged)
		{
			bool hasDefault = false;
			for (size_t j = i; j < fonts.size(); j++)
			{
				if (j > i && !fonts[j].Config.MergeMode) { break; }
				if (fonts[j].Identifier == "FONT_DEFAULT") { hasDefault = true; break; }
			}

			group = &plans.emplace_back();
			group->IsDefault = hasDefault;
			group->Ranges    = this->Ranges;
		}

		font.Config.MergeMode = isMerged;
		group->Fonts.push_back(std::move(font));
	}

	bool hasDefault = std::any_of(plans.begin(), plans.end(), [](const FontPlan_t& aPlan) { return aPlan.IsDefault; });
	if (!hasDefault)
	{
		FontPlan_t& plan = plans.emplace_back();
		plan.IsDefault = true;
		plan.Ranges    = this->Ranges;
	}

	std::vector<std::string> keys;
	std::vector<FontPlan_t>  toBuild;
	for (FontPlan_t& plan : plans)
	{
		AppendKey(plan.Key, plan.IsDefault);
		AppendKey(plan.Key, plan.Ranges.get());
		for (const FontPlanFont_t& font : plan.Fonts)
		{
			AppendKey(plan.Key, font);
		}
		for (const std::shared_ptr<const std::vector<uint8_t>>& fallback : fallbacks)
		{
			AppendKey(plan.Key, fallback.get());
		}

		keys.push_back(plan.Key);

		bool isBuilt = std::any_of(this->Atlases.begin(), this->Atlases.end(), [&plan](const std::unique_ptr<FontAtlas_t>& aEntry) { return aEntry->Key == plan.Key; });
		if (!isBuilt)
		{
			toBuild.push_back(std::move(plan));
		}
	}

	std::filesystem::path cachePath;
	std::vector<ImWchar>  collected;
	if (this->IsGlyphCacheDirty)
	{
		this->IsGlyphCacheDirty = false;
		cachePath = this->GlyphCachePath;

		for (unsigned int codepoint = 0; codepoint <= 0xFFFF; codepoint++)
		{
			if (this->CollectedGlyphs.GetBit(codepoint)) { collected.push_back(static_cast<ImWchar>(codepoint)); }
		}
	}

	this->Build = std::async(std::launch::async, BuildAtlases, std::move(keys), std::move(toBuild), std::move(fallbacks), std::move(cachePath), std::move(collected));
	return EFontsUpdate::None;
}

bool CFontManager::IsReady() const
{
	return !this->Atlases.empty();
}

EFontsUpdate CFontManager::Apply(FontBuild_t aBuild)
{
	std::vector<FontAtlas_t*> built;
	for (const std::unique_ptr<FontAtlas_t>& entry : aBuild.Built)
	{
		built.push_back(entry.get());
	}

	std::vector<std::unique_ptr<FontAtlas_t>> next;
	for (const std::string& key : aBuild.Keys)
	{
		auto takeFrom = [&key](std::vector<std::unique_ptr<FontAtlas_t>>& aEntries) -> std::unique_ptr<FontAtlas_t>
		{
			auto it = std::find_if(aEntries.begin(), aEntries.end(), [&key](const std::unique_ptr<FontAtlas_t>& aEntry) { return aEntry && aEntry->Key == key; });
			return it != aEntries.end() ? std::move(*it) : nullptr;
		};

		std::unique_ptr<FontAtlas_t> entry = takeFrom(aBuild.Built);
		if (!entry) { entry = takeFrom(this->Atlases); }
		if (entry) { next.push_back(std::move(entry)); }
	}

	auto findFont = [&next](const std::string& aIdentifier) -> ImFont*
	{
		for (const std::unique_ptr<FontAtlas_t>& entry : next)
		{
			for (const auto& [identifier, pointer] : entry->Fonts)
			{
				if (identifier == aIdentifier) { return pointer; }
			}
		}
		return nullptr;
	};

	struct Change_t
	{
		std::string                        Identifier;
		ImFont*                            Pointer;
		std::vector<FONTS_RECEIVECALLBACK> Subscribers;
	};

	std::vector<Change_t> changes;
	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		for (const ManagedFont_t& font : this->Registry)
		{
			ImFont* pointer = findFont(font.Identifier);
			if (pointer != font.Pointer)
			{
				changes.push_back({ font.Identifier, pointer, font.Subscribers });
			}
		}
	}

	/* Subscribers drop the old pointers before their atlases are freed. */
	for (const Change_t& change : changes)
	{
		this->NotifyFontUpdate(change.Identifier.c_str(), nullptr);

		for (FONTS_RECEIVECALLBACK callback : change.Subscribers)
		{
			callback(change.Identifier.c_str(), nullptr);
		}
	}

	ImGuiIO& io = ImGui::GetIO();
	auto itDefault = std::find_if(next.begin(), next.end(), [](const std::unique_ptr<FontAtlas_t>& aEntry) { return aEntry->IsDefault; });
	bool isDefaultChanged = itDefault != next.end() && (*itDefault)->Atlas != io.Fonts;

	/* What is left of the previous atlases was not reused. */
	this->InvalidateDeviceObjects();

	if (isDefaultChanged)
	{
		ImFontAtlas* previous = io.Fonts;
		io.Fonts = (*itDefault)->Atlas;
		(*itDefault)->IsOwnedByContext = true;
		IM_DELETE(previous);
	}

	this->Atlases = std::move(next);

	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		for (ManagedFont_t& font : this->Registry)
		{
			font.Pointer = findFont(font.Identifier);
		}
	}

	for (const Change_t& change : changes)
	{
		this->NotifyFontUpdate(change.Identifier.c_str(), change.Pointer);

		for (FONTS_RECEIVECALLBACK callback : change.Subscribers)
		{
			callback(change.Identifier.c_str(), change.Pointer);
		}
	}

	this->LogAtlases(built, aBuild.BuildTime);

	return isDefaultChanged ? EFontsUpdate::DefaultAtlas : EFontsUpdate::Atlases;
}

void CFontManager::LogAtlases(const std::vector<FontAtlas_t*>& aBuilt, std::chrono::milliseconds aBuildTime) const
{
	auto glyphCount = [](const ImFontAtlas* aAtlas)
	{
		size_t glyphs = 0;
		for (const ImFont* font : aAtlas->Fonts)
		{
			glyphs += font->Glyphs.Size;
		}
		return glyphs;
	};

	auto megabytes = [](size_t aBytes) { return aBytes / (1024.0 * 1024.0); };

	for (const FontAtlas_t* entry : aBuilt)
	{
		const ImFontAtlas* atlas = entry->Atlas;
		this->LogDebug(
			"Atlas \"%s\": %dx%d, %zu glyphs, %.1f MB texture, %zu placeholders.",
			atlas->ConfigData.empty() ? "" : atlas->ConfigData[0].Name,
			atlas->TexWidth,
			atlas->TexHeight,
			glyphCount(atlas) - entry->Placeholders,
			megabytes(static_cast<size_t>(atlas->TexWidth) * atlas->TexHeight * 4),
			entry->Placeholders
		);
	}

	size_t glyphs       = 0;
	size_t placeholders = 0;
	size_t texture      = 0;
	size_t pixels       = 0;
	for (const std::unique_ptr<FontAtlas_t>& entry : this->Atlases)
	{
		size_t size = static_cast<size_t>(entry->Atlas->TexWidth) * entry->Atlas->TexHeight;
		glyphs       += glyphCount(entry->Atlas) - entry->Placeholders;
		placeholders += entry->Placeholders;
		texture      += size * 4;

		/* The renderer backend keeps the alpha and RGBA pixels of io.Fonts, the others are freed after the upload. */
		if (entry->IsDefault) { pixels += size * 5; }
	}

	this->LogDebug(
		"%zu atlases (%zu built in %lld ms, %zu reused): %zu glyphs, %.1f MB textures, %.1f MB pixels kept in RAM, %zu placeholders (%.1f MB).",
		this->Atlases.size(),
		aBuilt.size(),
		static_cast<long long>(aBuildTime.count()),
		this->Atlases.size() - aBuilt.size(),
		glyphs,
		megabytes(texture),
		megabytes(pixels),
		placeholders,
		megabytes(placeholders * sizeof(ImFontGlyph))
	);
}

void CFontManager::SetTexts(const std::vector<const char*>& aTexts)
{
	this->TextGlyphs.Clear();

	for (const char* text : aTexts)
	{
		this->TextGlyphs.AddText(text);
	}

	this->UpdateRanges();
}

void CFontManager::SetGlyphCache(const std::filesystem::path& aPath)
{
	this->GlyphCachePath = aPath;
	LoadGlyphCache(aPath, this->CollectedGlyphs);
	this->UpdateRanges();
}

void CFontManager::CollectMissingGlyphs(const ImDrawData* aDrawData)
{
	if (!CollectPlaceholderGlyphs(aDrawData, this->CollectedGlyphs)) { return; }

	this->IsGlyphCacheDirty = !this->GlyphCachePath.empty();
	this->UpdateRanges();
}

void CFontManager::UpdateRanges()
{
	ImFontGlyphRangesBuilder rb{};
	AddBaseGlyphRanges(rb);

	for (int i = 0; i < rb.UsedChars.Size; i++)
	{
		rb.UsedChars[i] |= this->TextGlyphs.UsedChars[i] | this->CollectedGlyphs.UsedChars[i];
	}

	ImVector<ImWchar> built;
	rb.BuildRanges(&built);

	/* Replaced only if the glyphs changed, the atlases are reused as long as it is the same pointer. */
	std::vector<ImWchar> ranges(built.begin(), built.end());
	if (this->Ranges && *this->Ranges == ranges) { return; }

	this->Ranges = std::make_shared<const std::vector<ImWchar>>(std::move(ranges));

	const std::lock_guard<std::mutex> lock(this->Mutex);
	this->IsFontAtlasBuilt = false;
}

void CFontManager::AddFallbackFont(const std::filesystem::path& aPath)
{
	std::error_code ec;
	if (!std::filesystem::is_regular_file(aPath, ec)) { return; }

	std::ifstream file(aPath, std::ios::binary);
	std::shared_ptr<const std::vector<uint8_t>> data = std::make_shared<const std::vector<uint8_t>>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

	if (data->empty()) { return; }

	const std::lock_guard<std::mutex> lock(this->Mutex);
	this->FallbackFonts.push_back(std::move(data));
	this->IsFontAtlasBuilt = false;
}

void CFontManager::CreateDeviceObjects(ID3D11Device* aDevice)
{
	if (!aDevice || this->Atlases.empty()) { return; }

	/* The pixels are freed after the upload, so another device needs the atlases built again. */
	if (this->Device && this->Device != aDevice)
	{
		this->InvalidateDeviceObjects();
		this->Device = nullptr;

		for (std::unique_ptr<FontAtlas_t>& entry : this->Atlases)
		{
			if (!entry->IsDefault) { entry->Key.clear(); }
		}

		const std::lock_guard<std::mutex> lock(this->Mutex);
		this->IsFontAtlasBuilt = false;
		return;
	}

	this->Device = aDevice;

	for (std::unique_ptr<FontAtlas_t>& entry : this->Atlases)
	{
		ImFontAtlas* atlas = entry->Atlas;

		if (entry->IsDefault) { continue; }
		if (atlas->TexID) { continue; }
		if (!atlas->TexPixelsAlpha8 && !atlas->TexPixelsRGBA32) { continue; }

		unsigned char* pixels = nullptr;
		int width = 0;
		int height = 0;
		atlas->GetTexDataAsRGBA32(&pixels, &width, &height);

		if (!pixels) { continue; }

		D3D11_TEXTURE2D_DESC desc{};
		desc.Width            = width;
		desc.Height           = height;
		desc.MipLevels        = 1;
		desc.ArraySize        = 1;
		desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage            = D3D11_USAGE_DEFAULT;
		desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA subResource{};
		subResource.pSysMem     = pixels;
		subResource.SysMemPitch = desc.Width * 4;

		ID3D11Texture2D* texture = nullptr;
		if (FAILED(aDevice->CreateTexture2D(&desc, &subResource, &texture))) { continue; }

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format              = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;

		ID3D11ShaderResourceView* srv = nullptr;
		aDevice->CreateShaderResourceView(texture, &srvDesc, &srv);
		texture->Release();

		atlas->TexID = static_cast<ImTextureID>(srv);

		/* The texture is kept until the atlas is replaced or the device changes, so the pixels are not needed again. */
		atlas->ClearTexData();
	}
}

void CFontManager::InvalidateDeviceObjects()
{
	for (std::unique_ptr<FontAtlas_t>& entry : this->Atlases)
	{
		if (!entry || entry->IsDefault || !entry->Atlas->TexID) { continue; }

		static_cast<ID3D11ShaderResourceView*>(entry->Atlas->TexID)->Release();
		entry->Atlas->TexID = nullptr;
	}
}

ManagedFont_t* CFontManager::Get(const char* aIdentifier)
{
	std::string str = aIdentifier;

	const std::lock_guard<std::mutex> lock(this->Mutex);
	auto it = std::find_if(this->Registry.begin(), this->Registry.end(), [str](ManagedFont_t& font) { return font.Identifier == str; });

	if (it == this->Registry.end()) { return nullptr; }

	/* get the reference */
	ManagedFont_t& font = *it;

	return &font;
}

void CFontManager::Release(const char* aIdentifier, FONTS_RECEIVECALLBACK aCallback)
{
	if (!aCallback) { return; }

	std::string str = aIdentifier;

	const std::lock_guard<std::mutex> lock(this->Mutex);
	auto it = std::find_if(this->Registry.begin(), this->Registry.end(), [str](ManagedFont_t& font) { return font.Identifier == str; });

	if (it == this->Registry.end()) { return; }

	/* get the reference */
	ManagedFont_t& font = *it;

	for (FONTS_RECEIVECALLBACK callback : font.Subscribers)
	{
		if (callback != aCallback) { continue; }

		/* remove the callback */
		font.Subscribers.erase(std::remove(font.Subscribers.begin(), font.Subscribers.end(), callback), font.Subscribers.end());

		break;
	}

	/* refCount reached 0, "mark" this font for deletion by freeing its underlying buffer */
	if (font.Subscribers.size() == 0 && !font.IsHostFont)
	{
		font.Data.reset();

		delete font.Config;
		font.Config = nullptr;

		this->Registry.erase(it);

		this->IsFontAtlasBuilt = false;
	}
}

void CFontManager::AddFont(const char* aIdentifier, float aFontSize, const char* aFilename, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig)
{
	if (aFontSize < 1.0f) { return; }
	if (!aFilename) { return; }
	if (!std::filesystem::exists(aFilename)) { return; }

	std::string str = aIdentifier;

	ManagedFont_t* font = Get(aIdentifier);

	const std::lock_guard<std::mutex> lock(this->Mutex);

	if (font != nullptr) /* font already exists */
	{
		/* add the callback */
		if (aCallback)
		{
			font->Subscribers.push_back(aCallback);
		}
	}
	else /* create new font */
	{
		size_t size = 0;
		void* buffer = ImFileLoadToMemory(aFilename, "rb", &size, 0);
		std::span<const uint8_t> data{ static_cast<const uint8_t*>(buffer), size };

		/* call AddFontInternal with the memory buffer */
		this->AddFontInternal(aIdentifier, aFontSize, data, aCallback, aConfig, true);

		/* delete buffer, because it was copied in AddFont */
		IM_FREE(buffer);
	}
}

void CFontManager::AddFont(const char* aIdentifier, float aFontSize, unsigned aResourceID, HMODULE aModule, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig)
{
	if (aFontSize < 1.0f) { return; }
	if (aResourceID == 0) { return; }
	if (!aModule) { return; }

	std::string str = aIdentifier;

	ManagedFont_t* font = this->Get(aIdentifier);

	const std::lock_guard<std::mutex> lock(this->Mutex);

	if (font != nullptr) /* font already exists */
	{
		/* add the callback */
		if (aCallback)
		{
			font->Subscribers.push_back(aCallback);
		}
	}
	else /* create new font */
	{
		/* get data */
		std::span<const uint8_t> data = CPackFile(aResourceID, RT_FONT, aModule).GetBytes();

		/* call AddFontInternal with the memory buffer */
		this->AddFontInternal(aIdentifier, aFontSize, data, aCallback, aConfig, true);
	}
}

void CFontManager::AddFont(const char* aIdentifier, float aFontSize, std::span<const uint8_t> aData, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig)
{
	if (aFontSize < 1.0f) { return; }
	if (aData.empty()) { return; }

	std::string str = aIdentifier;

	ManagedFont_t* font = this->Get(aIdentifier);

	const std::lock_guard<std::mutex> lock(this->Mutex);

	if (font != nullptr) /* font already exists */
	{
		/* add the callback */
		if (aCallback)
		{
			font->Subscribers.push_back(aCallback);
		}
	}
	else /* create new font */
	{
		/* call AddFontInternal with the memory buffer */
		this->AddFontInternal(aIdentifier, aFontSize, aData, aCallback, aConfig, true);
	}
}

void CFontManager::ReplaceFont(const char* aIdentifier, float aFontSize, const char* aFilename, ImFontConfig* aConfig)
{
	if (!aFilename) { return; }
	if (!std::filesystem::exists(aFilename)) { return; }

	size_t size = 0;
	void* buffer = ImFileLoadToMemory(aFilename, "rb", &size, 0);

	this->ReplaceFont(aIdentifier, aFontSize, std::span<const uint8_t>{ static_cast<const uint8_t*>(buffer), size }, aConfig);

	IM_FREE(buffer);
}

void CFontManager::ReplaceFont(const char* aIdentifier, float aFontSize, const CPackFile& aPackFile, ImFontConfig* aConfig)
{
	this->ReplaceFont(aIdentifier, aFontSize, aPackFile.GetBytes(), aConfig);
}

void CFontManager::ReplaceFont(const char* aIdentifier, float aFontSize, std::span<const uint8_t> aData, ImFontConfig* aConfig)
{
	if (aFontSize < 1.0f) { return; }
	if (aData.empty()) { return; }

	std::string str = aIdentifier;

	const std::lock_guard<std::mutex> lock(this->Mutex);
	auto it = std::find_if(this->Registry.begin(), this->Registry.end(), [str](ManagedFont_t& font) { return font.Identifier == str; });

	ImFontConfig* previousConfig = it != this->Registry.end() ? it->Config : nullptr;

	ManagedFont_t font = this->CreateManagedFont(aIdentifier, aFontSize, aData, aConfig ? aConfig : previousConfig, false);
	font.IsHostFont = true;

	if (it != this->Registry.end())
	{
		font.Subscribers = it->Subscribers;
		*it = font;
		delete previousConfig;
	}
	else
	{
		this->Registry.push_back(font);
	}

	this->IsFontAtlasBuilt = false;
}

void CFontManager::AddDefaultFont()
{
	{
		const std::lock_guard<std::mutex> lock(this->Mutex);
		auto it = std::find_if(this->Registry.begin(), this->Registry.end(), [](ManagedFont_t& font) { return font.Identifier == "FONT_DEFAULT"; });

		if (it != this->Registry.end()) { return; }
	}

	this->ReplaceFont("FONT_DEFAULT", 15.0f, Resources::FontInter, nullptr);
}

void CFontManager::ResizeFont(const char* aIdentifier, float aFontSize)
{
	if (aFontSize < 1.0f) { return; }

	std::string str = aIdentifier;

	const std::lock_guard<std::mutex> lock(this->Mutex);
	auto it = std::find_if(this->Registry.begin(), this->Registry.end(), [str](ManagedFont_t& font) { return font.Identifier == str; });

	if (it != this->Registry.end()) /* font already exists */
	{
		/* get the reference */
		ManagedFont_t& font = *it;

		if (font.Size != aFontSize)
		{
			font.Size = aFontSize;

			/* invalidate the font atlas to be rebuilt on Advance */
			this->IsFontAtlasBuilt = false;
		}
	}
}

uint32_t CFontManager::CleanupRefs(void* aStartAddress, void* aEndAddress)
{
	uint32_t refCounter = 0;

	const std::lock_guard<std::mutex> lock(this->Mutex);

	std::erase_if(this->Registry, [&](ManagedFont_t& font)
	{
		size_t removed = std::erase_if(font.Subscribers, [&](FONTS_RECEIVECALLBACK aCallback)
		{
			return aCallback >= aStartAddress && aCallback <= aEndAddress;
		});

		refCounter += static_cast<uint32_t>(removed);

		if (removed == 0 || !font.Subscribers.empty() || font.IsHostFont) { return false; }

		delete font.Config;
		font.Config = nullptr;

		this->IsFontAtlasBuilt = false;
		return true;
	});

	return refCounter;
}

void CFontManager::AddFontInternal(const char* aIdentifier, float aFontSize, std::span<const uint8_t> aData, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig, bool aUseDeclaredRanges)
{
	if (aData.empty()) { return; }

	std::string str = aIdentifier;

	ManagedFont_t font = this->CreateManagedFont(aIdentifier, aFontSize, aData, aConfig, aUseDeclaredRanges);

	/* add the callback */
	if (aCallback)
	{
		font.Subscribers.push_back(aCallback);
	}

	this->Registry.push_back(font);

	/* invalidate the font atlas to be rebuilt on Advance */
	this->IsFontAtlasBuilt = false;
}

ManagedFont_t CFontManager::CreateManagedFont(std::string aIdentifier, float aFontSize, std::span<const uint8_t> aData, ImFontConfig* aConfig, bool aUseDeclaredRanges)
{
	/* allocate new managed font */
	ManagedFont_t font{};
	font.Identifier = aIdentifier;
	font.Size = aFontSize;
	font.Data = std::make_shared<const std::vector<uint8_t>>(aData.begin(), aData.end());

	/* allocate font config */
	ImFontConfig* config = new ImFontConfig();
	if (aConfig)
	{
		memcpy_s(config, sizeof(ImFontConfig), aConfig, sizeof(ImFontConfig));
	}

	/* overwrite name */
	memset(config->Name, 0, sizeof(config->Name));
	char* p = &config->Name[0];
	snprintf(p, sizeof(config->Name), "%s", aIdentifier.c_str());

	/* overwrite owner status */
	config->FontDataOwnedByAtlas = false;

	if (aUseDeclaredRanges)
	{
		/* Copied, the declared ranges may live in the addon's memory. ImGui's default if none are declared. */
		static const ImWchar s_DefaultRanges[] = { 0x0020, 0x00FF, 0 };
		const ImWchar* declared = config->GlyphRanges ? config->GlyphRanges : s_DefaultRanges;

		std::vector<ImWchar> ranges;
		for (; declared[0] && declared[1]; declared += 2)
		{
			ranges.push_back(declared[0]);
			ranges.push_back(declared[1]);
		}
		ranges.push_back(0);

		font.GlyphRanges = std::make_shared<const std::vector<ImWchar>>(std::move(ranges));
	}
	config->GlyphRanges = nullptr;

	/* set config */
	font.Config = config;

	return font;
}
