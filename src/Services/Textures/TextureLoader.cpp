///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  TextureLoader.cpp
/// Description  :  Provides functions to load textures and fetch created textures.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Services/Textures/TextureLoader.h"

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
	Texture* ADDONAPI_Get(const char* aIdentifier)
	{
		return TextureService->Get(aIdentifier);
	}

	Texture* ADDONAPI_GetOrCreateFromFile(const char* aIdentifier, const char* aFilename)
	{
		return TextureService->GetOrCreate(aIdentifier, aFilename);
	}
	
	Texture* ADDONAPI_GetOrCreateFromResource(const char* aIdentifier, unsigned aResourceID, HMODULE aModule)
	{
		return TextureService->GetOrCreate(aIdentifier, aResourceID, aModule);;
	}
	
	Texture* ADDONAPI_GetOrCreateFromURL(const char* aIdentifier, const char* aRemote, const char* aEndpoint)
	{
		return TextureService->GetOrCreate(aIdentifier, aRemote, aEndpoint);
	}
	
	Texture* ADDONAPI_GetOrCreateFromMemory(const char* aIdentifier, void* aData, size_t aSize)
	{
		return TextureService->GetOrCreate(aIdentifier, aData, aSize);
	}

	void ADDONAPI_LoadFromFile(const char* aIdentifier, const char* aFilename, TEXTURES_RECEIVECALLBACK aCallback)
	{
		TextureService->Load(aIdentifier, aFilename, aCallback);
	}
	
	void ADDONAPI_LoadFromResource(const char* aIdentifier, unsigned aResourceID, HMODULE aModule, TEXTURES_RECEIVECALLBACK aCallback)
	{
		TextureService->Load(aIdentifier, aResourceID, aModule, aCallback);
	}
	
	void ADDONAPI_LoadFromURL(const char* aIdentifier, const char* aRemote, const char* aEndpoint, TEXTURES_RECEIVECALLBACK aCallback)
	{
		TextureService->Load(aIdentifier, aRemote, aEndpoint, aCallback);
	}
	
	void ADDONAPI_LoadFromMemory(const char* aIdentifier, void* aData, size_t aSize, TEXTURES_RECEIVECALLBACK aCallback)
	{
		TextureService->Load(aIdentifier, aData, aSize, aCallback);
	}
}

void CTextureLoader::Advance()
{
	const std::lock_guard<std::mutex> lock(Mutex);
	while (this->QueuedTextures.size() > 0)
	{
		this->CreateTexture(this->QueuedTextures.front());
		this->QueuedTextures.erase(this->QueuedTextures.begin());
	}
}

Texture* CTextureLoader::Get(const char* aIdentifier)
{
	std::string str = aIdentifier;

	Texture* result = nullptr;

	{
		const std::lock_guard<std::mutex> lock(Mutex);
		if (this->Registry.find(str) != this->Registry.end())
		{
			result = this->Registry[str];
		}
	}

	return result;
}

Texture* CTextureLoader::GetOrCreate(const char* aIdentifier, const char* aFilename)
{
	Texture* result = Get(aIdentifier);

	if (this->OverrideTexture(aIdentifier, nullptr))
	{
		return result;
	}
	else if (!result)
	{
		this->Load(aIdentifier, aFilename, nullptr);
	}

	return result;
}

Texture* CTextureLoader::GetOrCreate(const char* aIdentifier, unsigned aResourceID, HMODULE aModule)
{
	Texture* result = this->Get(aIdentifier);

	if (this->OverrideTexture(aIdentifier, nullptr))
	{
		return result;
	}
	else if (!result)
	{
		this->Load(aIdentifier, aResourceID, aModule, nullptr);
	}

	return result;
}

Texture* CTextureLoader::GetOrCreate(const char* aIdentifier, const char* aRemote, const char* aEndpoint)
{
	Texture* result = this->Get(aIdentifier);

	if (this->OverrideTexture(aIdentifier, nullptr))
	{
		return result;
	}
	else if (!result)
	{
		this->Load(aIdentifier, aRemote, aEndpoint, nullptr);
	}

	return result;
}

Texture* CTextureLoader::GetOrCreate(const char* aIdentifier, void* aData, size_t aSize)
{
	Texture* result = this->Get(aIdentifier);

	if (this->OverrideTexture(aIdentifier, nullptr))
	{
		return result;
	}
	else if (!result)
	{
		this->Load(aIdentifier, aData, aSize, nullptr);
	}

	return result;
}

void CTextureLoader::Load(const char* aIdentifier, const char* aFilename, TEXTURES_RECEIVECALLBACK aCallback)
{
	//LogInfo(CH_TEXTURES, "this->LoadFromFile(aIdentifier: %s, aFilename: %s, aCallback: %p)", aIdentifier, aFilename, aCallback);

	std::string str = aIdentifier;

	Texture* tex = this->Get(str.c_str());
	if (tex != nullptr)
	{
		if (aCallback)
		{
			aCallback(aIdentifier, tex);
		}
		return;
	}
	else if (this->OverrideTexture(aIdentifier, aCallback))
	{
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

	this->QueueTexture(aIdentifier, image_data, image_width, image_height, aCallback);
}

void CTextureLoader::Load(const char* aIdentifier, unsigned aResourceID, HMODULE aModule, TEXTURES_RECEIVECALLBACK aCallback)
{
	//LogInfo(CH_TEXTURES, "this->LoadFromResource(aIdentifier: %s, aResourceID: %u, aModule: %p, aCallback: %p)", aIdentifier, aResourceID, aModule, aCallback);

	std::string str = aIdentifier;

	Texture* tex = this->Get(str.c_str());
	if (tex != nullptr)
	{
		if (aCallback)
		{
			aCallback(aIdentifier, tex);
		}
		return;
	}
	else if (this->OverrideTexture(aIdentifier, aCallback))
	{
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
	unsigned char* image_data = stbi_load_from_memory((const stbi_uc*)imageFile, imageFileSize, &image_width, &image_height, &image_components, 4);

	this->QueueTexture(str.c_str(), image_data, image_width, image_height, aCallback);
}

void CTextureLoader::Load(const char* aIdentifier, const char* aRemote, const char* aEndpoint, TEXTURES_RECEIVECALLBACK aCallback)
{
	//LogInfo(CH_TEXTURES, "this->LoadFromURL(aIdentifier: %s, aRemote: %s, aEndpoint: %s, aCallback: %p)", aIdentifier, aRemote, aEndpoint, aCallback);

	std::string str = aIdentifier;

	Texture* tex = this->Get(str.c_str());
	if (tex != nullptr)
	{
		if (aCallback)
		{
			aCallback(aIdentifier, tex);
		}
		return;
	}
	else if (this->OverrideTexture(aIdentifier, aCallback))
	{
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
	int image_components;

	stbi_uc* data = stbi_load_from_memory(remote_data, static_cast<int>(size), &image_width, &image_height, &image_components, 4);

	delete[] remote_data;

	this->QueueTexture(str.c_str(), data, image_width, image_height, aCallback);
}

void CTextureLoader::Load(const char* aIdentifier, void* aData, size_t aSize, TEXTURES_RECEIVECALLBACK aCallback)
{
	//LogInfo(CH_TEXTURES, "this->LoadFromMemory(aIdentifier: %s, aData: %p, aSize: %u, aCallback: %p)", aIdentifier, aData, aSize, aCallback);

	std::string str = aIdentifier;

	Texture* tex = this->Get(str.c_str());
	if (tex != nullptr)
	{
		if (aCallback)
		{
			aCallback(aIdentifier, tex);
		}
		return;
	}
	else if (this->OverrideTexture(aIdentifier, aCallback))
	{
		return;
	}

	int image_width = 0;
	int image_height = 0;
	int image_components = 0;
	unsigned char* image_data = stbi_load_from_memory((const stbi_uc*)aData, static_cast<int>(aSize), &image_width, &image_height, &image_components, 4);

	this->QueueTexture(str.c_str(), image_data, image_width, image_height, aCallback);
}

bool CTextureLoader::OverrideTexture(const char* aIdentifier, TEXTURES_RECEIVECALLBACK aCallback)
{
	std::string file = aIdentifier;
	file.append(".png");
	std::filesystem::path customPath = Path::D_GW2_ADDONS_NEXUS / "Textures" / file.c_str();

	if (std::filesystem::exists(customPath))
	{
		// Load from disk into a raw RGBA buffer
		int image_width = 0;
		int image_height = 0;
		unsigned char* image_data = stbi_load(customPath.string().c_str(), &image_width, &image_height, NULL, 4);

		this->QueueTexture(aIdentifier, image_data, image_width, image_height, aCallback);
		return true;
	}
	return false;
}

void CTextureLoader::QueueTexture(const char* aIdentifier, unsigned char* aImageData, unsigned aWidth, unsigned aHeight, TEXTURES_RECEIVECALLBACK aCallback)
{
	std::string str = aIdentifier;

	{
		const std::lock_guard<std::mutex> lock(this->Mutex);
		for (QueuedTexture& tex : this->QueuedTextures)
		{
			if (tex.Identifier == str)
			{
				return;
			}
		}
	}

	//LogDebug(CH_TEXTURES, "Queued %s", str.c_str());

	QueuedTexture raw{};
	raw.Identifier = str;
	raw.Data = aImageData;
	raw.Width = aWidth;
	raw.Height = aHeight;
	raw.Callback = aCallback;

	{
		const std::lock_guard<std::mutex> lock(this->Mutex);
		this->QueuedTextures.push_back(raw);
	}
}

void CTextureLoader::CreateTexture(QueuedTexture aQueuedTexture)
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
		LogDebug(CH_TEXTURES, "pTexture was null");
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

	this->Registry[aQueuedTexture.Identifier] = tex;
	if (aQueuedTexture.Callback)
	{
		aQueuedTexture.Callback(aQueuedTexture.Identifier.c_str(), tex);
	}

	stbi_image_free(aQueuedTexture.Data);
	aQueuedTexture.Data = {};
}
