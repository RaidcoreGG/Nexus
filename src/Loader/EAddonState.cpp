#include "EAddonState.h"

/// returns null if value is not in enum
_Ret_maybenull_z_
char const* ToString(EAddonState state) {
	switch(state) {
		case EAddonState::None:                     return "None";
		case EAddonState::NotLoaded:                return "NotLoaded";
		case EAddonState::NotLoadedDuplicate:       return "NotLoadedDuplicate";
		case EAddonState::NotLoadedIncompatible:    return "NotLoadedIncompatible";
		case EAddonState::NotLoadedIncompatibleAPI: return "NotLoadedIncompatibleAPI";
		case EAddonState::Loaded:                   return "Loaded";
		case EAddonState::LoadedLOCKED:             return "LoadedLOCKED";
		default: return 0;
	}
}

