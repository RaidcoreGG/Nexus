#pragma once

#include <stdint.h>

//TODO(Rennorb) @cleanup: I would put this into AddonDefinition.h, but that wil produce a cyclic dependency (ty c)
// so it just lived here for now.

/// Specificly u32 sized type used globally across nexus, namely in the networking implementation and addon deffinition.
typedef int32_t AddonSignature;

