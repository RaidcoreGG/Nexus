#pragma once

///----------------------------------------------------------------------------------------------------
/// Raidcore::Nexus::GUI Namespace
///----------------------------------------------------------------------------------------------------
namespace Raidcore::Nexus::GUI
{
	typedef void (*GUI_REGISTERCLOSEONESCAPE)(const char* aWindowName, bool* aIsVisible);
	typedef void (*GUI_DEREGISTERCLOSEONESCAPE)(const char* aWindowName);
}
