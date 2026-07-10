///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  HoContext.cpp
/// Description  :  Host context implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "HoContext.h"

#include "Core/Addons/Addon.h"
#include "Engine/Clockwork/Clockwork.h"

namespace Raidcore::Nexus::Host
{
	Context::Context(CLogApi& aLogger, std::filesystem::path aLoaderDirectory)
		: _Logger(aLogger)
		, _LoaderDirectoryPath(std::move(aLoaderDirectory))
	{
		this->_Loader = std::make_unique<Host::Loader>(
			&this->_Logger,
			CAddon::Factory, /* FIXME */
			this->_LoaderDirectoryPath
		);
		this->_Library = std::make_unique<Host::LibraryMgr>(
			&this->_Logger,
			this->_Loader.get()
		);
		this->_EventApi = std::make_unique<Host::EventApi>(
			this->_Loader.get()
		);

		Clockwork::Run<void>(Raidcore::Clockwork::ETaskPriority::Normal, [this](Clockwork::CancellationToken aToken)
		{
			this->Library().AddSource("https://api.raidcore.gg/addonlibrary");
			this->Library().AddSource("https://api.raidcore.gg/arcdpslibrary");
			this->Library().Update();
		});
	}

	void Context::Shutdown()
	{
		this->_Loader.reset();
		this->_Library.reset();
		this->_EventApi.reset();
	}

	Host::Loader& Context::Loader()
	{
		return *this->_Loader;
	}

	Host::LibraryMgr& Context::Library()
	{
		return *this->_Library;
	}

	Host::EventApi& Context::Events()
	{
		return *this->_EventApi;
	}
}
