///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  PrefContext.h
/// Description  :  Provides functions to load and save settings.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <filesystem>
#include <mutex>

#pragma warning(push, 0)
#include "nlohmann/json.hpp"
#pragma warning(pop)
using json = nlohmann::json;

#include "Engine/Logging/LogApi.h"

constexpr const char* CH_SETTINGS = "Settings";

///----------------------------------------------------------------------------------------------------
/// CSettings Class
///----------------------------------------------------------------------------------------------------
class CSettings
{
	public:
	CSettings(std::filesystem::path aPath, CLogApi* aLogger);

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

	///----------------------------------------------------------------------------------------------------
	/// Set:
	/// 	Stores a setting.
	///----------------------------------------------------------------------------------------------------
	template <typename T>
	void Set(const std::string& aIdentifier, T aValue)
	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		this->Store[aIdentifier] = aValue;
		this->SaveInternal();

		this->NotifyChanged(aIdentifier, this->Store[aIdentifier]);
	}

	///----------------------------------------------------------------------------------------------------
	/// Get:
	/// 	Retrieves a setting.
	///----------------------------------------------------------------------------------------------------
	template <typename T>
	T Get(const std::string& aIdentifier, T aDefaultValue = {})
	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		if (this->Store[aIdentifier].is_null())
		{
			this->Store[aIdentifier] = aDefaultValue;
			this->SaveInternal();

			return aDefaultValue;
		}

		return this->Store[aIdentifier].get<T>();
	}

	///----------------------------------------------------------------------------------------------------
	/// Subscribe:
	/// 	Subscribes to updates when a setting is changed.
	/// 	Immediately notifies the subscribed callback with the value.
	///----------------------------------------------------------------------------------------------------
	template <typename T>
	void Subscribe(const std::string& aIdentifier, std::function<void(T)> aCallback)
	{
		OnChangeCallback wrapperCb = [callback = std::move(aCallback)](const json& aValue)
		{
			try
			{
				callback(aValue.get<T>());
			}
			catch (...)
			{
				// TODO: Error handling
			}
		};

		{
			const std::lock_guard<std::mutex> lock(this->NotifierMutex);

			std::vector<OnChangeCallback>& notifiers = this->Notifiers[aIdentifier];
			notifiers.push_back(wrapperCb);
		}

		/* Notify the new subscriber. */
		{
			const std::lock_guard<std::mutex> lock(this->Mutex);

			/* Only notify, if not null. */
			if (!this->Store[aIdentifier].is_null())
			{
				wrapperCb(this->Store[aIdentifier]);
			}
		}
	}

	///----------------------------------------------------------------------------------------------------
	/// Remove:
	/// 	Removes a setting entirely.
	///----------------------------------------------------------------------------------------------------
	void Remove(const std::string& aIdentifier);

	private:
	CLogApi*                                                       Logger = nullptr;

	std::filesystem::path                                          Path;

	/* Settings */
	std::mutex                                                     Mutex;
	json                                                           Store;

	/* Notifiers */
	using OnChangeCallback = std::function<void(const json&)>;
	std::mutex                                                     NotifierMutex;
	std::unordered_map<std::string, std::vector<OnChangeCallback>> Notifiers;

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

	///----------------------------------------------------------------------------------------------------
	/// NotifyChanged:
	/// 	Calls the notifiers of a given settings identifier with the new value.
	///----------------------------------------------------------------------------------------------------
	void NotifyChanged(const std::string& aIdentifier, const json& aNewValue);
};
