///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  CfgManager.cpp
/// Description  :  Manager for addon configurations.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "CfgManager.h"

#include <fstream>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

/* Config Migration Keys */
constexpr const char* KM_PAUSEUPDATES  = "IsPausingUpdates";
constexpr const char* KM_ISLOADED      = "IsLoaded";

/* Config Active Keys */
constexpr const char* K_SIGNATURE      = "Signature";
constexpr const char* K_ISFAVORITE     = "IsFavorite";
constexpr const char* K_UPDATEMODE     = "UpdateMode";
constexpr const char* K_PRERELEASES    = "AllowPrereleases";
constexpr const char* K_SHOULDLOAD     = "ShouldLoad";
constexpr const char* K_DISABLEVERSION = "DisableVersion";
constexpr const char* K_LASTGAMEBUILD  = "LastGameBuild";
constexpr const char* K_NAME           = "Name";

CConfigMgr::CConfigMgr(CLogApi* aLogger, std::filesystem::path aConfigPath, std::vector<uint32_t> aWhitelist)
{
	if (aConfigPath.empty())
	{
		throw "Config path can't be empty.";
	}

	this->Logger = aLogger;

	this->ReadOnly = aWhitelist.size() > 0;
	this->Path = aConfigPath;
	this->Whitelist = aWhitelist;

	this->LoadConfigs();
}

CConfigMgr::~CConfigMgr()
{
	this->Logger = nullptr;
}

void CConfigMgr::SaveConfigs()
{
	if (this->ReadOnly) { return; }

	json cfgPack = json::array();

	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		for (auto& [sig, cfg] : this->Configs)
		{
			/* Do not save, if it was uninstalled. */
			if (!cfg->Persist) { continue; }

			/* Map values. Add signature to identify. */
			json cfgJSON = json{
				{ K_SIGNATURE,      sig                   },
				{ K_ISFAVORITE,     cfg->IsFavorite       },
				{ K_UPDATEMODE,     cfg->UpdateMode       },
				{ K_PRERELEASES,    cfg->AllowPreReleases },
				{ K_SHOULDLOAD,     cfg->ShouldLoad       },
				{ K_DISABLEVERSION, cfg->DisableVersion   },
				{ K_LASTGAMEBUILD,  cfg->LastGameBuild    },
				{ K_NAME,           cfg->LastName         }
			};

			cfgPack.push_back(cfgJSON);
		}
	}

	std::ofstream file(this->Path);

	if (!file.is_open())
	{
		this->Logger->Warning(
			CH_CFGMGR,
			"Failed to open \"%s\" for writing.",
			this->Path.string().c_str()
		);
		return;
	}

	file << cfgPack.dump(1, '\t') << std::endl;
	file.close();
}

Config_t* CConfigMgr::RegisterConfig(uint32_t aSignature)
{
	Config_t* config = nullptr;

	/* Scoping for mutex. */
	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		auto it = this->Configs.find(aSignature);

		if (it != this->Configs.end())
		{
			return it->second;
		}

		config = new Config_t();
		this->Configs.emplace(aSignature, config);
	}

	this->SaveConfigs();

	return config;
}

void CConfigMgr::DeleteConfig(uint32_t aSignature)
{
	/* Scoping for mutex. */
	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		auto it = this->Configs.find(aSignature);

		if (it == this->Configs.end())
		{
			return;
		}

		delete it->second;
		this->Configs.erase(it);
	}

	this->SaveConfigs();
}

void CConfigMgr::LoadConfigs()
{
	if (!std::filesystem::exists(this->Path)) { return; }
	if (this->Configs.size() > 0)             { return; }

	try
	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		std::ifstream file(this->Path);

		if (!file.is_open())
		{
			this->Logger->Warning(
				CH_CFGMGR,
				"Failed to open \"%s\" for reading.",
				this->Path.string().c_str()
			);
			return;
		}

		json cfgPack = json::parse(file);

		if (cfgPack.is_null())
		{
			this->Logger->Warning(
				CH_CFGMGR,
				"Failed to parse \"%s\". Config was null.",
				this->Path.string().c_str()
			);
			return;
		}

		for (json& cfgJSON : cfgPack)
		{
			if (cfgJSON.is_null())              { continue; }
			if (cfgJSON[K_SIGNATURE].is_null()) { continue; }

			uint32_t sig = cfgJSON.value(K_SIGNATURE, 0);

			if (sig == 0) { continue; }

			Config_t* config = new Config_t();

			/* Migration. */
			if (!cfgJSON[KM_PAUSEUPDATES].is_null())
			{
				bool pauseUpdates = cfgJSON[KM_PAUSEUPDATES].get<bool>();
				config->UpdateMode = pauseUpdates ? EUpdateMode::Background : EUpdateMode::Automatic;
			}

			/* Load new key manually, to override migration if both exist. */
			if (!cfgJSON[K_UPDATEMODE].is_null())
			{
				config->UpdateMode = cfgJSON[K_UPDATEMODE].get<EUpdateMode>();
			}

			if (config->UpdateMode == EUpdateMode::None)
			{
				config->UpdateMode = EUpdateMode::Background;
			}

			/* Migration. */
			if (!cfgJSON[KM_ISLOADED].is_null())
			{
				config->ShouldLoad = cfgJSON[KM_ISLOADED].get<bool>();
			}

			/* Load new key manually, to override migration if both exist. */
			if (!cfgJSON[K_SHOULDLOAD].is_null())
			{
				config->ShouldLoad = cfgJSON[K_SHOULDLOAD].get<bool>();
			}

			/* If -ggaddons sig list, compare against that. */
			if (this->ReadOnly)
			{
				config->ShouldLoad = false;

				auto whitelisted = std::find(this->Whitelist.begin(), this->Whitelist.end(), sig);

				if (whitelisted != this->Whitelist.end())
				{
					config->ShouldLoad = true;
				}
			}

			/* Calling .value() is less writing, but redundant as defaults are set on the struct definition. */
			config->IsFavorite       = cfgJSON.value(K_ISFAVORITE,     false);
			config->AllowPreReleases = cfgJSON.value(K_PRERELEASES,    false);
			config->DisableVersion   = cfgJSON.value(K_DISABLEVERSION, "");
			config->LastGameBuild    = cfgJSON.value(K_LASTGAMEBUILD,  0);
			config->LastName         = cfgJSON.value(K_NAME,           "");

			this->Configs.emplace(sig, config);
		}

		file.close();
	}
	catch (json::parse_error& ex)
	{
		this->Logger->Warning(
			CH_CFGMGR,
			"Failed to parse \"%s\". Error: %s",
			this->Path.filename().string().c_str(),
			ex.what()
		);
	}
}
