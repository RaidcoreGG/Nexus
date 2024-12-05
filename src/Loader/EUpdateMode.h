///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  EUpdateMode.h
/// Description  :  Contains the update behaviors.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef EUPDATEMODE_H
#define EUPDATEMODE_H

///----------------------------------------------------------------------------------------------------
/// EUpdateMode Enumeration
///----------------------------------------------------------------------------------------------------
enum class EUpdateMode
{
	None,
	AutoUpdate, /* Automatically check and perform updates. */
	Prompt,     /* Automatically check for updates, but prompt to perform. */
	Disabled    /* Automatically check for updates, but do nothing. */
};

#endif
