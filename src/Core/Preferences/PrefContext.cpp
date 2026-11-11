///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  PrefContext.cpp
/// Description  :  Provides functions to load and save settings.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "PrefContext.h"

#include <cassert>
#include <filesystem>
#include <fstream>

constexpr const char* DROPPED_KEYS[] = { "LastGameBuild", "QAShowArcDPS" };

CPreferences* CPreferences::Get()
{
	static CPreferences s_Preferences = CPreferences();
	return &s_Preferences;
}

CPreferences::CPreferences()
{
	/* Constructed after the logger, so it is destroyed before it: the final save may log. */
	CLogApi::Get();
}

CPreferences::~CPreferences()
{
	{
		const std::lock_guard<std::mutex> lock(this->Mutex);
		this->IsRunning = false;
	}

	this->FlushCondition.notify_all();

	if (this->FlushThread.joinable())
	{
		this->FlushThread.join();
	}

	if (this->IsDirty)
	{
		this->SaveInternal(this->Store);
	}
}

void CPreferences::Init(std::filesystem::path aPath)
{
	std::vector<ISetting*> changed;

	{
		const std::lock_guard<std::mutex> lock(this->Mutex);

		if (this->IsInitialized)
		{
			LogCritical(CH_SETTINGS, "Preferences were already initialized.");
			assert(false && "CPreferences::Init may only be called once.");
			return;
		}

		this->IsInitialized = true;
		this->Path = aPath;

		const bool canSave = this->LoadInternal();

		for (const char* key : DROPPED_KEYS)
		{
			if (this->Store.erase(key) > 0)
			{
				this->IsDirty = true;
			}
		}

		for (auto& [key, setting] : this->Registry)
		{
			auto it = this->Store.find(key);

			if (it != this->Store.end() && setting->Apply(*it))
			{
				changed.push_back(setting);
			}
		}

		if (canSave)
		{
			this->IsRunning   = true;
			this->FlushThread = std::thread(&CPreferences::FlushLoop, this);
		}
	}

	for (ISetting* setting : changed)
	{
		setting->NotifyChanged();
	}
}

void CPreferences::Register(ISetting* aSetting)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	const std::string& key = aSetting->GetKey();

	if (this->Registry.find(key) != this->Registry.end())
	{
		LogCritical(CH_SETTINGS, "Setting \"%s\" is already owned by another instance.", key.c_str());
		assert(false && "A setting key may only have one owner.");
		return;
	}

	this->Registry.emplace(key, aSetting);

	auto it = this->Store.find(key);

	if (it != this->Store.end())
	{
		aSetting->Apply(*it);
	}
}

void CPreferences::Deregister(ISetting* aSetting)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Registry.find(aSetting->GetKey());

	if (it != this->Registry.end() && it->second == aSetting)
	{
		this->Registry.erase(it);
	}
}

void CPreferences::Write(const std::string& aKey, json aValue)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	/* Init replaces the store with the file, so an earlier write would be lost. */
	if (!this->IsInitialized)
	{
		LogCritical(CH_SETTINGS, "Setting \"%s\" was written before the preferences were initialized.", aKey.c_str());
		assert(false && "CPreferences::Write called before Init.");
		return;
	}

	this->Store[aKey] = std::move(aValue);

	/* Only the first change wakes the thread, later ones are saved with it. */
	if (this->IsRunning && !this->IsDirty)
	{
		this->IsDirty = true;
		this->FlushCondition.notify_one();
	}
}

void CPreferences::FlushLoop()
{
	std::unique_lock<std::mutex> lock(this->Mutex);

	while (this->IsRunning)
	{
		this->FlushCondition.wait(lock, [this] { return this->IsDirty || !this->IsRunning; });
		this->FlushCondition.wait_for(lock, FLUSH_DELAY, [this] { return !this->IsRunning; });

		/* The destructor saves what is pending. */
		if (!this->IsRunning)
		{
			continue;
		}

		json snapshot = this->Store;
		this->IsDirty = false;

		/* Written outside the lock so settings never wait on the disk. */
		lock.unlock();
		this->SaveInternal(snapshot);
		lock.lock();
	}
}

bool CPreferences::LoadInternal()
{
	if (this->Path.empty() || !std::filesystem::exists(this->Path))
	{
		return true;
	}

	try
	{
		std::ifstream file(this->Path);
		json          store = json::parse(file);

		if (store.is_object())
		{
			this->Store = std::move(store);
			return true;
		}

		LogWarning(CH_SETTINGS, "Settings.json does not contain an object.");
	}
	catch (const json::parse_error& ex)
	{
		LogWarning(CH_SETTINGS, "Settings.json could not be parsed. Error: %s", ex.what());
	}

	/* Kept aside, otherwise the first save replaces it with only the values changed since. */
	std::filesystem::path backup = this->Path;
	backup += ".bak";

	std::error_code ec;
	std::filesystem::remove(backup, ec);
	std::filesystem::rename(this->Path, backup, ec);

	if (ec)
	{
		LogCritical(CH_SETTINGS, "Settings.json could not be backed up. Changes will not be saved this session.");
		return false;
	}

	LogWarning(CH_SETTINGS, "Settings.json was moved to Settings.json.bak. Defaults are used.");
	return true;
}

void CPreferences::SaveInternal(const json& aStore)
{
	try
	{
		std::ofstream file(this->Path);
		file << aStore.dump(1, '\t') << std::endl;
		file.close();
	}
	catch (...)
	{
		LogWarning(CH_SETTINGS, "Settings.json could not be saved.");
	}
}
