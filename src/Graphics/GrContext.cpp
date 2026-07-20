///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  GrContext.cpp
/// Description  :  Graphics context implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "GrContext.h"

#include "Index/IdxEnum.h"
#include "Index/Index.h"

namespace Raidcore::Nexus::Graphics
{
	Context::Context(Core::LogApi& aLogger)
		: _Logger(aLogger)
	{

		this->_Metrics = std::make_unique<Metrics_t>();
		this->_Window = std::make_unique<Window_t>();

		this->_TextureLoader = std::make_unique<Graphics::TextureLoader>(
			&this->_Logger,
			*this->_Window,
			Index(EPath::DIR_TEXTURES)
		);
	}

	void Context::Shutdown()
	{

	}

	Graphics::TextureLoader& Context::Textures()
	{
		return *this->_TextureLoader;
	}

	Graphics::Metrics_t& Context::Metrics()
	{
		return *this->_Metrics;
	}

	Graphics::Window_t& Context::Window()
	{
		return *this->_Window;
	}
}
