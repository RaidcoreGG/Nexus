///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  QaConst.h
/// Description  :  Constant data for Quick Access.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef QACONST_H
#define QACONST_H

#include <string>

#include "QaEnum.h"

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

#endif
