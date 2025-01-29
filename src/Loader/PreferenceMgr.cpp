///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  PreferenceMgr.cpp
/// Description  :  Loader component for managing addon preferences.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "PreferenceMgr.h"

#include <fstream>

#include "LoaderConst.h"

constexpr const char* K_SIGNATURE = "Signature";

CPreferenceMgr::CPreferenceMgr(CLogHandler* aLogger, std::filesystem::path aConfigPath, bool aReadOnly)
{
	this->Logger = aLogger;

	this->ReadOnly = aReadOnly;
	this->Path = aConfigPath;

	this->LoadPrefsSafe();
}

CPreferenceMgr::~CPreferenceMgr()
{
	this->Logger = nullptr;
}

void CPreferenceMgr::LoadPrefsSafe()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	this->LoadPrefs();
}

void CPreferenceMgr::SavePrefsSafe()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	this->SavePrefs();
}

void CPreferenceMgr::LoadPrefs()
{
	this->Preferences.clear();

	if (!std::filesystem::exists(this->Path)) { return; }

	try
	{
		std::ifstream file(this->Path);

		json prefPack = json::parse(file);

		for (json& prefJSON : prefPack)
		{
			if (prefJSON[K_SIGNATURE].is_null()) { continue; }

			signed int sig = prefJSON[K_SIGNATURE].get<signed int>();

			if (sig == 0) { continue; }

			this->Preferences[sig] = prefJSON;
		}

		file.close();
	}
	catch (json::parse_error& ex)
	{
		this->Logger->Warning(CH_LOADER, "Prefs: %s could not be parsed. Error: %s", this->Path.filename().string().c_str(), ex.what());
	}
}

void CPreferenceMgr::SavePrefs()
{
	if (this->ReadOnly) { return; }

	json prefPack = json::array();

	for (auto& [sig, pref] : this->Preferences)
	{
		/* convert prefs */
		json prefJSON = pref.ToJSON();

		/* add signature */
		prefJSON[K_SIGNATURE] = sig;
	}

	std::ofstream file(this->Path);
	file << prefPack.dump(1, '\t') << std::endl;
	file.close();
}

AddonPrefs* CPreferenceMgr::RegisterPrefs(signed int aSignature)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	auto it = this->Preferences.find(aSignature);

	if (it != this->Preferences.end())
	{
		return &it->second;
	}

	/* set defaults */
	this->Preferences[aSignature] = AddonPrefs();

	this->SavePrefs();

	return &this->Preferences[aSignature];
}

void CPreferenceMgr::DeletePrefs(signed int aSignature)
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	this->Preferences.erase(aSignature);

	this->SavePrefs();
}
