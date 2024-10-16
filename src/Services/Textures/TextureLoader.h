///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  TextureLoader.h
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

#include "FuncDefs.h"
#include "QueuedTexture.h"
#include "Services/Logging/LogHandler.h"
#include "Texture.h"

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
	CTextureLoader(CLogHandler* aLogger);
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
	std::map<std::string, Texture*> GetRegistry() const;

	///----------------------------------------------------------------------------------------------------
	/// GetQueuedTextures:
	/// 	Returns a copy of all currently queued textures.
	///----------------------------------------------------------------------------------------------------
	std::vector<QueuedTexture> GetQueuedTextures() const;

	///----------------------------------------------------------------------------------------------------
	/// Verify:
	/// 	Removes all TextureReceiver Callbacks that are within the provided address space.
	///----------------------------------------------------------------------------------------------------
	int Verify(void* aStartAddress, void* aEndAddress);

	private:
	CLogHandler*                                 Logger         = nullptr;

	mutable std::mutex                           Mutex;
	std::map<std::string, Texture*>              Registry;
	std::vector<QueuedTexture>                   QueuedTextures;

	struct StagedTextureCallback
	{
		TEXTURES_RECEIVECALLBACK                 Callback;
		bool                                     IsValid;
	};
	std::map<std::string, StagedTextureCallback> PendingCallbacks; /* set to false if no longer valid */

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
