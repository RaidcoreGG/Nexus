#ifndef STATE_H
#define STATE_H

/* Namespace for global state variables */
namespace State
{
	extern bool IsShutdown;
	extern bool IsDxLoaded;
	extern bool IsDxCreated;
	extern bool IsChainloading;
	extern bool IsImGuiInitialized;
	extern bool IsImGuiInitializable;
}

#endif