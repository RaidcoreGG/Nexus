#include "State.h"

namespace State
{
	ggState	AddonHost					= ggState::NONE;
	DxState	Directx						= DxState::NONE;
	bool	IsChainloading				= false;
	bool	IsImGuiInitialized			= false;

	bool	IsDeveloperMode				= false;
	bool	IsVanilla					= false;
	bool	IsConsoleEnabled			= false;
}