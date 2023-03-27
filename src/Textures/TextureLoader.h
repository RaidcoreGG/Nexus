#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#include <d3d11.h>
#include <string>
#include <map>
#include <mutex>
#include <vector>
#include <wincodec.h>

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

	//void Shutdown();

	/* Returns a Texture* with the given identifier or nullptr. */
	Texture* Get(std::string aIdentifier);

	/* Requests to load a texture from file. */
	void LoadFromFile(std::string aIdentifier, std::string aFilename, TEXTURES_RECEIVECALLBACK aCallback);
	/* Requests to load a texture from an embedded resource. */
	void LoadFromResource(std::string aIdentifier, unsigned aResourceID, HMODULE aModule, TEXTURES_RECEIVECALLBACK aCallback);

	/* Pushes a texture into the queue to load during the next frame. */
	void QueueTexture(std::string aIdentifier, unsigned char* aImageData, unsigned aWidth, unsigned aHeight, TEXTURES_RECEIVECALLBACK aCallback);
	/* Creates a texture and adds it to the registry. */
	void CreateTexture(QueuedTexture aQueuedTexture);
}

#endif