///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  TxLoader.h
/// Description  :  Provides functions to load textures and fetch created textures.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <Windows.h>

#include "TxFuncDefs.h"
#include "TxQueueEntry.h"
#include "TxTexture.h"
#include "Services/Logging/LogApi.h"

constexpr const char* CH_TEXTURES = "Textures";

///----------------------------------------------------------------------------------------------------
/// CTextureLoader Class
///----------------------------------------------------------------------------------------------------
class CTextureLoader
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CTextureLoader(CLogApi* aLogger);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CTextureLoader();

	///----------------------------------------------------------------------------------------------------
	/// Advance:
	/// 	Processes all currently queued textures.
	///----------------------------------------------------------------------------------------------------
	void Advance();

	///----------------------------------------------------------------------------------------------------
	/// Get:
	/// 	Returns a Texture_t* with the given identifier or nullptr.
	///----------------------------------------------------------------------------------------------------
	Texture_t* Get(const char* aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// GetOrCreate:
	/// 	Returns a Texture_t* with the given identifier or creates it from file path.
	///----------------------------------------------------------------------------------------------------
	Texture_t* GetOrCreate(const char* aIdentifier, const char* aFilename);

	///----------------------------------------------------------------------------------------------------
	/// GetOrCreate:
	/// 	Returns a Texture_t* with the given identifier or creates it from embedded resource.
	///----------------------------------------------------------------------------------------------------
	Texture_t* GetOrCreate(const char* aIdentifier, unsigned aResourceID, HMODULE aModule);

	///----------------------------------------------------------------------------------------------------
	/// GetOrCreate:
	/// 	Returns a Texture_t* with the given identifier or creates it from remote URL.
	///----------------------------------------------------------------------------------------------------
	Texture_t* GetOrCreate(const char* aIdentifier, const char* aRemote, const char* aEndpoint);

	///----------------------------------------------------------------------------------------------------
	/// GetOrCreate:
	/// 	Returns a Texture_t* with the given identifier or creates it from memory.
	///----------------------------------------------------------------------------------------------------
	Texture_t* GetOrCreate(const char* aIdentifier, void* aData, size_t aSize);

	///----------------------------------------------------------------------------------------------------
	/// Load:
	/// 	Requests to load a texture from file and returns to the given callback.
	///----------------------------------------------------------------------------------------------------
	void Load(const char* aIdentifier, const char* aFilename, TEXTURES_RECEIVECALLBACK aCallback, bool aIsShadowing = false);

	///----------------------------------------------------------------------------------------------------
	/// Load:
	/// 	Requests to load a texture from an embedded resource and returns to the given callback.
	///----------------------------------------------------------------------------------------------------
	void Load(const char* aIdentifier, unsigned aResourceID, HMODULE aModule, TEXTURES_RECEIVECALLBACK aCallback, bool aIsShadowing = false);

	///----------------------------------------------------------------------------------------------------
	/// Load:
	/// 	Requests to load a texture from remote URL and returns to the given callback.
	///----------------------------------------------------------------------------------------------------
	void Load(const char* aIdentifier, const char* aRemote, const char* aEndpoint, TEXTURES_RECEIVECALLBACK aCallback, bool aIsShadowing = false);

	///----------------------------------------------------------------------------------------------------
	/// Load:
	/// 	Requests to load a texture from memory and returns to the given callback.
	///----------------------------------------------------------------------------------------------------
	void Load(const char* aIdentifier, void* aData, size_t aSize, TEXTURES_RECEIVECALLBACK aCallback, bool aIsShadowing = false);

	///----------------------------------------------------------------------------------------------------
	/// GetRegistry:
	/// 	Returns a copy of the registry.
	///----------------------------------------------------------------------------------------------------
	std::map<std::string, Texture_t*> GetRegistry() const;

	///----------------------------------------------------------------------------------------------------
	/// GetQueuedTextures:
	/// 	Returns a copy of all currently queued textures.
	///----------------------------------------------------------------------------------------------------
	std::map<std::string, QueuedTexture_t> GetQueuedTextures() const;

	///----------------------------------------------------------------------------------------------------
	/// Verify:
	/// 	Removes all TextureReceiver Callbacks that are within the provided address space.
	///----------------------------------------------------------------------------------------------------
	int Verify(void* aStartAddress, void* aEndAddress);

	private:
	CLogApi*                             Logger = nullptr;

	mutable std::mutex                   Mutex;
	std::map<std::string, Texture_t*>      Registry;
	std::map<std::string, QueuedTexture_t> QueuedTextures;

	///----------------------------------------------------------------------------------------------------
	/// ProcessRequest:
	/// 	Processes the load request.
	/// 	Returns true if request is already ongoing or fulfilled.
	/// 	Returns false if the load should be cancelled.
	///----------------------------------------------------------------------------------------------------
	bool ProcessRequest(const char* aIdentifier, TEXTURES_RECEIVECALLBACK aCallback, bool aIsShadowing);

	///----------------------------------------------------------------------------------------------------
	/// ShadowTexture:
	/// 	Renames the given identifier, to an alternative name.
	///----------------------------------------------------------------------------------------------------
	void ShadowTexture(const char* aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// OverrideTexture:
	/// 	Internal function to override texture load with custom user texture on disk.
	/// 	Returns true if an override exists and queues it.
	///----------------------------------------------------------------------------------------------------
	bool OverrideTexture(const char* aIdentifier, TEXTURES_RECEIVECALLBACK aCallback);

	///----------------------------------------------------------------------------------------------------
	/// IsQueued:
	/// 	Returns a true if the given identifier is already queued.
	///----------------------------------------------------------------------------------------------------
	bool IsQueued(const char* aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// Enqueue:
	/// 	Adds a queue to the entry awaiting processing.
	///----------------------------------------------------------------------------------------------------
	void Enqueue(const char* aIdentifier, TEXTURES_RECEIVECALLBACK aCallback);

	///----------------------------------------------------------------------------------------------------
	/// Enqueue:
	/// 	Adds data to a queue entry.
	///----------------------------------------------------------------------------------------------------
	void Enqueue(const char* aIdentifier, unsigned char* aData, int aWidth, int aHeight);

	///----------------------------------------------------------------------------------------------------
	/// Dequeue:
	/// 	Drops a queue entry.
	///----------------------------------------------------------------------------------------------------
	void Dequeue(const char* aIdentifier);

	///----------------------------------------------------------------------------------------------------
	/// CreateTexture:
	/// 	Creates a texture and adds it to the registry.
	///----------------------------------------------------------------------------------------------------
	void CreateTexture(const std::string& aIdentifier, QueuedTexture_t& aQueuedTexture);

	///----------------------------------------------------------------------------------------------------
	/// DispatchTexture:
	/// 	Dispatches a texture.
	///----------------------------------------------------------------------------------------------------
	void DispatchTexture(const std::string& aIdentifier, Texture_t* aTexture, TEXTURES_RECEIVECALLBACK aCallback);
};

#endif
