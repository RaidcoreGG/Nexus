#include "EAddonFlags.h"

#include <type_traits>

EAddonFlags operator|(EAddonFlags lhs, EAddonFlags rhs)
{
	return static_cast<EAddonFlags>(
		std::underlying_type_t<EAddonFlags>(lhs) |
		std::underlying_type_t<EAddonFlags>(rhs)
		);
}

EAddonFlags operator&(EAddonFlags lhs, EAddonFlags rhs)
{
	return static_cast<EAddonFlags>(
		std::underlying_type_t<EAddonFlags>(lhs) &
		std::underlying_type_t<EAddonFlags>(rhs)
		);
}