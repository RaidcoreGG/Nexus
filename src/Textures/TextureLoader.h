#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#include <d3d11.h>
#include <string>
#include <map>
#include <mutex>

#include "../Renderer.h"

struct Texture
{
    unsigned Width;
    unsigned Height;
    ID3D11ShaderResourceView* Resource;
};

typedef Texture (*TEXTURES_GET)(std::string aIdentifier);
typedef Texture (*TEXTURES_LOADFROMFILE)(std::string aIdentifier, std::string aFilename);
typedef Texture (*TEXTURES_LOADFROMRESOURCE)(std::string aIdentifier, std::string aName, HMODULE aModule);

namespace TextureLoader
{
	extern std::mutex Mutex;
	extern std::map<std::string, Texture> Registry;

    //void Shutdown();

    Texture Get(std::string aIdentifier);

    Texture LoadFromFile(std::string aIdentifier, std::string aFilename);
    Texture LoadFromResource(std::string aIdentifier, std::string aName, HMODULE aModule);

    Texture CreateTexture(std::string aIdentifier, unsigned char* aImageData, unsigned aWidth, unsigned aHeight);
}

#endif