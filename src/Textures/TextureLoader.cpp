#include "TextureLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../stb/stb_image.h"
#include <wincodec.h>

#include "../Shared.h"
#include "../Paths.h"
#include "../core.h"

namespace TextureLoader
{
    std::mutex Mutex;
    std::map<std::string, Texture> Registry;

    //void Shutdown();

    Texture Get(std::string aIdentifier)
    {
        Mutex.lock();
        if (Registry.find(aIdentifier) != Registry.end())
        {
            Mutex.unlock();
            return Registry[aIdentifier];
        }

        Mutex.unlock();
        return Texture{};
    }

    Texture LoadFromFile(std::string aIdentifier, std::string aFilename)
    {
        Texture tex = Get(aIdentifier);
        if (tex.Resource != nullptr) { return tex; }

        // Load from disk into a raw RGBA buffer
        int image_width = 0;
        int image_height = 0;
        unsigned char* image_data = stbi_load(aFilename.c_str(), &image_width, &image_height, NULL, 4);

        tex = CreateTexture(aIdentifier, image_data, image_width, image_height);

        stbi_image_free(image_data);

        return tex;
    }

    Texture LoadFromResource(std::string aIdentifier, std::string aName, HMODULE aModule)
    {
        Texture tex = Get(aIdentifier);
        if (tex.Resource != nullptr) { return tex; }

        HRSRC imageResHandle = FindResourceA(aModule, MAKEINTRESOURCEA(aName.c_str()), "PNG");
        if (!imageResHandle)
        {
            return tex;
        }

        HGLOBAL imageResDataHandle = LoadResource(aModule, imageResHandle);
        if (!imageResDataHandle)
        {
            return tex;
        }

        LPVOID imageFile = LockResource(imageResDataHandle);
        if (!imageFile)
        {
            return tex;
        }

        DWORD imageFileSize = SizeofResource(aModule, imageResHandle);
        if (!imageFileSize)
        {
            return tex;
        }

        int image_width = 0;
        int image_height = 0;
        int image_components = 0;
        unsigned char* image_data = stbi_load_from_memory((const stbi_uc*)imageFile, imageFileSize, &image_width, &image_height, &image_components, 0);

        tex = CreateTexture(aIdentifier, image_data, image_width, image_height);

        stbi_image_free(image_data);

        return tex;
    }

    Texture CreateTexture(std::string aIdentifier, unsigned char* aImageData, unsigned aWidth, unsigned aHeight)
    {
        Texture tex{};
        tex.Width = aWidth;
        tex.Height = aHeight;

        // Create texture
        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Width = aWidth;
        desc.Height = aHeight;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;

        ID3D11Texture2D* pTexture = NULL;
        D3D11_SUBRESOURCE_DATA subResource;
        subResource.pSysMem = aImageData;
        subResource.SysMemPitch = desc.Width * 4;
        subResource.SysMemSlicePitch = 0;
        Renderer::Device->CreateTexture2D(&desc, &subResource, &pTexture);

        // Create texture view
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = desc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;

        Renderer::Device->CreateShaderResourceView(pTexture, &srvDesc, &tex.Resource);
        pTexture->Release();

        Registry[aIdentifier] = tex;

        return tex;
    }
}