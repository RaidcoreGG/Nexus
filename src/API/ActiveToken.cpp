#include "ActiveToken.h"

bool operator==(const ActiveToken& lhs, const ActiveToken& rhs)
{
	return	lhs.Key == rhs.Key;
}

bool operator!=(const ActiveToken& lhs, const ActiveToken& rhs)
{
	return	!(lhs == rhs);
}