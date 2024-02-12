#include "TextureLoader.h"

#include <d3d11.h>
#include <vector>
#include <wincodec.h>
#include <filesystem>

#include "Consts.h"
#include "Shared.h"
#include "Paths.h"
#include "core.h"
#include "Renderer.h"

/* For some reason this has to be defined AND included here. */
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "httplib/httplib.h"

namespace TextureLoader
{
	std::mutex Mutex;
	std::map<std::string, Texture*> Registry;

	std::vector<QueuedTexture> QueuedTextures;

	Texture* Get(const char* aIdentifier)
	{
		std::string str = aIdentifier;

		Texture* result = nullptr;

		TextureLoader::Mutex.lock();
		{
			if (Registry.find(str) != Registry.end())
			{
				result = Registry[str];
			}
		}
		TextureLoader::Mutex.unlock();

		return result;
	}

	void LoadFromFile(const char* aIdentifier, const char* aFilename, TEXTURES_RECEIVECALLBACK aCallback)
	{
		std::string str = aIdentifier;

		Texture* tex = Get(str.c_str());
		if (tex != nullptr)
		{
			if (aCallback)
			{
				aCallback(aIdentifier, tex);
			}
			return;
		}

		if (!std::filesystem::exists(aFilename))
		{
			LogWarning(CH_TEXTURES, "File provided does not exist: %s (%s)", aFilename, str.c_str());
			return;
		}

		// Load from disk into a raw RGBA buffer
		int image_width = 0;
		int image_height = 0;
		unsigned char* image_data = stbi_load(aFilename, &image_width, &image_height, NULL, 4);

		QueueTexture(aIdentifier, image_data, image_width, image_height, aCallback);
	}
	void LoadFromResource(const char* aIdentifier, unsigned aResourceID, HMODULE aModule, TEXTURES_RECEIVECALLBACK aCallback)
	{
		std::string str = aIdentifier;

		Texture* tex = Get(str.c_str());
		if (tex != nullptr)
		{
			if (aCallback)
			{
				aCallback(aIdentifier, tex);
			}
			return;
		}

		HRSRC imageResHandle = FindResourceA(aModule, MAKEINTRESOURCEA(aResourceID), "PNG");
		if (!imageResHandle)
		{
			LogDebug(CH_TEXTURES, "Resource not found ResID: %u (%s)", aResourceID, str.c_str());
			return;
		}

		HGLOBAL imageResDataHandle = LoadResource(aModule, imageResHandle);
		if (!imageResDataHandle)
		{
			LogDebug(CH_TEXTURES, "Failed loading resource: %u (%s)", aResourceID, str.c_str());
			return;
		}

		LPVOID imageFile = LockResource(imageResDataHandle);
		if (!imageFile)
		{
			LogDebug(CH_TEXTURES, "Failed locking resource: %u (%s)", aResourceID, str.c_str());
			return;
		}

		DWORD imageFileSize = SizeofResource(aModule, imageResHandle);
		if (!imageFileSize)
		{
			LogDebug(CH_TEXTURES, "Failed getting size of resource: %u (%s)", aResourceID, str.c_str());
			return;
		}

		int image_width = 0;
		int image_height = 0;
		int image_components = 0;
		unsigned char* image_data = stbi_load_from_memory((const stbi_uc*)imageFile, imageFileSize, &image_width, &image_height, &image_components, 0);

		QueueTexture(str.c_str(), image_data, image_width, image_height, aCallback);
	}
	void LoadFromURL(const char* aIdentifier, const char* aRemote, const char* aEndpoint, TEXTURES_RECEIVECALLBACK aCallback)
	{
		std::string str = aIdentifier;

		Texture* tex = Get(str.c_str());
		if (tex != nullptr)
		{
			if (aCallback)
			{
				aCallback(aIdentifier, tex);
			}
			return;
		}

		httplib::Client client(aRemote);
		client.enable_server_certificate_verification(false);
		auto result = client.Get(aEndpoint);

		if (!result)
		{
			LogDebug(CH_TEXTURES, "Error fetching %s%s (%s)", aRemote, aEndpoint, aIdentifier);
			return;
		}

		// Status is not HTTP_OK
		if (result->status != 200)
		{
			LogDebug(CH_TEXTURES, "Status %d when fetching %s%s (%s)", result->status, aRemote, aEndpoint, aIdentifier);
			return;
		}

		size_t size = result->body.size();
		unsigned char* remote_data = new unsigned char[size];
		std::memcpy(remote_data, result->body.c_str(), size);

		int image_width = 0;
		int image_height = 0;
		int comp;
		// TODO: free data
		stbi_uc* data = stbi_load_from_memory(remote_data, static_cast<int>(size), &image_width, &image_height, &comp, 0);

		delete[] remote_data;

		QueueTexture(str.c_str(), data, image_width, image_height, aCallback);
	}
	void LoadFromMemory(const char* aIdentifier, void* aData, size_t aSize, TEXTURES_RECEIVECALLBACK aCallback)
	{
		std::string str = aIdentifier;

		Texture* tex = Get(str.c_str());
		if (tex != nullptr)
		{
			if (aCallback)
			{
				aCallback(aIdentifier, tex);
			}
			return;
		}

		int image_width = 0;
		int image_height = 0;
		int image_components = 0;
		unsigned char* image_data = stbi_load_from_memory((const stbi_uc*)aData, static_cast<int>(aSize), &image_width, &image_height, &image_components, 0);

		QueueTexture(str.c_str(), image_data, image_width, image_height, aCallback);
	}

	void ProcessQueue()
	{
		TextureLoader::Mutex.lock();
		while (TextureLoader::QueuedTextures.size() > 0)
		{
			TextureLoader::CreateTexture(TextureLoader::QueuedTextures.front());
			TextureLoader::QueuedTextures.erase(TextureLoader::QueuedTextures.begin());
		}
		TextureLoader::Mutex.unlock();
	}

	void QueueTexture(const char* aIdentifier, unsigned char* aImageData, unsigned aWidth, unsigned aHeight, TEXTURES_RECEIVECALLBACK aCallback)
	{
		std::string str = aIdentifier;

		//LogDebug(CH_TEXTURES, "Queued %s", str.c_str());

		QueuedTexture raw{};
		raw.Identifier = str;
		raw.Data = aImageData;
		raw.Width = aWidth;
		raw.Height = aHeight;
		raw.Callback = aCallback;

		TextureLoader::Mutex.lock();
		{
			QueuedTextures.push_back(raw);
		}
		TextureLoader::Mutex.unlock();
	}
	void CreateTexture(QueuedTexture aQueuedTexture)
	{
		//LogDebug(CH_TEXTURES, "Create %s", aQueuedTexture.Identifier.c_str());

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

		if (!pTexture)
		{
			//LogDebug(CH_TEXTURES, "pTexture was null");
			stbi_image_free(aQueuedTexture.Data);
			return;
		}

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
			aQueuedTexture.Callback(aQueuedTexture.Identifier.c_str(), tex);
		}

		stbi_image_free(aQueuedTexture.Data);
		aQueuedTexture.Data = {};
	}
}