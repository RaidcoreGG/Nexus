///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  CfgManager.h
/// Description  :  Manager for addon configurations.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef CFGMANAGER_H
#define CFGMANAGER_H

#include <cstdint>
#include <filesystem>
#include <mutex>
#include <unordered_map>

#include "Config.h"
#include "Engine/Logging/LogApi.h"

constexpr const char* CH_CFGMGR = "Config";

///----------------------------------------------------------------------------------------------------
/// CConfigMgr Class
///----------------------------------------------------------------------------------------------------
class CConfigMgr
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CConfigMgr(CLogApi* aLogger, std::filesystem::path aConfigPath, bool aReadOnly = false);

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	~CConfigMgr();

	///----------------------------------------------------------------------------------------------------
	/// SaveConfigs:
	/// 	Saves the addon configs to disk.
	///----------------------------------------------------------------------------------------------------
	void SaveConfigs();

	///----------------------------------------------------------------------------------------------------
	/// RegisterConfig:
	/// 	Returns the config of the given addon signature or registers them, if they don't exist.
	///----------------------------------------------------------------------------------------------------
	Config_t* RegisterConfig(uint32_t aSignature);

	///----------------------------------------------------------------------------------------------------
	/// DeleteConfig:
	/// 	Deletes the config of the given addon signature, also deletes it from disk.
	///----------------------------------------------------------------------------------------------------
	void DeleteConfig(uint32_t aSignature);

	private:
	CLogApi*                                Logger = nullptr;

	bool                                    ReadOnly = false;
	std::filesystem::path                   Path;

	std::mutex                              Mutex;
	std::unordered_map<uint32_t, Config_t*> Configs;

	///----------------------------------------------------------------------------------------------------
	/// LoadConfigs:
	/// 	Loads the addon configs from disk.
	///----------------------------------------------------------------------------------------------------
	void LoadConfigs();
};

#endif
