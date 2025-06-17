///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  EvtFuncDefs.h
/// Description  :  Function definitions for events.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef EVTFUNCDEFS_H
#define EVTFUNCDEFS_H

typedef void (*EVENT_CONSUME)                    (void* aEventArgs);
typedef void (*EVENTS_RAISE)                     (const char* aIdentifier, void* aEventData);
typedef void (*EVENTS_RAISENOTIFICATION)         (const char* aIdentifier);
typedef void (*EVENTS_RAISE_TARGETED)            (uint32_t aSignature, const char* aIdentifier, void* aEventData);
typedef void (*EVENTS_RAISENOTIFICATION_TARGETED)(uint32_t aSignature, const char* aIdentifier);
typedef void (*EVENTS_SUBSCRIBE)                 (const char* aIdentifier, EVENT_CONSUME aConsumeEventCallback);

#endif
