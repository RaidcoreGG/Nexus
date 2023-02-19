#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#include <d3d11.h>
#include <string>
#include <map>
#include <mutex>
#include <vector>
#include <wincodec.h>

#include "../Shared.h"
#include "../Paths.h"
#include "../core.h"
#include "../Renderer.h"

#include "FuncDefs.h"
#include "Texture.h"
#include "QueuedTexture.h"

namespace TextureLoader
{
	extern std::mutex Mutex;
	extern std::map<std::string, Texture> Registry;

    extern std::vector<QueuedTexture> QueuedTextures;

    //void Shutdown();

    Texture Get(std::string aIdentifier);

    void LoadFromFile(std::string aIdentifier, std::string aFilename, TEXTURES_RECEIVECALLBACK aCallback);
    void LoadFromResource(std::string aIdentifier, unsigned aResourceID, HMODULE aModule, TEXTURES_RECEIVECALLBACK aCallback);

    void QueueTexture(std::string aIdentifier, unsigned char* aImageData, unsigned aWidth, unsigned aHeight, TEXTURES_RECEIVECALLBACK aCallback);
    void CreateTexture(QueuedTexture aQueuedTexture);
}

#endif