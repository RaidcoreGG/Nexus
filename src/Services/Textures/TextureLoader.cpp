///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  TextureLoader.cpp
/// Description  :  Provides functions to load textures and fetch created textures.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Services/Textures/TextureLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "httplib/httplib.h"

#include <d3d11.h>
#include <filesystem>
#include <vector>

#include "Index.h"
#include "Renderer.h"

CTextureLoader::CTextureLoader(CLogHandler* aLogger)
{
	assert(aLogger);

	this->Logger = aLogger;
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

void CTextureLoader::Load(const char* aIdentifier, const char* aFilename, TEXTURES_RECEIVECALLBACK aCallback, bool aIsShadowing)
{
	//this->Logger->Info(CH_TEXTURES, "this->LoadFromFile(aIdentifier: %s, aFilename: %s, aCallback: %p)", aIdentifier, aFilename, aCallback);

	std::string str = aIdentifier;

	Texture* tex = aIsShadowing ? nullptr : this->Get(str.c_str());
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
		this->Logger->Warning(CH_TEXTURES, "File provided does not exist: %s (%s)", aFilename, str.c_str());

		/* nullptr response on fail */
		if (aCallback)
		{
			aCallback(str.c_str(), nullptr);
		}

		return;
	}

	// Load from disk into a raw RGBA buffer
	int image_width = 0;
	int image_height = 0;
	unsigned char* image_data = stbi_load(aFilename, &image_width, &image_height, NULL, 4);

	this->QueueTexture(aIdentifier, image_data, image_width, image_height, aCallback);
}

void CTextureLoader::Load(const char* aIdentifier, unsigned aResourceID, HMODULE aModule, TEXTURES_RECEIVECALLBACK aCallback, bool aIsShadowing)
{
	//this->Logger->Info(CH_TEXTURES, "this->LoadFromResource(aIdentifier: %s, aResourceID: %u, aModule: %p, aCallback: %p)", aIdentifier, aResourceID, aModule, aCallback);

	std::string str = aIdentifier;

	Texture* tex = aIsShadowing ? nullptr : this->Get(str.c_str());
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
		this->Logger->Debug(CH_TEXTURES, "Resource not found ResID: %u (%s)", aResourceID, str.c_str());

		/* nullptr response on fail */
		if (aCallback)
		{
			aCallback(str.c_str(), nullptr);
		}

		return;
	}

	HGLOBAL imageResDataHandle = LoadResource(aModule, imageResHandle);
	if (!imageResDataHandle)
	{
		this->Logger->Debug(CH_TEXTURES, "Failed loading resource: %u (%s)", aResourceID, str.c_str());

		/* nullptr response on fail */
		if (aCallback)
		{
			aCallback(str.c_str(), nullptr);
		}

		return;
	}

	LPVOID imageFile = LockResource(imageResDataHandle);
	if (!imageFile)
	{
		this->Logger->Debug(CH_TEXTURES, "Failed locking resource: %u (%s)", aResourceID, str.c_str());

		/* nullptr response on fail */
		if (aCallback)
		{
			aCallback(str.c_str(), nullptr);
		}

		return;
	}

	DWORD imageFileSize = SizeofResource(aModule, imageResHandle);
	if (!imageFileSize)
	{
		this->Logger->Debug(CH_TEXTURES, "Failed getting size of resource: %u (%s)", aResourceID, str.c_str());

		/* nullptr response on fail */
		if (aCallback)
		{
			aCallback(str.c_str(), nullptr);
		}

		return;
	}

	int image_width = 0;
	int image_height = 0;
	int image_components = 0;
	unsigned char* image_data = stbi_load_from_memory((const stbi_uc*)imageFile, imageFileSize, &image_width, &image_height, &image_components, 4);

	this->QueueTexture(str.c_str(), image_data, image_width, image_height, aCallback);
}

void CTextureLoader::Load(const char* aIdentifier, const char* aRemote, const char* aEndpoint, TEXTURES_RECEIVECALLBACK aCallback, bool aIsShadowing)
{
	//this->Logger->Info(CH_TEXTURES, "this->LoadFromURL(aIdentifier: %s, aRemote: %s, aEndpoint: %s, aCallback: %p)", aIdentifier, aRemote, aEndpoint, aCallback);

	std::string str = aIdentifier;

	Texture* tex = aIsShadowing ? nullptr : this->Get(str.c_str());
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
	
	{
		const std::lock_guard<std::mutex> lock(this->Mutex);
		this->PendingCallbacks[str] = StagedTextureCallback{ aCallback, true };
	}

	std::string remote = aRemote;
	std::string endpoint = aEndpoint;

	std::thread([this, str, remote, endpoint, aCallback]() {
		httplib::Client client(remote);
		client.enable_server_certificate_verification(false);
		auto result = client.Get(endpoint);

		if (!result)
		{
			this->Logger->Debug(CH_TEXTURES, "Error fetching %s%s (%s)\nError: %s", remote.c_str(), endpoint.c_str(), str.c_str(), httplib::to_string(result.error()).c_str());

			if (aCallback)
			{
				const std::lock_guard<std::mutex> lock(this->Mutex);
				if (this->PendingCallbacks[str].IsValid == false)
				{
					this->PendingCallbacks.erase(str);
				}
				else
				{
					aCallback(str.c_str(), nullptr);
				}
			}

			return;
		}

		// Status is not HTTP_OK
		if (result->status != 200)
		{
			this->Logger->Debug(CH_TEXTURES, "Status %d when fetching %s%s (%s) | %s", result->status, remote.c_str(), endpoint.c_str(), str.c_str(), httplib::to_string(result.error()).c_str());
			
			/* nullptr response on fail */
			if (aCallback)
			{
				const std::lock_guard<std::mutex> lock(this->Mutex);
				if (this->PendingCallbacks[str].IsValid == false)
				{
					this->PendingCallbacks.erase(str);
				}
				else
				{
					aCallback(str.c_str(), nullptr);
				}
			}

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

		{
			const std::lock_guard<std::mutex> lock(this->Mutex);
			if (this->PendingCallbacks[str].IsValid == false)
			{
				this->PendingCallbacks.erase(str);
				if (data)
				{
					stbi_image_free(data);
				}
				return;
			}
		}

		this->QueueTexture(str.c_str(), data, image_width, image_height, aCallback);
	}).detach();
}

void CTextureLoader::Load(const char* aIdentifier, void* aData, size_t aSize, TEXTURES_RECEIVECALLBACK aCallback, bool aIsShadowing)
{
	//this->Logger->Info(CH_TEXTURES, "this->LoadFromMemory(aIdentifier: %s, aData: %p, aSize: %u, aCallback: %p)", aIdentifier, aData, aSize, aCallback);

	std::string str = aIdentifier;

	Texture* tex = aIsShadowing ? nullptr : this->Get(str.c_str());
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

std::map<std::string, Texture*> CTextureLoader::GetRegistry() const
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	return this->Registry;
}

std::vector<QueuedTexture> CTextureLoader::GetQueuedTextures() const
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	return this->QueuedTextures;
}

int CTextureLoader::Verify(void* aStartAddress, void* aEndAddress)
{
	int refCounter = 0;

	const std::lock_guard<std::mutex> lock(this->Mutex);

	for (QueuedTexture& qTex : this->QueuedTextures)
	{
		if (qTex.Callback >= aStartAddress && qTex.Callback <= aEndAddress)
		{
			qTex.Callback = nullptr;
			refCounter++;
		}
	}

	for (auto [identifier, callback] : this->PendingCallbacks)
	{
		if (callback.Callback >= aStartAddress && callback.Callback <= aEndAddress)
		{
			this->PendingCallbacks[identifier].IsValid = false;
			refCounter++;
		}
	}

	return refCounter;
}

bool CTextureLoader::OverrideTexture(const char* aIdentifier, TEXTURES_RECEIVECALLBACK aCallback)
{
	std::string file = aIdentifier;
	file.append(".png");
	std::filesystem::path customPath = Index::D_GW2_ADDONS_NEXUS / "Textures" / file.c_str();

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

	const std::lock_guard<std::mutex> lock(this->Mutex);

	for (QueuedTexture& tex : this->QueuedTextures)
	{
		if (tex.Identifier == str)
		{
			return;
		}
	}

	//this->Logger->Debug(CH_TEXTURES, "Queued %s", str.c_str());

	QueuedTexture raw{};
	raw.Identifier = str;
	raw.Data = aImageData;
	raw.Width = aWidth;
	raw.Height = aHeight;
	raw.Callback = aCallback;

	this->QueuedTextures.push_back(raw);
	this->PendingCallbacks.erase(str);
}

void CTextureLoader::CreateTexture(QueuedTexture aQueuedTexture)
{
	//this->Logger->Debug(CH_TEXTURES, "Create %s", aQueuedTexture.Identifier.c_str());

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
		this->Logger->Debug(CH_TEXTURES, "pTexture was null");
		stbi_image_free(aQueuedTexture.Data);

		/* nullptr response on fail */
		if (aQueuedTexture.Callback)
		{
			aQueuedTexture.Callback(aQueuedTexture.Identifier.c_str(), nullptr);
		}

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

	if (aQueuedTexture.Data)
	{
		stbi_image_free(aQueuedTexture.Data);
		aQueuedTexture.Data = {};
	}
}
