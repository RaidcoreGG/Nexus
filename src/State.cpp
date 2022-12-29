#include "State.h"

namespace State
{
	bool IsShutdown					= false;
	bool IsDxLoaded					= false;
	bool IsDxCreated				= false;
	bool IsChainloading				= false;
	bool IsImGuiInitialized			= false;
	bool IsImGuiInitializable		= false;
	bool IsAddonLibraryInitialized	= false;

	bool IsDeveloperMode			= false;
	bool IsVanilla					= false;
	bool IsConsoleEnabled			= false;
}