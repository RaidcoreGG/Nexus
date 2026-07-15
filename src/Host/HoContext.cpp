///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  HoContext.cpp
/// Description  :  Host context implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "HoContext.h"

#include "Host/Addons/Addon.h"
#include "thirdparty/Clockwork/Clockwork.h"
#include "Util/CmdLine.h"
#include "Util/Strings.h"

namespace Raidcore::Nexus::Host
{
	Context::Context(
		CLogApi&              aLogger,
		std::filesystem::path aLoaderDirectory,
		std::filesystem::path aAddonConfigDefaultPath
	)
		: _Logger(aLogger)
		, _LoaderDirectoryPath(std::move(aLoaderDirectory))
		, _AddonConfigDefaultPath(std::move(aAddonConfigDefaultPath))
	{

		std::filesystem::path cfgpath = this->_AddonConfigDefaultPath;
		std::vector<uint32_t> cfgwhitelist = {};

		if (CmdLine::HasArgument("-ggaddons"))
		{
			std::vector<std::string> idList = String::Split(CmdLine::GetArgumentValue("-ggaddons"), ",");

			/* If only one entry and it contains ".json" it's a custom config. */
			if (idList.size() == 1 && String::Contains(idList[0], ".json"))
			{
				cfgpath = idList[0];
			}
			else
			{
				for (std::string addonsig : idList)
				{
					try
					{
						uint32_t sig = std::stoi(addonsig, nullptr, 0);
						cfgwhitelist.push_back(sig);
					}
					catch (const std::invalid_argument& e)
					{
						this->_Logger.Trace(CH_LOADER, "Invalid argument(-ggaddons) : %s (exc: %s)", addonsig.c_str(), e.what());
					}
					catch (const std::out_of_range& e)
					{
						this->_Logger.Trace(CH_LOADER, "Out of range (-ggaddons): %s (exc: %s)", addonsig.c_str(), e.what());
					}
				}
			}
		}

		this->_ConfigMgr = std::make_unique<Host::ConfigMgr>(
			&this->_Logger,
			cfgpath,
			cfgwhitelist
		);
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
		this->_ConfigMgr.reset();
		this->_Loader.reset();
		this->_Library.reset();
		this->_EventApi.reset();
	}

	Host::ConfigMgr& Context::Config()
	{
		return *this->_ConfigMgr;
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
