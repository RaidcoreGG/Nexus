#include "EAddonState.h"

/// returns null if value is not in enum
Option<char const*> ToString(EAddonState state) {
	switch(state) {
		case EAddonState::None:                     return Some("None");
		case EAddonState::NotLoaded:                return Some("NotLoaded");
		case EAddonState::NotLoadedDuplicate:       return Some("NotLoadedDuplicate");
		case EAddonState::NotLoadedIncompatible:    return Some("NotLoadedIncompatible");
		case EAddonState::NotLoadedIncompatibleAPI: return Some("NotLoadedIncompatibleAPI");
		case EAddonState::Loaded:                   return Some("Loaded");
		case EAddonState::LoadedLOCKED:             return Some("LoadedLOCKED");
		default: return None<char const*>();
	}
}

