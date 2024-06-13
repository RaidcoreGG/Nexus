///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  TextureLoader.h
/// Description  :  Provides functions to load textures and fetch created textures.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#include <Windows.h>
#include <mutex>
#include <map>
#include <string>
#include <vector>

#include "FuncDefs.h"

#include "Texture.h"
#include "QueuedTexture.h"

constexpr const char* CH_TEXTURES = "Textures";

///----------------------------------------------------------------------------------------------------
/// TextureLoader Namespace
///----------------------------------------------------------------------------------------------------
namespace TextureLoader
{
	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_Get:
	/// 	Addon API wrapper function for Get.
	///----------------------------------------------------------------------------------------------------
	Texture* ADDONAPI_Get(const char* aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_GetOrCreateFromFile:
	/// 	Addon API wrapper function for GetOrCreate from file.
	///----------------------------------------------------------------------------------------------------
	Texture* ADDONAPI_GetOrCreateFromFile(const char* aIdentifier, const char* aFilename);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_GetOrCreateFromResource:
	/// 	Addon API wrapper function for GetOrCreate from embedded resource.
	///----------------------------------------------------------------------------------------------------
	Texture* ADDONAPI_GetOrCreateFromResource(const char* aIdentifier, unsigned aResourceID, HMODULE aModule);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_GetOrCreateFromURL:
	/// 	Addon API wrapper function for GetOrCreate from remote URL.
	///----------------------------------------------------------------------------------------------------
	Texture* ADDONAPI_GetOrCreateFromURL(const char* aIdentifier, const char* aRemote, const char* aEndpoint);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_GetOrCreateFromMemory:
	/// 	Addon API wrapper function for GetOrCreate from memory.
	///----------------------------------------------------------------------------------------------------
	Texture* ADDONAPI_GetOrCreateFromMemory(const char* aIdentifier, void* aData, size_t aSize);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_LoadFromFile:
	/// 	Addon API wrapper function for LoadFromFile.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_LoadFromFile(const char* aIdentifier, const char* aFilename, TEXTURES_RECEIVECALLBACK aCallback);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_LoadFromResource:
	/// 	Addon API wrapper function for LoadFromResource.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_LoadFromResource(const char* aIdentifier, unsigned aResourceID, HMODULE aModule, TEXTURES_RECEIVECALLBACK aCallback);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_LoadFromURL:
	/// 	Addon API wrapper function for LoadFromURL.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_LoadFromURL(const char* aIdentifier, const char* aRemote, const char* aEndpoint, TEXTURES_RECEIVECALLBACK aCallback);

	///----------------------------------------------------------------------------------------------------
	/// ADDONAPI_LoadFromMemory:
	/// 	Addon API wrapper function for LoadFromMemory.
	///----------------------------------------------------------------------------------------------------
	void ADDONAPI_LoadFromMemory(const char* aIdentifier, void* aData, size_t aSize, TEXTURES_RECEIVECALLBACK aCallback);
}

///----------------------------------------------------------------------------------------------------
/// CTextureLoader Class
///----------------------------------------------------------------------------------------------------
class CTextureLoader
{
public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CTextureLoader() = default;
	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CTextureLoader() = default;

	///----------------------------------------------------------------------------------------------------
	/// Advance:
	/// 	Processes all currently queued textures.
	///----------------------------------------------------------------------------------------------------
	void Advance();

	///----------------------------------------------------------------------------------------------------
	/// Get:
	/// 	Returns a Texture* with the given identifier or nullptr.
	///----------------------------------------------------------------------------------------------------
	Texture* Get(const char* aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// GetOrCreate:
	/// 	Returns a Texture* with the given identifier or creates it from file path.
	///----------------------------------------------------------------------------------------------------
	Texture* GetOrCreate(const char* aIdentifier, const char* aFilename);

	///----------------------------------------------------------------------------------------------------
	/// GetOrCreate:
	/// 	Returns a Texture* with the given identifier or creates it from embedded resource.
	///----------------------------------------------------------------------------------------------------
	Texture* GetOrCreate(const char* aIdentifier, unsigned aResourceID, HMODULE aModule);

	///----------------------------------------------------------------------------------------------------
	/// GetOrCreate:
	/// 	Returns a Texture* with the given identifier or creates it from remote URL.
	///----------------------------------------------------------------------------------------------------
	Texture* GetOrCreate(const char* aIdentifier, const char* aRemote, const char* aEndpoint);

	///----------------------------------------------------------------------------------------------------
	/// GetOrCreate:
	/// 	Returns a Texture* with the given identifier or creates it from memory.
	///----------------------------------------------------------------------------------------------------
	Texture* GetOrCreate(const char* aIdentifier, void* aData, size_t aSize);

	///----------------------------------------------------------------------------------------------------
	/// Load:
	/// 	Requests to load a texture from file and returns to the given callback.
	///----------------------------------------------------------------------------------------------------
	void Load(const char* aIdentifier, const char* aFilename, TEXTURES_RECEIVECALLBACK aCallback);

	///----------------------------------------------------------------------------------------------------
	/// Load:
	/// 	Requests to load a texture from an embedded resource and returns to the given callback.
	///----------------------------------------------------------------------------------------------------
	void Load(const char* aIdentifier, unsigned aResourceID, HMODULE aModule, TEXTURES_RECEIVECALLBACK aCallback);

	///----------------------------------------------------------------------------------------------------
	/// Load:
	/// 	Requests to load a texture from remote URL and returns to the given callback.
	///----------------------------------------------------------------------------------------------------
	void Load(const char* aIdentifier, const char* aRemote, const char* aEndpoint, TEXTURES_RECEIVECALLBACK aCallback);

	///----------------------------------------------------------------------------------------------------
	/// Load:
	/// 	Requests to load a texture from memory and returns to the given callback.
	///----------------------------------------------------------------------------------------------------
	void Load(const char* aIdentifier, void* aData, size_t aSize, TEXTURES_RECEIVECALLBACK aCallback);

	///----------------------------------------------------------------------------------------------------
	/// GetRegistry:
	/// 	Returns a copy of the registry.
	///----------------------------------------------------------------------------------------------------
	std::map<std::string, Texture*> GetRegistry() const;

	///----------------------------------------------------------------------------------------------------
	/// GetQueuedTextures:
	/// 	Returns a copy of all currently queued textures.
	///----------------------------------------------------------------------------------------------------
	std::vector<QueuedTexture> GetQueuedTextures() const;

private:
	mutable std::mutex				Mutex;
	std::map<std::string, Texture*>	Registry;
	std::vector<QueuedTexture>		QueuedTextures;

	///----------------------------------------------------------------------------------------------------
	/// OverrideTexture:
	/// 	Internal function to override texture load with custom user texture on disk.
	///----------------------------------------------------------------------------------------------------
	bool OverrideTexture(const char* aIdentifier, TEXTURES_RECEIVECALLBACK aCallback);

	///----------------------------------------------------------------------------------------------------
	/// QueueTexture:
	/// 	Pushes a texture into the queue to load during the next frame.
	///----------------------------------------------------------------------------------------------------
	void QueueTexture(const char* aIdentifier, unsigned char* aImageData, unsigned aWidth, unsigned aHeight, TEXTURES_RECEIVECALLBACK aCallback);

	///----------------------------------------------------------------------------------------------------
	/// CreateTexture:
	/// 	Creates a texture and adds it to the registry.
	///----------------------------------------------------------------------------------------------------
	void CreateTexture(QueuedTexture aQueuedTexture);
};

#endif
