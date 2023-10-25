#include "TextureLoader.h"

/* For some reason this has to be defined AND included here. */
#define STB_IMAGE_IMPLEMENTATION
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "../stb/stb_image.h"
#include "../httplib/httplib.h"

namespace TextureLoader
{
	std::mutex Mutex;
	std::map<std::string, Texture*> Registry;

	std::vector<QueuedTexture> QueuedTextures;

	Texture* Get(const char* aIdentifier)
	{
		std::string str = aIdentifier;

		Texture* result = nullptr;

		Mutex.lock();
		{
			if (Registry.find(str) != Registry.end())
			{
				result = Registry[str];
			}
		}
		Mutex.unlock();

		return result;
	}


	void LoadFromURL(const char* aIdentifier, const char* aRemote, const char* aEndpoint, TEXTURES_RECEIVECALLBACK aCallback) {
		std::string str = aIdentifier;
		httplib::Client client(aRemote);
		client.enable_server_certificate_verification(false);
		auto result = client.Get(aEndpoint);

		if (!result) {
			// TODO: Log error
			return;
		}

		// Status is not HTTP_OK
		if (result->status != 200) {
			// TODO: Log error
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

		free(remote_data);

		QueueTexture(str.c_str(), data, image_width, image_height, aCallback);
	}

	void LoadFromFile(const char* aIdentifier, const char* aFilename, TEXTURES_RECEIVECALLBACK aCallback)
	{
		std::string str = aIdentifier;

		Texture* tex = Get(str.c_str());
		if (tex != nullptr)
		{
			aCallback(aIdentifier, tex);
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
			aCallback(str.c_str(), tex);
			return;
		}

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

		QueueTexture(str.c_str(), image_data, image_width, image_height, aCallback);
	}

	void QueueTexture(const char* aIdentifier, unsigned char* aImageData, unsigned aWidth, unsigned aHeight, TEXTURES_RECEIVECALLBACK aCallback)
	{
		std::string str = aIdentifier;

		LogDebug(CH_TEXTURES, "Queued %s", str.c_str());

		QueuedTexture raw{};
		raw.Identifier = str;
		raw.Data = aImageData;
		raw.Width = aWidth;
		raw.Height = aHeight;
		raw.Callback = aCallback;

		Mutex.lock();
		{
			QueuedTextures.push_back(raw);
		}
		Mutex.unlock();
	}

	void CreateTexture(QueuedTexture aQueuedTexture)
	{
		LogDebug(CH_TEXTURES, "Create %s", aQueuedTexture.Identifier.c_str());
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
			aQueuedTexture.Callback(aQueuedTexture.Identifier.c_str(), tex);
		}

		stbi_image_free(aQueuedTexture.Data);
		aQueuedTexture.Data = {};
	}
}