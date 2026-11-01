///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Resources.h
/// Description  :  Embedded files of Nexus, generated from Resources.inc.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "Core/Platform/Windows/Exe/ExePackFile.h"

///----------------------------------------------------------------------------------------------------
/// Resources Namespace
///----------------------------------------------------------------------------------------------------
namespace Resources
{
#define PACK_FILE(name, type, path) inline constexpr Raidcore::Core::Platform::Windows::CPackFile name{ #name, Raidcore::Core::Platform::Windows::EPackFileType::type };
#include "Resources.inc"
#undef PACK_FILE
}
