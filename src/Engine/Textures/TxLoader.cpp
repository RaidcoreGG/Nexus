///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  TextureLoader.cpp
/// Description  :  Provides functions to load textures and fetch created textures.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Engine/Textures/TxLoader.h"

#include <d3d11.h>
#include <filesystem>

#pragma warning(push, 0)
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "httplib/httplib.h"
#pragma warning(pop)

#include "Util/Time.h"
#include "Util/Url.h"

CTextureLoader::CTextureLoader(CLogApi* aLogger, RenderContext_t* aRenderCtx, std::filesystem::path aOverridesDirectory) : IRefCleaner("TextureLoader")
{
	assert(aLogger);
	assert(aRenderCtx);

	this->Logger        = aLogger;
	this->RenderContext = aRenderCtx;

	this->OverridesDirectory = aOverridesDirectory;

	/* 8 Worker threads. This is disgusting. May I interest you in a threadpool? */
	for (size_t i = 0; i < 8; i++)
	{
		this->DownloadThreads.push_back(std::thread(&CTextureLoader::ProcessDownloads, this));
	}
}

CTextureLoader::~CTextureLoader()
{
	this->IsRunning = false;
	this->ConVar.notify_all();

	for (size_t i = 0; i < 8; i++)
	{
		if (this->DownloadThreads[i].joinable())
		{
			this->DownloadThreads[i].join();
		}
	}

	const std::lock_guard<std::mutex> lock(this->Mutex);

	for (auto it = this->Registry.begin(); it != this->Registry.end();)
	{
		/* Release texture. */
		it->second->Resource->Release();

		/* Deallocate wrapper. */
		delete it->second;

		/* Erase entry. */
		it = this->Registry.erase(it);
	}
}

void CTextureLoader::Advance()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	long long now = Time::GetTimestampMs();

	for (auto it = this->QueuedTextures.begin(); it != this->QueuedTextures.end();)
	{
		switch (it->second.Stage)
		{
			default:
			case ETextureStage::None:
			case ETextureStage::Prepare:
			{
				if (now - it->second.Time > 60000)
				{
					this->Logger->Debug(CH_TEXTURES, "Dropped texture with ID \"%s\" from queue after %dms at stage %d.", it->first.c_str(), now - it->second.Time, it->second.Stage);
					it->second.Stage = ETextureStage::INVALID;
					++it;
				}
				else
				{
					++it;
				}
				break;
			}
			case ETextureStage::Ready:
			{
				this->CreateTexture(it->first, it->second);
				++it;
				break;
			}
			case ETextureStage::Done:
			case ETextureStage::INVALID:
			{
				if (it->second.Data)
				{
					stbi_image_free(it->second.Data);
					it->second.Data = nullptr;
				}

				/* Only dispatch invalid textures. Created (Done) already were dispatched. */
				if (it->second.Stage == ETextureStage::INVALID && it->second.Callback)
				{
					this->DispatchTexture(it->first, nullptr, it->second.Callback);
				}

				it = this->QueuedTextures.erase(it);
				break;
			}
		}
	}
}

Texture_t* CTextureLoader::Get(const char* aIdentifier)
{
	if (!aIdentifier) { return nullptr; }

	Texture_t* result = nullptr;

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(aIdentifier);

	if (it != this->Registry.end())
	{
		result = it->second;
	}

	return result;
}

Texture_t* CTextureLoader::GetOrCreate(const char* aIdentifier, const char* aFilename)
{
	Texture_t* result = Get(aIdentifier);

	if (!result)
	{
		this->Load(aIdentifier, aFilename, nullptr);
	}

	return result;
}

Texture_t* CTextureLoader::GetOrCreate(const char* aIdentifier, unsigned aResourceID, HMODULE aModule)
{
	Texture_t* result = this->Get(aIdentifier);

	if (!result)
	{
		this->Load(aIdentifier, aResourceID, aModule, nullptr);
	}

	return result;
}

Texture_t* CTextureLoader::GetOrCreate(const char* aIdentifier, const char* aRemote, const char* aEndpoint)
{
	Texture_t* result = this->Get(aIdentifier);

	if (!result)
	{
		this->Load(aIdentifier, aRemote, aEndpoint, nullptr);
	}

	return result;
}

Texture_t* CTextureLoader::GetOrCreate(const char* aIdentifier, void* aData, size_t aSize)
{
	Texture_t* result = this->Get(aIdentifier);

	if (!result)
	{
		this->Load(aIdentifier, aData, aSize, nullptr);
	}

	return result;
}

void CTextureLoader::Load(const char* aIdentifier, const char* aFilename, TEXTURES_RECEIVECALLBACK aCallback, bool aIsShadowing)
{
	/* Preprocess the request to determine, if we should load. */
	if (this->ProcessRequest(aIdentifier, aCallback, aIsShadowing))
	{
		return;
	}

	/* Queue the callback. */
	this->Enqueue(aIdentifier, aCallback);

	if (!std::filesystem::exists(aFilename))
	{
		this->Logger->Warning(CH_TEXTURES, "File provided does not exist: %s (%s)", aFilename, aIdentifier);

		/* nullptr response on fail */
		this->DispatchTexture(aIdentifier, nullptr, aCallback);
		this->Dequeue(aIdentifier);

		return;
	}

	// Load from disk into a raw RGBA buffer
	int width = 0;
	int height = 0;
	unsigned char* data = stbi_load(aFilename, &width, &height, NULL, 4);

	this->Enqueue(aIdentifier, data, width, height);
}

void CTextureLoader::Load(const char* aIdentifier, unsigned aResourceID, HMODULE aModule, TEXTURES_RECEIVECALLBACK aCallback, bool aIsShadowing)
{
	/* Preprocess the request to determine, if we should load. */
	if (this->ProcessRequest(aIdentifier, aCallback, aIsShadowing))
	{
		return;
	}

	/* Queue the callback. */
	this->Enqueue(aIdentifier, aCallback);

	HRSRC imageResHandle = FindResourceA(aModule, MAKEINTRESOURCEA(aResourceID), "PNG");
	if (!imageResHandle)
	{
		this->Logger->Debug(CH_TEXTURES, "Resource not found ResID: %u (%s)", aResourceID, aIdentifier);

		/* nullptr response on fail */
		this->DispatchTexture(aIdentifier, nullptr, aCallback);
		this->Dequeue(aIdentifier);

		return;
	}

	HGLOBAL imageResDataHandle = LoadResource(aModule, imageResHandle);
	if (!imageResDataHandle)
	{
		this->Logger->Debug(CH_TEXTURES, "Failed loading resource: %u (%s)", aResourceID, aIdentifier);

		/* nullptr response on fail */
		this->DispatchTexture(aIdentifier, nullptr, aCallback);
		this->Dequeue(aIdentifier);

		return;
	}

	LPVOID imageFile = LockResource(imageResDataHandle);
	if (!imageFile)
	{
		this->Logger->Debug(CH_TEXTURES, "Failed locking resource: %u (%s)", aResourceID, aIdentifier);

		/* nullptr response on fail */
		this->DispatchTexture(aIdentifier, nullptr, aCallback);
		this->Dequeue(aIdentifier);

		return;
	}

	DWORD imageFileSize = SizeofResource(aModule, imageResHandle);
	if (!imageFileSize)
	{
		this->Logger->Debug(CH_TEXTURES, "Failed getting size of resource: %u (%s)", aResourceID, aIdentifier);

		/* nullptr response on fail */
		this->DispatchTexture(aIdentifier, nullptr, aCallback);
		this->Dequeue(aIdentifier);

		return;
	}

	/* Load the texture. */
	int width = 0;
	int height = 0;
	int copmonents = 0;
	unsigned char* data = stbi_load_from_memory((const stbi_uc*)imageFile, imageFileSize, &width, &height, &copmonents, 4);

	/* Queue the data. */
	this->Enqueue(aIdentifier, data, width, height);
}

void CTextureLoader::Load(const char* aIdentifier, const char* aRemote, const char* aEndpoint, TEXTURES_RECEIVECALLBACK aCallback, bool aIsShadowing)
{
	/* Preprocess the request to determine, if we should load. */
	if (this->ProcessRequest(aIdentifier, aCallback, aIsShadowing))
	{
		return;
	}

	/* Queue the callback and URL. */
	this->Enqueue(aIdentifier, std::string(aRemote) + std::string(aEndpoint), aCallback);
}

void CTextureLoader::Load(const char* aIdentifier, void* aData, size_t aSize, TEXTURES_RECEIVECALLBACK aCallback, bool aIsShadowing)
{
	/* Preprocess the request to determine, if we should load. */
	if (this->ProcessRequest(aIdentifier, aCallback, aIsShadowing))
	{
		return;
	}

	/* Queue the callback. */
	this->Enqueue(aIdentifier, aCallback);

	/* Load the texture. */
	int width = 0;
	int height = 0;
	int components = 0;
	unsigned char* data = stbi_load_from_memory((const stbi_uc*)aData, static_cast<int>(aSize), &width, &height, &components, 4);

	/* Queue the data. */
	this->Enqueue(aIdentifier, data, width, height);
}

std::map<std::string, Texture_t*> CTextureLoader::GetRegistry() const
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	return this->Registry;
}

std::map<std::string, QueuedTexture_t> CTextureLoader::GetQueuedTextures() const
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	return this->QueuedTextures;
}

uint32_t CTextureLoader::CleanupRefs(void* aStartAddress, void* aEndAddress)
{
	uint32_t refCounter = 0;

	const std::lock_guard<std::mutex> lock(this->Mutex);

	for (auto& [identifier, qTex] : this->QueuedTextures)
	{
		if (qTex.Callback >= aStartAddress && qTex.Callback <= aEndAddress)
		{
			/* Same as Dequeue() */
			qTex.Stage = ETextureStage::INVALID;
			qTex.Callback = nullptr;

			refCounter++;
		}
	}

	return refCounter;
}

bool CTextureLoader::ProcessRequest(const char* aIdentifier, TEXTURES_RECEIVECALLBACK aCallback, bool aIsShadowing)
{
	/* If this is already queued, stop processing. */
	if (this->IsQueued(aIdentifier))
	{
		return true;
	}

	Texture_t* result = this->Get(aIdentifier);

	/* If shadowing any existing texture. */
	if (aIsShadowing)
	{
		/* Reset the result. */
		result = nullptr;

		/* Rename existing texture. */
		this->ShadowTexture(aIdentifier);
	}

	/* We already have a result. Dispatch it and stop processing. */
	if (result)
	{
		this->DispatchTexture(aIdentifier, result, aCallback);
		return true;
	}

	/* Stop processing, if overriding. */
	if (this->OverrideTexture(aIdentifier, aCallback))
	{
		return true;
	}

	return false;
}

void CTextureLoader::ShadowTexture(const char* aIdentifier)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	std::string id = aIdentifier;
	int i = 0;

	auto targetIt = this->Registry.find(id);

	/* Target identifier does not even exist. */
	if (targetIt == this->Registry.end())
	{
		return;
	}

	/* Iterate until free identifier is found. */
	while (auto freeIt = this->Registry.find(id) != this->Registry.end())
	{
		i++;
		id = aIdentifier;
		id.append("_");
		id.append(std::to_string(i));
	}

	/* Move target iterate to free identifier. */
	this->Registry.emplace(id, targetIt->second);
	this->Registry.erase(targetIt);
}

bool CTextureLoader::OverrideTexture(const char* aIdentifier, TEXTURES_RECEIVECALLBACK aCallback)
{
	if (this->OverridesDirectory.empty()) { return false; }

	std::filesystem::path overridepath = this->OverridesDirectory / (aIdentifier + std::string{ ".png" });

	if (std::filesystem::exists(overridepath))
	{
		int width = 0;
		int height = 0;
		unsigned char* data = stbi_load(overridepath.string().c_str(), &width, &height, NULL, 4);

		this->Enqueue(aIdentifier, data, width, height);

		/* Signal to stop processing. */
		return true;
	}

	/* Signal to continue processing. */
	return false;
}

bool CTextureLoader::IsQueued(const char* aIdentifier)
{
	if (!aIdentifier) { return false; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->QueuedTextures.find(aIdentifier);

	if (it != this->QueuedTextures.end())
	{
		return true;
	}

	return false;
}

void CTextureLoader::Enqueue(const char* aIdentifier, TEXTURES_RECEIVECALLBACK aCallback)
{
	if (!aIdentifier) { return; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->QueuedTextures.find(aIdentifier);

	if (it == this->QueuedTextures.end())
	{
		QueuedTexture_t entry{};
		entry.Stage    = ETextureStage::Prepare;
		entry.Data     = nullptr;
		entry.Width    = 0;
		entry.Height   = 0;
		entry.Callback = aCallback;
		entry.Time = Time::GetTimestampMs();

		this->QueuedTextures.emplace(aIdentifier, entry);
	}
}

void CTextureLoader::Enqueue(const char* aIdentifier, std::string aDownloadURL, TEXTURES_RECEIVECALLBACK aCallback)
{
	if (!aIdentifier) { return; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->QueuedTextures.find(aIdentifier);

	if (it != this->QueuedTextures.end())
	{
		it->second.Stage = ETextureStage::Prepare;
		it->second.DownloadURL = aDownloadURL;
		it->second.Callback = aCallback;
	}
	else
	{
		QueuedTexture_t entry{};
		entry.Stage = ETextureStage::Prepare;
		entry.DownloadURL = aDownloadURL;
		entry.Callback = aCallback;
		entry.Time = Time::GetTimestampMs();

		this->QueuedTextures.emplace(aIdentifier, entry);
	}
}

void CTextureLoader::Enqueue(const char* aIdentifier, unsigned char* aData, int aWidth, int aHeight)
{
	if (!aIdentifier) { return; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->QueuedTextures.find(aIdentifier);

	if (it != this->QueuedTextures.end())
	{
		it->second.Stage = ETextureStage::Ready;
		it->second.Data   = aData;
		it->second.Width  = aWidth;
		it->second.Height = aHeight;
	}
	else
	{
		QueuedTexture_t entry{};
		entry.Stage = ETextureStage::Ready;
		entry.Data = aData;
		entry.Width = aWidth;
		entry.Height = aHeight;
		entry.Time = Time::GetTimestampMs();

		this->QueuedTextures.emplace(aIdentifier, entry);
	}
}

void CTextureLoader::Dequeue(const char* aIdentifier)
{
	if (!aIdentifier) { return; }

	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->QueuedTextures.find(aIdentifier);

	if (it != this->QueuedTextures.end())
	{
		it->second.Stage = ETextureStage::INVALID;
	}
}

void CTextureLoader::CreateTexture(const std::string& aIdentifier, QueuedTexture_t& aQueuedTexture)
{
	/* Create texture description. */
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width            = aQueuedTexture.Width;
	desc.Height           = aQueuedTexture.Height;
	desc.MipLevels        = 1;
	desc.ArraySize        = 1;
	desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage            = D3D11_USAGE_DEFAULT;
	desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags   = 0;

	/* Create Texture. */
	D3D11_SUBRESOURCE_DATA subResource{};
	subResource.pSysMem          = aQueuedTexture.Data;
	subResource.SysMemPitch      = desc.Width * 4;
	subResource.SysMemSlicePitch = 0;

	ID3D11Texture2D* pTexture = nullptr;
	this->RenderContext->Device->CreateTexture2D(&desc, &subResource, &pTexture);

	if (!pTexture)
	{
		this->Logger->Debug(CH_TEXTURES, "pTexture was null");
		stbi_image_free(aQueuedTexture.Data);

		/* Manual dequeue, because of Mutex lock. */
		aQueuedTexture.Stage = ETextureStage::INVALID;

		return;
	}

	/* Create SRV. */
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels       = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;

	ID3D11ShaderResourceView* srv = nullptr;
	this->RenderContext->Device->CreateShaderResourceView(pTexture, &srvDesc, &srv);

	pTexture->Release();

	Texture_t* result = new Texture_t{
		aQueuedTexture.Width,
		aQueuedTexture.Height,
		srv
	};

	this->Registry.emplace(aIdentifier, result);

	this->DispatchTexture(aIdentifier, result, aQueuedTexture.Callback);

	if (aQueuedTexture.Data)
	{
		stbi_image_free(aQueuedTexture.Data);
		aQueuedTexture.Data = nullptr;
	}

	aQueuedTexture.Stage = ETextureStage::Done;
}

void CTextureLoader::DispatchTexture(const std::string& aIdentifier, Texture_t* aTexture, TEXTURES_RECEIVECALLBACK aCallback)
{
	if (aIdentifier.empty()) { return; }
	if (!aCallback)          { return; }

	try
	{
		aCallback(aIdentifier.c_str(), aTexture);
	}
	catch (...)
	{
		this->Logger->Debug(CH_TEXTURES, "DispatchTexture() failed with: %s %p %p", aIdentifier.c_str(), aTexture, aCallback);
	}
}

void CTextureLoader::ProcessDownloads()
{
	while (this->IsRunning)
	{
		{
			std::unique_lock<std::mutex> lock(this->Mutex);
			this->ConVar.wait_for(lock, std::chrono::milliseconds(5000), [this] {
				return this->QueuedTextures.size() > 0 || !this->IsRunning;
			});
		}

		/* Early exit without processing the remaining textures. */
		if (!this->IsRunning)
		{
			break;
		}

		std::map<std::string, QueuedTexture_t> queueCpy = GetQueuedTextures();

		for (auto& [id, qtex] : queueCpy)
		{
			std::string downloadUrl = "";

			/* Scope and lock, to verify we're processing this texture on this thread. */
			{
				const std::lock_guard<std::mutex> lock(this->Mutex);
				auto it = this->QueuedTextures.find(id);

				if (it == this->QueuedTextures.end())
				{
					/* Already gone again :( */
					continue;
				}
				else if (it->second.DownloadURL.empty())
				{
					continue;
				}
				else if (it->second.Stage == ETextureStage::Prepare && !it->second.DownloadURL.empty())
				{
					std::swap(downloadUrl, it->second.DownloadURL);
				}
				else
				{
					continue;
				}
			}

			/* If we swapped a download URL, download it. */
			if (!downloadUrl.empty())
			{
				std::string remote = URL::GetBase(qtex.DownloadURL);
				std::string endpoint = URL::GetEndpoint(qtex.DownloadURL);

				httplib::Client client(remote);
				client.enable_server_certificate_verification(true);
				client.set_follow_location(true);
				client.set_url_encode(false);

				auto result = client.Get(endpoint);

				if (!result)
				{
					this->Logger->Debug(CH_TEXTURES, "Error fetching %s%s (%s)\nError: %s", remote.c_str(), endpoint.c_str(), id.c_str(), httplib::to_string(result.error()).c_str());

					/* nullptr response on fail */
					this->Dequeue(id.c_str());

					return;
				}

				// Status is not HTTP_OK
				if (result->status != 200)
				{
					this->Logger->Debug(CH_TEXTURES, "Status %d when fetching %s%s (%s) | %s", result->status, remote.c_str(), endpoint.c_str(), id.c_str(), httplib::to_string(result.error()).c_str());

					/* nullptr response on fail */
					this->Dequeue(id.c_str());

					return;
				}

				size_t size = result->body.size();
				unsigned char* remote_data = new unsigned char[size];
				std::memcpy(remote_data, result->body.c_str(), size);

				int width = 0;
				int height = 0;
				int components;

				stbi_uc* data = stbi_load_from_memory(remote_data, static_cast<int>(size), &width, &height, &components, 4);

				delete[] remote_data;

				/* Enqueue the data. */
				this->Enqueue(id.c_str(), data, width, height);
			}
		}
	}
}
