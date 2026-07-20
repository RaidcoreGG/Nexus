///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  PrefContext.cpp
/// Description  :  Provides functions to load and save settings.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "PrefContext.h"

#include <filesystem>
#include <fstream>

namespace Raidcore::Nexus::Core
{
	SettingsMgr::SettingsMgr(std::filesystem::path aPath, LogApi* aLogger)
	{
		assert(aLogger);

		this->Logger = aLogger;

		this->Path = aPath;
		this->Load();
	}

	void SettingsMgr::Load()
	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		this->LoadInternal();
	}

	void SettingsMgr::Save()
	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		this->SaveInternal();
	}

	void SettingsMgr::Remove(const std::string& aIdentifier)
	{
		{
			const std::lock_guard<std::mutex> lock(this->Mutex);

			this->Store.erase(aIdentifier);
			this->SaveInternal();
		}

		{
			const std::lock_guard<std::mutex> lock(this->NotifierMutex);
			this->Notifiers[aIdentifier].clear();
		}
	}

	void SettingsMgr::LoadInternal()
	{
		if (!std::filesystem::exists(this->Path))
		{
			return;
		}

		try
		{
			std::ifstream file(this->Path);
			this->Store = json::parse(file);
			file.close();
		}
		catch (json::parse_error& ex)
		{
			this->Logger->Warning(CH_SETTINGS, "Settings.json could not be parsed. Error: %s", ex.what());
		}
	}

	void SettingsMgr::SaveInternal()
	{
		try
		{
			std::ofstream file(this->Path);
			file << this->Store.dump(1, '\t') << std::endl;
			file.close();
		}
		catch (...)
		{
			this->Logger->Warning(CH_SETTINGS, "Settings.json could not be saved.");
		}
	}

	void SettingsMgr::NotifyChanged(const std::string& aIdentifier, const json& aNewValue)
	{
		/* Don't notify if null. */
		if (aNewValue.is_null())
		{
			return;
		}

		std::vector<OnChangeCallback> callbacksCopy;

		{
			const std::lock_guard<std::mutex> lock(this->NotifierMutex);

			auto it = this->Notifiers.find(aIdentifier);

			if (it != this->Notifiers.end())
			{
				callbacksCopy = it->second;
			}
		}

		for (auto& wrappedCallback : callbacksCopy)
		{
			try
			{
				wrappedCallback(aNewValue);
			}
			catch (...)
			{
				// TODO: Error handling
			}
		}
	}
}
