///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Index.h
/// Description  :  Path index.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef INDEX_H
#define INDEX_H

#include <windows.h>
#include <filesystem>

#include "IdxEnum.h"

///----------------------------------------------------------------------------------------------------
/// CreateIndex:
/// 	Creates the index, relative to the provided module.
///----------------------------------------------------------------------------------------------------
void CreateIndex(HMODULE aModule);

///----------------------------------------------------------------------------------------------------
/// Index:
/// 	Retrieves a path from the index.
///----------------------------------------------------------------------------------------------------
std::filesystem::path Index(EPath aIndex);

#endif
