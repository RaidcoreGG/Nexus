///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Context.cpp
/// Description  :  Contains the main context.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Context.h"

#include <psapi.h>

#include "Branch.h"
#include "Core/Addons/Addon.h"
#include "Core/Index/Index.h"
#include "Util/CmdLine.h"
#include "Util/Strings.h"
#include "Util/Url.h"
#include "Version.h"

CContext* CContext::GetContext()
{
	static CContext s_Context;
	return &s_Context;
}

MajorMinorBuildRevision_t const& CContext::GetVersion()
{
	static MajorMinorBuildRevision_t version =
	{
		V_MAJOR,
		V_MINOR,
		V_BUILD,
		V_REVISION
	};
	return version;
}

const char* CContext::GetBuild()
{
#ifdef _DEBUG
	return "debug/" BRANCH_NAME;
#else
	return "release/" BRANCH_NAME;
#endif
}

void CContext::SetModule(HMODULE aModule)
{
	assert(!this->Module);
	this->Module = aModule;

	MODULEINFO moduleInfo{};
	GetModuleInformation(GetCurrentProcess(), this->Module, &moduleInfo, sizeof(moduleInfo));
	this->ModuleSize = moduleInfo.SizeOfImage;
}

HMODULE CContext::GetModule()
{
	return this->Module;
}

DWORD CContext::GetModuleSize()
{
	return this->ModuleSize;
}

RenderContext_t* CContext::GetRendererCtx()
{
	static RenderContext_t s_RendererCtx = RenderContext_t();
	return &s_RendererCtx;
}

CLogApi* CContext::GetLogger()
{
	static CLogApi s_Logger = CLogApi();
	return &s_Logger;
}

CTextureLoader* CContext::GetTextureService()
{
	static CTextureLoader s_TextureApi = CTextureLoader(
		this->GetLogger(),
		this->GetRendererCtx(),
		Index(EPath::DIR_TEXTURES)
	);
	return &s_TextureApi;
}

CDataLinkApi* CContext::GetDataLink()
{
	static CDataLinkApi s_DataLinkApi = CDataLinkApi(
		this->GetLogger()
	);
	return &s_DataLinkApi;
}

CEventApi* CContext::GetEventApi()
{
	static CEventApi s_EventApi = CEventApi(
		this->GetLoaderBase()
	);
	return &s_EventApi;
}

CLoaderBase* CContext::GetLoaderBase()
{
	static CLoaderBase s_Loader = CLoaderBase(
		this->GetLogger(),
		this->GetRendererCtx(),
		IAddonFactory,
		Index(EPath::DIR_ADDONS)
	);
	return &s_Loader;
}

CLibraryMgr* CContext::GetAddonLibrary()
{
	static CLibraryMgr s_LibraryMgr = CLibraryMgr(
		this->GetLogger(),
		this->GetLoaderBase()
	);
	return &s_LibraryMgr;
}

CRawInputApi* CContext::GetRawInputApi()
{
	static CRawInputApi s_RawInputApi = CRawInputApi(
		this->GetRendererCtx()
	);
	return &s_RawInputApi;
}

CInputBindApi* CContext::GetInputBindApi()
{
	static CInputBindApi s_InputBindApi = CInputBindApi(
		this->GetEventApi(),
		this->GetLogger(),
		Index(EPath::InputBinds)
	);
	return &s_InputBindApi;
}

CGameBindsApi* CContext::GetGameBindsApi()
{
	static CGameBindsApi s_GameBindsApi = CGameBindsApi(
		this->GetRawInputApi(),
		this->GetLogger(),
		this->GetEventApi(),
		Index(EPath::GameBinds)
	);
	return &s_GameBindsApi;
}

CUiContext* CContext::GetUIContext()
{
	static CUiContext s_UiContext = CUiContext(
		this->GetRendererCtx(),
		this->GetLogger(),
		this->GetTextureService(),
		this->GetDataLink(),
		this->GetInputBindApi(),
		this->GetEventApi(),
		this->GetMumbleReader()
	);
	return &s_UiContext;
}

CSettings* CContext::GetSettingsCtx()
{
	static CSettings s_SettingsApi = CSettings(
		Index(EPath::Settings),
		this->GetLogger()
	);
	return &s_SettingsApi;
}

CMumbleReader* CContext::GetMumbleReader()
{
	static CMumbleReader s_MumbleReader = CMumbleReader(
		this->GetDataLink(),
		this->GetEventApi(),
		this->GetLogger()
	);
	return &s_MumbleReader;
}

CHttpClient* CContext::GetHttpClient(std::string aURL)
{
	const std::lock_guard<std::mutex> lock(this->HttpClientMutex);

	std::string baseurl = URL::GetBase(aURL);

	std::string baseurl_noprotocol = baseurl;
	baseurl_noprotocol = String::Replace(baseurl_noprotocol, "htts://", "");
	baseurl_noprotocol = String::Replace(baseurl_noprotocol, "https://", "");

	auto it = this->HttpClients.find(baseurl_noprotocol);

	if (it != this->HttpClients.end())
	{
		return it->second;
	}

	std::filesystem::path cachedir = Index(EPath::DIR_COMMON) / baseurl_noprotocol;
	uint32_t cacheLifetime = 30 * 60; // 30 minutes

	if (baseurl == "https://api.raidcore.gg")
	{
		cacheLifetime = 5 * 60; // 5 minutes
	}
	else if (baseurl == "https://api.github.com")
	{
		cacheLifetime = 60 * 60; // 60 minutes
	}

	CHttpClient* client = new CHttpClient(this->GetLogger(), baseurl, cachedir, cacheLifetime);
	this->HttpClients.emplace(baseurl_noprotocol, client);

	return client;
}

CSelfUpdater* CContext::GetSelfUpdater()
{
	static CSelfUpdater s_SelfUpdater = CSelfUpdater(
		this->GetLogger()
	);
	return &s_SelfUpdater;
}

CArcApi* CContext::GetArcApi()
{
	static CArcApi s_ArcApi = CArcApi();
	return &s_ArcApi;
}

CConfigMgr* CContext::GetCfgMgr()
{
	static std::filesystem::path cfgpath      = Index(EPath::AddonConfigDefault);
	static std::vector<uint32_t> cfgwhitelist = {};

	static bool s_CmdLineParsed = [this] {
		if (CmdLine::HasArgument("-ggaddons"))
		{
			std::vector<std::string> idList = String::Split(CmdLine::GetArgumentValue("-ggaddons"), ",");

			/* If only one entry and it contains ".json" it's a custom config. */
			if (idList.size() == 1 && String::Contains(idList[0], ".json"))
			{
				cfgpath = idList[0];
			}
			else
			{
				for (std::string addonsig : idList)
				{
					try
					{
						uint32_t sig = std::stoi(addonsig, nullptr, 0);
						cfgwhitelist.push_back(sig);
					}
					catch (const std::invalid_argument& e)
					{
						this->GetLogger()->Trace(CH_LOADERBASE, "Invalid argument(-ggaddons) : %s (exc: %s)", addonsig.c_str(), e.what());
					}
					catch (const std::out_of_range& e)
					{
						this->GetLogger()->Trace(CH_LOADERBASE, "Out of range (-ggaddons): %s (exc: %s)", addonsig.c_str(), e.what());
					}
				}
			}
		}
		return true;
	}();

	static CConfigMgr s_CfgMgr = CConfigMgr(
		this->GetLogger(),
		cfgpath,
		cfgwhitelist
	);
	return &s_CfgMgr;
}

CContext::~CContext()
{
	const std::lock_guard<std::mutex> lock(this->HttpClientMutex);

	for (auto it = this->HttpClients.begin(); it != this->HttpClients.end();)
	{
		/* Deallocate client. */
		delete it->second;

		/* Erase entry. */
		it = this->HttpClients.erase(it);
	}
}
