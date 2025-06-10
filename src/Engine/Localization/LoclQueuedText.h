///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LoclQueuedText.h
/// Description  :  Definition for queued text struct.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef LOCLQUEUEDTEXT_H
#define LOCLQUEUEDTEXT_H

#include <string>

///----------------------------------------------------------------------------------------------------
/// QueuedText_t Struct
///----------------------------------------------------------------------------------------------------
struct QueuedText_t
{
	std::string Identifier;
	std::string LanguageIdentifier;
	std::string Text;
};

#endif
