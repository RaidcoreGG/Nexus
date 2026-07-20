///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  CoContext.cpp
/// Description  :  Core context implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "CoContext.h"

#include <filesystem>
#include <memory>

#include "Core/Preferences/PrefContext.h"
#include "Core/DataLink/DlApi.h"
#include "Core/Logging/LogApi.h"

namespace Raidcore::Nexus::Core
{
	Context::Context(std::filesystem::path aSettingsPath)
		: _SettingsPath(aSettingsPath)
	{
		this->_LogApi = std::make_unique<Core::LogApi>();

		this->_DataLink = std::make_unique<Core::DataLinkApi>(
			this->_LogApi.get()
		);

		this->_Settings = std::make_unique<Core::SettingsMgr>(
			this->_SettingsPath,
			this->_LogApi.get()
		);
	}

	void Context::Shutdown()
	{
		this->_DataLink.reset();
		this->_LogApi.reset();
		this->_Settings.reset();
	}

	Core::LogApi& Context::Logger()
	{
		return *this->_LogApi;
	}

	Core::DataLinkApi& Context::DataLink()
	{
		return *this->_DataLink;
	}

	Core::SettingsMgr& Context::Settings()
	{
		return *this->_Settings;
	}
}
