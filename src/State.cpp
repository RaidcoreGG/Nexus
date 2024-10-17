#include "State.h"

#include <Windows.h>
#include <string>
#include <algorithm>
#include <shellapi.h>
#include <regex>
#include <type_traits>

#include "Consts.h"
#include "Shared.h"
#include "Index.h"

#include "Util/Strings.h"
#include "Util/CmdLine.h"

namespace State
{
	ENexusState		Nexus						= ENexusState::NONE;
	EDxState		Directx						= EDxState::NONE;
	bool			IsChainloading				= false;
}

