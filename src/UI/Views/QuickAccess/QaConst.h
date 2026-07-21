///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  QaConst.h
/// Description  :  Constant data for Quick Access.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <string>

#include "QaEnum.h"

///----------------------------------------------------------------------------------------------------
/// Raidcore::Nexus::GUI Namespace
///----------------------------------------------------------------------------------------------------
namespace Raidcore::Nexus::GUI
{
	///----------------------------------------------------------------------------------------------------
	/// EQaVisibilityToString:
	/// 	Returns a localizable string for a visibility setting.
	///----------------------------------------------------------------------------------------------------
	std::string EQaVisibilityToString(EQaVisibility aQAVisibility);

	///----------------------------------------------------------------------------------------------------
	/// EQaPositionToString:
	/// 	Returns a localizable string for a position setting.
	///----------------------------------------------------------------------------------------------------
	std::string EQaPositionToString(EQaPosition aQAPosition);
}
