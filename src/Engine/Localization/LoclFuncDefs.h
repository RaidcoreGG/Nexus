///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LoclFuncDefs.h
/// Description  :  Function definitions for localization.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef LOCLFUNCDEFS_H
#define LOCLFUNCDEFS_H

typedef const char* (*LOCALIZATION_TRANSLATE)  (const char* aIdentifier);
typedef const char* (*LOCALIZATION_TRANSLATETO)(const char* aIdentifier, const char* aLanguageIdentifier);
typedef void        (*LOCALIZATION_SET)        (const char* aIdentifier, const char* aLanguageIdentifier, const char* aString);

#endif
