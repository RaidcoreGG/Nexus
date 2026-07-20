///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  CoContext.cpp
/// Description  :  Core context implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "CoContext.h"


#include "Core/DataLink/DlApi.h"
#include "Core/Logging/LogApi.h"
#include "Core/Preferences/PrefContext.h"
#include "Functions/FnRegistry.h"
#include "Index/IdxEnum.h"
#include "Index/Index.h"

namespace Raidcore::Nexus::Core
{
	Context::Context()
	{
		this->_LogApi = std::make_unique<Core::LogApi>();

		this->_DataLink = std::make_unique<Core::DataLinkApi>(
			this->_LogApi.get()
		);

		this->_FuncRegistry = std::make_unique<Core::FuncRegistry>(
			this->_LogApi.get()
		);

		this->_Settings = std::make_unique<Core::SettingsMgr>(
			Index(EPath::Settings),
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

	Core::FuncRegistry& Context::Function()
	{
		return *this->_FuncRegistry;
	}

	Core::SettingsMgr& Context::Settings()
	{
		return *this->_Settings;
	}
}
