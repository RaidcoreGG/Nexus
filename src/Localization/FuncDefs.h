#ifndef LOCALIZATION_FUNCDEFS_H
#define LOCALIZATION_FUNCDEFS_H

typedef const char* (*LOCALIZATION_TRANSLATE)(const char* aIdentifier);
typedef const char* (*LOCALIZATION_TRANSLATETO)(const char* aIdentifier, const char* aLanguageIdentifier);
/*typedef void (*LOCALIZATION_SET)(const char* aIdentifier, const char* aLanguageIdentifier, const char* aString);*/

#endif