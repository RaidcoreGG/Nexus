///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  FontManager.cpp
/// Description  :  Handles fonts for the ImGui implementation of the GUI.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "FontManager.h"

#include <algorithm>

#include "core.h"
#include "resource.h"
#include "Shared.h"

#include "imgui/imgui_internal.h"

const char* GetDefaultCompressedFontDataTTFBase85();

namespace FontManager
{
	void ADDONAPI_Get(const char* aIdentifier, FONTS_RECEIVECALLBACK aCallback)
	{
		if (!aCallback) { return; }

		CFontManager& inst = CFontManager::GetInstance();
		inst.Get(aIdentifier, aCallback);
	}

	void ADDONAPI_Release(const char* aIdentifier, FONTS_RECEIVECALLBACK aCallback)
	{
		if (!aCallback) { return; }

		CFontManager& inst = CFontManager::GetInstance();
		inst.Release(aIdentifier, aCallback);
	}

	void ADDONAPI_AddFontFromFile(const char* aIdentifier, float aFontSize, const char* aFilename, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig)
	{
		CFontManager& inst = CFontManager::GetInstance();
		inst.AddFont(aIdentifier, aFontSize, aFilename, aCallback, aConfig);
	}

	void ADDONAPI_AddFontFromResource(const char* aIdentifier, float aFontSize, unsigned aResourceID, HMODULE aModule, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig)
	{
		CFontManager& inst = CFontManager::GetInstance();
		inst.AddFont(aIdentifier, aFontSize, aResourceID, aModule, aCallback, aConfig);
	}

	void ADDONAPI_AddFontFromMemory(const char* aIdentifier, float aFontSize, void* aData, size_t aSize, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig)
	{
		CFontManager& inst = CFontManager::GetInstance();
		inst.AddFont(aIdentifier, aFontSize, aData, aSize, aCallback, aConfig);
	}
}

CFontManager& CFontManager::GetInstance()
{
	static CFontManager Instance;
	return Instance;
}

bool CFontManager::Advance()
{
	if (this->IsFontAtlasBuilt) { return false; }

	ImGuiIO& io = ImGui::GetIO();

	/* build glyph ranges */
	ImVector<ImWchar> ranges;
	ImFontGlyphRangesBuilder rb{};
	ImWchar rangesLatinExt[] =
	{
		0x0100, 0x017F,
		0x0180, 0x024F,
		0,
	};
	rb.AddRanges(io.Fonts->GetGlyphRangesDefault());
	rb.AddRanges(rangesLatinExt);
	rb.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
	rb.AddRanges(io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
	rb.BuildRanges(&ranges);

	io.Fonts->Clear();

	const std::lock_guard<std::mutex> lock(this->Mutex);

	std::vector<std::string> markedForDeletion;
	for (auto& font : this->Registry)
	{
		if (!font.Data && font.DataSize == 0 && font.Subscribers.size() == 0)
		{
			markedForDeletion.push_back(font.Identifier);
		}
	}

	for (auto& identifier : markedForDeletion)
	{
		this->Registry.erase(std::find_if(this->Registry.begin(), this->Registry.end(), [identifier](ManagedFont& font) { return font.Identifier == identifier; }));
	}

	for (auto& font : this->Registry)
	{
		font.Pointer = io.Fonts->AddFontFromMemoryTTF(font.Data, font.DataSize, font.Size, font.Config, ranges.Data);
	}

	/* finally build atlas */
	io.Fonts->Build();

	/* set state */
	this->IsFontAtlasBuilt = true;

	/* finally notify all callbacks with the new fonts */
	this->NotifyCallbacks();
	return true;
}

void CFontManager::Get(const char* aIdentifier, FONTS_RECEIVECALLBACK aCallback)
{
	if (!aCallback) { return; }

	std::string str = aIdentifier;

	const std::lock_guard<std::mutex> lock(this->Mutex);
	auto it = std::find_if(this->Registry.begin(), this->Registry.end(), [str](ManagedFont& font) { return font.Identifier == str; });

	if (it == this->Registry.end()) { return; }

	/* get the reference */
	ManagedFont& font = *it;

	aCallback(aIdentifier, font.Pointer);
}

void CFontManager::Release(const char* aIdentifier, FONTS_RECEIVECALLBACK aCallback)
{
	if (!aCallback) { return; }

	std::string str = aIdentifier;

	const std::lock_guard<std::mutex> lock(this->Mutex);
	auto it = std::find_if(this->Registry.begin(), this->Registry.end(), [str](ManagedFont& font) { return font.Identifier == str; });

	if (it == this->Registry.end()) { return; }

	/* get the reference */
	ManagedFont& font = *it;

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

		this->IsFontAtlasBuilt = false;
	}
}

void CFontManager::AddFont(const char* aIdentifier, float aFontSize, const char* aFilename, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig)
{
	if (aFontSize < 1.0f) { return; }
	if (!aFilename) { return; }

	std::string str = aIdentifier;

	const std::lock_guard<std::mutex> lock(this->Mutex);
	auto it = std::find_if(this->Registry.begin(), this->Registry.end(), [str](ManagedFont& font) { return font.Identifier == str; });

	if (it != this->Registry.end()) /* font already exists */
	{
		/* get the reference */
		ManagedFont& font = *it;

		/* add the callback */
		if (aCallback)
		{
			font.Subscribers.push_back(aCallback);
		}
	}
	else /* create new font */
	{
		/* get file stream */
		//std::ifstream file(aFilename);

		/* abort if failed to open */
		//if (!file.is_open()) { return; }

		/* get filesize */
		//file.seekg(0, std::ios::end);
		//size_t size = file.tellg();
		//size += 1;

		/* allocate buffer and read file */
		//char* buffer = new char[size];
		//file.read(buffer, size);

		size_t size = 0;
		void* buffer = ImFileLoadToMemory(aFilename, "rb", &size, 0);

		/* call this->AddFontInternal with the memory buffer */
		this->AddFontInternal(aIdentifier, aFontSize, buffer, size, aCallback, aConfig);

		/* delete buffer, because it was copied in this->AddFont */
		delete[] buffer;
	}
}

void CFontManager::AddFont(const char* aIdentifier, float aFontSize, unsigned aResourceID, HMODULE aModule, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig)
{
	if (aFontSize < 1.0f) { return; }
	if (aResourceID == 0) { return; }
	if (!aModule) { return; }

	std::string str = aIdentifier;

	const std::lock_guard<std::mutex> lock(this->Mutex);
	auto it = std::find_if(this->Registry.begin(), this->Registry.end(), [str](ManagedFont& font) { return font.Identifier == str; });

	if (it != this->Registry.end()) /* font already exists */
	{
		/* get the reference */
		ManagedFont& font = *it;

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
		GetResource(aModule, MAKEINTRESOURCE(aResourceID), RT_FONT, &buffer, (DWORD*)&size);

		/* call this->AddFontInternal with the memory buffer */
		this->AddFontInternal(aIdentifier, aFontSize, buffer, size, aCallback, aConfig);
	}
}

void CFontManager::AddFont(const char* aIdentifier, float aFontSize, void* aData, size_t aSize, FONTS_RECEIVECALLBACK aCallback, ImFontConfig* aConfig)
{
	if (aFontSize < 1.0f) { return; }
	if (!aData) { return; }
	if (aSize == 0) { return; }

	std::string str = aIdentifier;

	const std::lock_guard<std::mutex> lock(this->Mutex);
	auto it = std::find_if(this->Registry.begin(), this->Registry.end(), [str](ManagedFont& font) { return font.Identifier == str; });

	if (it != this->Registry.end()) /* font already exists */
	{
		/* get the reference */
		ManagedFont& font = *it;

		/* add the callback */
		if (aCallback)
		{
			font.Subscribers.push_back(aCallback);
		}
	}
	else /* create new font */
	{
		/* call this->AddFontInternal with the memory buffer */
		this->AddFontInternal(aIdentifier, aFontSize, aData, aSize, aCallback, aConfig);
	}
}

void CFontManager::AddDefaultFont(FONTS_RECEIVECALLBACK aCallback)
{
	std::string str = "FONT_DEFAULT";

	const std::lock_guard<std::mutex> lock(this->Mutex);
	auto it = std::find_if(this->Registry.begin(), this->Registry.end(), [str](ManagedFont& font) { return font.Identifier == str; });

	if (it != this->Registry.end()) /* font already exists */
	{
		/* get the reference */
		ManagedFont& font = *it;

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
		GetResource(NexusHandle, MAKEINTRESOURCE(RES_FONT_PROGGYCLEAN), RT_FONT, &buffer, (DWORD*)&size);

		/* call this->AddFontInternal with the memory buffer */
		this->AddFontInternal("FONT_DEFAULT", 13.0f, buffer, size, aCallback, nullptr);
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

	/* copy the data to the buffer */
	char* buffer = new char[aSize];
	memcpy_s(buffer, aSize, aData, aSize);

	/* allocate new managed font */
	ManagedFont font{};
	font.Identifier = str;
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
	snprintf(p, sizeof(config->Name), "%s", aIdentifier);

	/* overwrite owner status */
	config->FontDataOwnedByAtlas = false;

	/* set config */
	font.Config = config;

	/* add the callback */
	if (aCallback)
	{
		font.Subscribers.push_back(aCallback);
	}

	this->Registry.push_back(font);

	/* invalidate the font atlas to be rebuilt on this->Advance */
	this->IsFontAtlasBuilt = false;
}

void CFontManager::NotifyCallbacks()
{
	for (auto const& font : this->Registry)
	{
		for (FONTS_RECEIVECALLBACK callback : font.Subscribers)
		{
			callback(font.Identifier.c_str(), font.Pointer);
		}
	}
}
