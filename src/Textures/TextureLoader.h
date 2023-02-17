#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#include <d3d11.h>
#include <string>
#include <map>
#include <mutex>
#include <vector>

#include "../Renderer.h"

struct Texture
{
    unsigned Width;
    unsigned Height;
    ID3D11ShaderResourceView* Resource;
};

typedef void    (*TEXTURES_RECEIVECALLBACK)(std::string aIdentifier, Texture aTexture);
typedef Texture (*TEXTURES_GET)(std::string aIdentifier);
typedef void    (*TEXTURES_LOADFROMFILE)(std::string aIdentifier, std::string aFilename, TEXTURES_RECEIVECALLBACK aCallback);
typedef void    (*TEXTURES_LOADFROMRESOURCE)(std::string aIdentifier, std::string aName, HMODULE aModule, TEXTURES_RECEIVECALLBACK aCallback);

struct QueuedTexture
{
    unsigned Width;
    unsigned Height;
    std::string Identifier;
    unsigned char* Data;
    TEXTURES_RECEIVECALLBACK Callback;
};

namespace TextureLoader
{
	extern std::mutex Mutex;
	extern std::map<std::string, Texture> Registry;

    extern std::vector<QueuedTexture> QueuedTextures;

    //void Shutdown();

    Texture Get(std::string aIdentifier);

    void LoadFromFile(std::string aIdentifier, std::string aFilename, TEXTURES_RECEIVECALLBACK aCallback);
    void LoadFromResource(std::string aIdentifier, std::string aName, HMODULE aModule, TEXTURES_RECEIVECALLBACK aCallback);

    void QueueTexture(std::string aIdentifier, unsigned char* aImageData, unsigned aWidth, unsigned aHeight, TEXTURES_RECEIVECALLBACK aCallback);
    void CreateTexture(QueuedTexture aQueuedTexture);
}

#endif