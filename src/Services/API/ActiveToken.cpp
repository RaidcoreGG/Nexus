#include "ActiveToken.h"

bool operator==(const ActiveToken_t& lhs, const ActiveToken_t& rhs)
{
	return	lhs.Key == rhs.Key;
}

bool operator!=(const ActiveToken_t& lhs, const ActiveToken_t& rhs)
{
	return	!(lhs == rhs);
}