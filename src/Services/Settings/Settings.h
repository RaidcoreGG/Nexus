///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Settings.h
/// Description  :  Provides functions to load and save settings.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef SETTINGS_H
#define SETTINGS_H

#include <filesystem>
#include <mutex>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "Services/Logging/LogHandler.h"

constexpr const char* CH_SETTINGS = "Settings";

///----------------------------------------------------------------------------------------------------
/// CSettings Class
///----------------------------------------------------------------------------------------------------
class CSettings
{
	public:
	CSettings(std::filesystem::path aPath, CLogHandler* aLogger);

	///----------------------------------------------------------------------------------------------------
	/// Load:
	/// 	Loads the settings.
	///----------------------------------------------------------------------------------------------------
	void Load();

	///----------------------------------------------------------------------------------------------------
	/// Save:
	/// 	Saves the settings.
	///----------------------------------------------------------------------------------------------------
	void Save();

	template <typename T>
	void Set(std::string aIdentifier, T aValue)
	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		this->Store[aIdentifier] = aValue;
		this->SaveInternal();
	}

	template <typename T>
	T Get(const std::string& aIdentifier)
	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		if (this->Store[aIdentifier].is_null())
		{
			return T{};
		}

		return this->Store[aIdentifier].get<T>();
	}

	private:
	CLogHandler*          Logger = nullptr;

	std::filesystem::path Path;

	std::mutex            Mutex;
	json                  Store;

	///----------------------------------------------------------------------------------------------------
	/// Load:
	/// 	Loads the settings without locking the mutex.
	///----------------------------------------------------------------------------------------------------
	void LoadInternal();

	///----------------------------------------------------------------------------------------------------
	/// Save:
	/// 	Saves the settings without locking the mutex.
	///----------------------------------------------------------------------------------------------------
	void SaveInternal();
};

#endif
