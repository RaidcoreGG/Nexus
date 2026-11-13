///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  PrefContext.h
/// Description  :  Provides functions to load and save settings.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

#include "Core/Logging/LogChannel.h"
#include "PrefSettingBase.h"

constexpr std::chrono::milliseconds FLUSH_DELAY = std::chrono::seconds(1);

///----------------------------------------------------------------------------------------------------
/// CPreferences Class
///----------------------------------------------------------------------------------------------------
class CPreferences : public ILogChannel
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// Get:
	/// 	Returns the process-wide settings store.
	///----------------------------------------------------------------------------------------------------
	static CPreferences* Get();

	///----------------------------------------------------------------------------------------------------
	/// Init:
	/// 	Sets the settings file, loads it and starts saving changes. Call exactly once, before any write.
	/// 	Settings registered before take the loaded values.
	///----------------------------------------------------------------------------------------------------
	void Init(std::filesystem::path aPath);

	///----------------------------------------------------------------------------------------------------
	/// Register:
	/// 	Registers a setting and gives it the stored value. Each key has exactly one owner.
	///----------------------------------------------------------------------------------------------------
	void Register(ISetting* aSetting);

	///----------------------------------------------------------------------------------------------------
	/// Deregister:
	/// 	Unregisters a setting.
	///----------------------------------------------------------------------------------------------------
	void Deregister(ISetting* aSetting);

	///----------------------------------------------------------------------------------------------------
	/// Write:
	/// 	Persists the value of a registered setting.
	///----------------------------------------------------------------------------------------------------
	void Write(const std::string& aKey, json aValue);

	~CPreferences();

	private:
	CPreferences();

	std::filesystem::path                      Path;

	std::mutex                                 Mutex;
	json                                       Store;
	std::unordered_map<std::string, ISetting*> Registry;
	bool                                       IsInitialized = false;
	bool                                       IsDirty       = false;
	bool                                       IsRunning     = false;

	std::condition_variable                    FlushCondition;
	std::thread                                FlushThread;

	///----------------------------------------------------------------------------------------------------
	/// LoadInternal:
	/// 	Loads the settings without locking the mutex. Returns false if the file must not be overwritten.
	///----------------------------------------------------------------------------------------------------
	bool LoadInternal();

	///----------------------------------------------------------------------------------------------------
	/// SaveInternal:
	/// 	Writes the given settings to the file.
	///----------------------------------------------------------------------------------------------------
	void SaveInternal(const json& aStore);

	///----------------------------------------------------------------------------------------------------
	/// FlushLoop:
	/// 	Sleeps until a change, then saves FLUSH_DELAY after it, together with the changes made meanwhile.
	///----------------------------------------------------------------------------------------------------
	void FlushLoop();
};
