///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  QaConst.cpp
/// Description  :  Constant data for Quick Access.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "QaConst.h"

#include "Consts.h"

std::string EQaVisibilityToString(EQaVisibility aQAVisibility)
{
	switch (aQAVisibility)
	{
		case EQaVisibility::AlwaysShow:  return "((000047))";
		case EQaVisibility::Gameplay:    return "((000093))";
		case EQaVisibility::OutOfCombat: return "((000094))";
		case EQaVisibility::InCombat:    return "((000095))";
		case EQaVisibility::Hide:        return "((000096))";
		default:                         return NULLSTR;
	}
}

std::string EQaPositionToString(EQaPosition aQAPosition)
{
	switch (aQAPosition)
	{
		case EQaPosition::Extend: return "((000067))";
		case EQaPosition::Under:  return "((000068))";
		case EQaPosition::Bottom: return "((000069))";
		case EQaPosition::Custom: return "((000070))";
		default:                  return NULLSTR;
	}
}
