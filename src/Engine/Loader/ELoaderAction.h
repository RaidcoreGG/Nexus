#ifndef ELOADERACTION_H
#define ELOADERACTION_H

enum class ELoaderAction
{
	None,
	Load,
	Unload,
	Uninstall,
	Reload,
	FreeLibrary,
	FreeLibraryThenLoad
};

#endif