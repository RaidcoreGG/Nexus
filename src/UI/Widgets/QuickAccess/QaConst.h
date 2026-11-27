///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  QaConst.h
/// Description  :  Constant data for Quick Access.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include "QaEnum.h"
#include "UI/Services/Localization/LoclText.h"

namespace Raidcore::UI
{
	///----------------------------------------------------------------------------------------------------
	/// EQaVisibilityToText:
	/// 	Returns the display string of a visibility setting.
	///----------------------------------------------------------------------------------------------------
	UI::CText EQaVisibilityToText(EQaVisibility aQAVisibility);

	///----------------------------------------------------------------------------------------------------
	/// EQaPositionToText:
	/// 	Returns the display string of a position setting.
	///----------------------------------------------------------------------------------------------------
	UI::CText EQaPositionToText(EQaPosition aQAPosition);
}
