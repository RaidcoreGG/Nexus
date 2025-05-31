///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  FontManager.cpp
/// Description  :  Handles fonts for the ImGui implementation of the UI.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "FontManager.h"

#include <filesystem>

#include "imgui/imgui_internal.h"

#include "Context.h"
#include "resource.h"
#include "Util/Resources.h"

CFontManager::CFontManager(CLocalization* aLocalization)
{
	this->Language = aLocalization;
}

CFontManager::~CFontManager()
{
	this->Language = nullptr;
}

void CFontManager::Reload()
{
	this->IsFontAtlasBuilt = false;
}

bool CFontManager::Advance()
{
	if (this->IsFontAtlasBuilt) { return false; }

	this->NotifyCallbacks(true);

	ImGuiIO& io = ImGui::GetIO();

	/* add default ranges */
	ImWchar rangesLatinExt[] =
	{
		0x0100, 0x017F,
		0x0180, 0x024F,
		0,
	};
	ImVector<ImWchar> ranges;
	ImFontGlyphRangesBuilder rb{};
	rb.AddRanges(io.Fonts->GetGlyphRangesDefault());
	rb.AddRanges(rangesLatinExt);

	ImVector<ImWchar> rangesExt;
	ImFontGlyphRangesBuilder rbExt{};
	rbExt.AddRanges(io.Fonts->GetGlyphRangesDefault());
	rbExt.AddRanges(rangesLatinExt);
	rbExt.AddRanges(io.Fonts->GetGlyphRangesChineseFull());
	rbExt.AddRanges(io.Fonts->GetGlyphRangesCyrillic());

	/* add ranges on demand*/
	for (const char* str : this->Language->GetAllTexts())
	{
		rb.AddText(str);
		rbExt.AddText(str);
	}

	/* build ranges */
	rb.BuildRanges(&ranges);
	rbExt.BuildRanges(&rangesExt);

	io.Fonts->Clear();

	const std::lock_guard<std::mutex> lock(this->Mutex);
	for (auto& font : this->Registry)
	{
		if (font.Identifier == "FONT_DEFAULT")
		{
			font.Pointer = io.Fonts->AddFontFromMemoryTTF(font.Data, static_cast<int>(font.DataSize), font.Size, font.Config, rangesExt.Data);
		}
		else
		{
			font.Pointer = io.Fonts->AddFontFromMemoryTTF(font.Data, static_cast<int>(font.DataSize), font.Size, font.Config, ranges.Data);
		}
	}

	/* finally build atlas */
	io.Fonts->Build();

	/* set state */
	this->IsFontAtlasBuilt = true;

	/* finally notify all callbacks with the new fonts */
	this->NotifyCallbacks();
	return true;
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
	if (font.Subscribers.size() == 0)
	{
		delete font.Data;
		font.Data = nullptr;
		font.DataSize = 0;

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

		/* call AddFontInternal with the memory buffer */
		this->AddFontInternal(aIdentifier, aFontSize, buffer, size, aCallback, aConfig);

		/* delete buffer, because it was copied in AddFont */
		delete[] buffer;
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
		/* out vars */
		void* buffer = nullptr;
		size_t size = 0;

		/* get data */
		Resources::Get(aModule, MAKEINTRESOURCE(aResourceID), RT_FONT, &buffer, (DWORD*)&size);

		/* call AddFontInternal with the memory buffer */
		this->AddFontInternal(aIdentifier, aFontSize, buffer, size, aCallback, aConfig);
	}
}

void CFontManager::AddFont(const char* aIdentifier, float aFontSize, void* aData, size_t aSize, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig)
{
	if (aFontSize < 1.0f) { return; }
	if (!aData) { return; }
	if (aSize == 0) { return; }

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
		this->AddFontInternal(aIdentifier, aFontSize, aData, aSize, aCallback, aConfig);
	}
}

void CFontManager::ReplaceFont(const char* aIdentifier, float aFontSize, const char* aFilename, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig)
{
	if (aFontSize < 1.0f) { return; }
	if (!aFilename) { return; }
	if (!std::filesystem::exists(aFilename)) { return; }

	std::string str = aIdentifier;

	const std::lock_guard<std::mutex> lock(this->Mutex);
	auto it = std::find_if(this->Registry.begin(), this->Registry.end(), [str](ManagedFont_t& font) { return font.Identifier == str; });

	size_t size = 0;
	void* buffer = ImFileLoadToMemory(aFilename, "rb", &size, 0);

	if (it != this->Registry.end()) /* font already exists */
	{
		/* get the reference */
		ManagedFont_t& oldFont = *it;

		/* free the data*/
		delete oldFont.Data;
		oldFont.Data = nullptr;
		oldFont.DataSize = 0;

		/* overwrite config if one was passed */
		if (aConfig)
		{
			delete oldFont.Config;
			oldFont.Config = nullptr;
		}
		else
		{
			aConfig = oldFont.Config;
		}

		ManagedFont_t font = this->CreateManagedFont(aIdentifier, aFontSize, buffer, size, aConfig);
		font.Subscribers = oldFont.Subscribers;

		*it = font;

		this->IsFontAtlasBuilt = false;
	}
	else
	{
		/* call AddFontInternal with the memory buffer */
		this->AddFontInternal(aIdentifier, aFontSize, buffer, size, aCallback, aConfig);
	}

	/* delete buffer, because it was copied in AddFont */
	delete[] buffer;
}

void CFontManager::ReplaceFont(const char* aIdentifier, float aFontSize, unsigned aResourceID, HMODULE aModule, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig)
{
	if (aFontSize < 1.0f) { return; }
	if (aResourceID == 0) { return; }
	if (!aModule) { return; }

	std::string str = aIdentifier;

	const std::lock_guard<std::mutex> lock(this->Mutex);
	auto it = std::find_if(this->Registry.begin(), this->Registry.end(), [str](ManagedFont_t& font) { return font.Identifier == str; });

	/* out vars */
	void* buffer = nullptr;
	size_t size = 0;

	/* get data */
	Resources::Get(aModule, MAKEINTRESOURCE(aResourceID), RT_FONT, &buffer, (DWORD*)&size);

	if (it != this->Registry.end()) /* font already exists */
	{
		/* get the reference */
		ManagedFont_t& oldFont = *it;

		/* free the data*/
		delete oldFont.Data;
		oldFont.Data = nullptr;
		oldFont.DataSize = 0;

		/* overwrite config if one was passed */
		if (aConfig)
		{
			delete oldFont.Config;
			oldFont.Config = nullptr;
		}
		else
		{
			aConfig = oldFont.Config;
		}

		ManagedFont_t font = this->CreateManagedFont(aIdentifier, aFontSize, buffer, size, aConfig);
		font.Subscribers = oldFont.Subscribers;

		*it = font;

		this->IsFontAtlasBuilt = false;
	}
	else
	{
		/* call AddFontInternal with the memory buffer */
		this->AddFontInternal(aIdentifier, aFontSize, buffer, size, aCallback, aConfig);
	}
}

void CFontManager::ReplaceFont(const char* aIdentifier, float aFontSize, void* aData, size_t aSize, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig)
{
	if (aFontSize < 1.0f) { return; }
	if (!aData) { return; }
	if (aSize == 0) { return; }

	std::string str = aIdentifier;

	const std::lock_guard<std::mutex> lock(this->Mutex);
	auto it = std::find_if(this->Registry.begin(), this->Registry.end(), [str](ManagedFont_t& font) { return font.Identifier == str; });

	if (it != this->Registry.end()) /* font already exists */
	{
		/* get the reference */
		ManagedFont_t& oldFont = *it;

		/* free the data*/
		delete oldFont.Data;
		oldFont.Data = nullptr;
		oldFont.DataSize = 0;

		/* overwrite config if one was passed */
		if (aConfig)
		{
			delete oldFont.Config;
			oldFont.Config = nullptr;
		}
		else
		{
			aConfig = oldFont.Config;
		}

		ManagedFont_t font = this->CreateManagedFont(aIdentifier, aFontSize, aData, aSize, aConfig);
		font.Subscribers = oldFont.Subscribers;

		*it = font;

		this->IsFontAtlasBuilt = false;
	}
	else
	{
		/* call AddFontInternal with the memory buffer */
		this->AddFontInternal(aIdentifier, aFontSize, aData, aSize, aCallback, aConfig);
	}
}

void CFontManager::AddDefaultFont(FONTS_RECEIVECALLBACK aCallback)
{
	std::string str = "FONT_DEFAULT";

	const std::lock_guard<std::mutex> lock(this->Mutex);
	auto it = std::find_if(this->Registry.begin(), this->Registry.end(), [str](ManagedFont_t& font) { return font.Identifier == str; });

	if (it != this->Registry.end()) /* font already exists */
	{
		/* get the reference */
		ManagedFont_t& font = *it;

		/* add the callback */
		if (aCallback)
		{
			font.Subscribers.push_back(aCallback);
		}
	}
	else /* create new font */
	{
		/* out vars */
		void* buffer = nullptr;
		size_t size = 0;

		/* get data */
		CContext* ctx = CContext::GetContext();
		Resources::Get(ctx->GetModule(), MAKEINTRESOURCE(RES_FONT_INTER), RT_FONT, &buffer, (DWORD*)&size);

		/* call AddFontInternal with the memory buffer */
		this->AddFontInternal("FONT_DEFAULT", 15.0f, buffer, size, aCallback, nullptr);
	}
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

int CFontManager::Verify(void* aStartAddress, void* aEndAddress)
{
	int refCounter = 0;

	const std::lock_guard<std::mutex> lock(this->Mutex);

	for (auto& font : this->Registry)
	{
		for (FONTS_RECEIVECALLBACK callback : font.Subscribers)
		{
			if (callback >= aStartAddress && callback <= aEndAddress)
			{
				font.Subscribers.erase(std::remove(font.Subscribers.begin(), font.Subscribers.end(), callback), font.Subscribers.end());
				refCounter++;
			}
		}

		if (font.Subscribers.size() == 0)
		{
			delete font.Data;
			font.Data = nullptr;
			font.DataSize = 0;

			delete font.Config;
			font.Config = nullptr;

			this->IsFontAtlasBuilt = false;
		}
	}

	return refCounter;
}

void CFontManager::AddFontInternal(const char* aIdentifier, float aFontSize, void* aData, size_t aSize, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig)
{
	if (!aData) { return; }
	if (aSize == 0) { return; }

	std::string str = aIdentifier;

	ManagedFont_t font = this->CreateManagedFont(aIdentifier, aFontSize, aData, aSize, aConfig);

	/* add the callback */
	if (aCallback)
	{
		font.Subscribers.push_back(aCallback);
	}

	this->Registry.push_back(font);

	/* invalidate the font atlas to be rebuilt on Advance */
	this->IsFontAtlasBuilt = false;
}

void CFontManager::NotifyCallbacks(bool aNotifyNull)
{
	for (auto const& font : this->Registry)
	{
		for (FONTS_RECEIVECALLBACK callback : font.Subscribers)
		{
			callback(font.Identifier.c_str(), aNotifyNull ? nullptr : font.Pointer);
		}
	}
}

ManagedFont_t CFontManager::CreateManagedFont(std::string aIdentifier, float aFontSize, void* aData, size_t aSize, ImFontConfig* aConfig)
{
	/* copy the data to the buffer */
	char* buffer = new char[aSize];
	memcpy_s(buffer, aSize, aData, aSize);

	/* allocate new managed font */
	ManagedFont_t font{};
	font.Identifier = aIdentifier;
	font.Size = aFontSize;
	font.Data = buffer;
	font.DataSize = aSize;

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

	/* set config */
	font.Config = config;

	return font;
}
