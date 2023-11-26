#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#include <d3d11.h>
#include <string>
#include <map>
#include <mutex>
#include <vector>
#include <wincodec.h>
#include <filesystem>

#include "../Consts.h"
#include "../Shared.h"
#include "../Paths.h"
#include "../core.h"
#include "../Renderer.h"

#include "FuncDefs.h"
#include "Texture.h"
#include "QueuedTexture.h"

namespace TextureLoader
{
	extern std::mutex						Mutex;
	extern std::map<std::string, Texture*>	Registry;
	extern std::vector<QueuedTexture>		QueuedTextures;

	/* Returns a Texture* with the given identifier or nullptr. */
	Texture* Get(const char* aIdentifier);

	/* Requests to load a texture from file. */
	void LoadFromFile(const char* aIdentifier, const char* aFilename, TEXTURES_RECEIVECALLBACK aCallback);
	/* Requests to load a texture from an embedded resource. */
	void LoadFromResource(const char* aIdentifier, unsigned aResourceID, HMODULE aModule, TEXTURES_RECEIVECALLBACK aCallback);
	/* Requests to load a texture from URL. */
	void LoadFromURL(const char* aIdentifier, const char* aRemote, const char* aEndpoint, TEXTURES_RECEIVECALLBACK aCallback);
	/* Requests to load a texture from memory. */
	void LoadFromMemory(const char* aIdentifier, void* aData, size_t aSize, TEXTURES_RECEIVECALLBACK aCallback);

	/* Processes all currently queued textures. */
	void ProcessQueue();

	/* Pushes a texture into the queue to load during the next frame. */
	void QueueTexture(const char* aIdentifier, unsigned char* aImageData, unsigned aWidth, unsigned aHeight, TEXTURES_RECEIVECALLBACK aCallback);
	/* Creates a texture and adds it to the registry. */
	void CreateTexture(QueuedTexture aQueuedTexture);
}

#endif