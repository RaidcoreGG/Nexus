///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LogBase.cpp
/// Description  :  Interface for logger implementations.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "LogBase.h"

namespace Raidcore::Nexus::Core
{
	ELogLevel ILogger::GetLogLevel()
	{
		return this->Level;
	}

	void ILogger::SetLogLevel(ELogLevel aLogLevel)
	{
		this->Level = aLogLevel;
	}
}
