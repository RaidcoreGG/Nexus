#include "TextureLoader.h"

/* For some reason this has to be defined AND included here. */
#define STB_IMAGE_IMPLEMENTATION
#include "../stb/stb_image.h"

namespace TextureLoader
{
	std::mutex Mutex;
	std::map<std::string, Texture*> Registry;

	std::vector<QueuedTexture> QueuedTextures;

	//void Shutdown();

	Texture* Get(std::string aIdentifier)
	{
		Mutex.lock();
		if (Registry.find(aIdentifier) != Registry.end())
		{
			Mutex.unlock();
			return Registry[aIdentifier];
		}

		Mutex.unlock();
		return nullptr;
	}

	void LoadFromFile(std::string aIdentifier, std::string aFilename, TEXTURES_RECEIVECALLBACK aCallback)
	{
		Texture* tex = Get(aIdentifier);
		if (tex != nullptr) { aCallback(aIdentifier, tex); }

		// Load from disk into a raw RGBA buffer
		int image_width = 0;
		int image_height = 0;
		unsigned char* image_data = stbi_load(aFilename.c_str(), &image_width, &image_height, NULL, 4);

		QueueTexture(aIdentifier, image_data, image_width, image_height, aCallback);
	}

	void LoadFromResource(std::string aIdentifier, unsigned aResourceID, HMODULE aModule, TEXTURES_RECEIVECALLBACK aCallback)
	{
		Texture* tex = Get(aIdentifier);
		if (tex != nullptr) { aCallback(aIdentifier, tex); }

		HRSRC imageResHandle = FindResourceA(aModule, MAKEINTRESOURCEA(aResourceID), "PNG");
		if (!imageResHandle)
		{
			return;
		}

		HGLOBAL imageResDataHandle = LoadResource(aModule, imageResHandle);
		if (!imageResDataHandle)
		{
			return;
		}

		LPVOID imageFile = LockResource(imageResDataHandle);
		if (!imageFile)
		{
			return;
		}

		DWORD imageFileSize = SizeofResource(aModule, imageResHandle);
		if (!imageFileSize)
		{
			return;
		}

		int image_width = 0;
		int image_height = 0;
		int image_components = 0;
		unsigned char* image_data = stbi_load_from_memory((const stbi_uc*)imageFile, imageFileSize, &image_width, &image_height, &image_components, 0);

		QueueTexture(aIdentifier, image_data, image_width, image_height, aCallback);
	}

	void QueueTexture(std::string aIdentifier, unsigned char* aImageData, unsigned aWidth, unsigned aHeight, TEXTURES_RECEIVECALLBACK aCallback)
	{
		LogDebug("Textures", "Queued %s", aIdentifier.c_str());
		Mutex.lock();

		QueuedTexture raw{};
		raw.Identifier = aIdentifier;
		raw.Data = aImageData;
		raw.Width = aWidth;
		raw.Height = aHeight;
		raw.Callback = aCallback;
		QueuedTextures.push_back(raw);

		Mutex.unlock();
	}

	void CreateTexture(QueuedTexture aQueuedTexture)
	{
		LogDebug("Textures", "Create %s", aQueuedTexture.Identifier.c_str());
		Texture* tex = new Texture{};
		tex->Width = aQueuedTexture.Width;
		tex->Height = aQueuedTexture.Height;

		// Create texture
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = aQueuedTexture.Width;
		desc.Height = aQueuedTexture.Height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;

		ID3D11Texture2D* pTexture = NULL;
		D3D11_SUBRESOURCE_DATA subResource{};
		subResource.pSysMem = aQueuedTexture.Data;
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

		Renderer::Device->CreateShaderResourceView(pTexture, &srvDesc, &tex->Resource);
		pTexture->Release();

		Registry[aQueuedTexture.Identifier] = tex;
		if (aQueuedTexture.Callback)
		{
			aQueuedTexture.Callback(aQueuedTexture.Identifier, tex);
		}

		stbi_image_free(aQueuedTexture.Data);
		aQueuedTexture.Data = {};
	}
}