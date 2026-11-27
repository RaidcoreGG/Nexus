///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  QaConst.cpp
/// Description  :  Constant data for Quick Access.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "QaConst.h"

namespace Raidcore::UI
{
	UI::CText EQaVisibilityToText(EQaVisibility aQAVisibility)
	{
		switch (aQAVisibility)
		{
			case EQaVisibility::AlwaysShow:  return Lang::QuickAccess::AlwaysShow;
			case EQaVisibility::Gameplay:    return Lang::QuickAccess::DuringGameplay;
			case EQaVisibility::OutOfCombat: return Lang::QuickAccess::OutOfCombat;
			case EQaVisibility::InCombat:    return Lang::QuickAccess::InCombat;
			case EQaVisibility::Hide:        return Lang::QuickAccess::NeverShow;
			default:                         return UI::CText{};
		}
	}

	UI::CText EQaPositionToText(EQaPosition aQAPosition)
	{
		switch (aQAPosition)
		{
			case EQaPosition::Extend: return Lang::QuickAccess::PosExtend;
			case EQaPosition::Under:  return Lang::QuickAccess::PosUnder;
			case EQaPosition::Bottom: return Lang::QuickAccess::PosBottom;
			case EQaPosition::Custom: return Lang::QuickAccess::PosCustom;
			default:                  return UI::CText{};
		}
	}
}
