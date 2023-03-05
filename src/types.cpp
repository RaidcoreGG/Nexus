#include "types.h"

bool operator==(const Vector2& lhs, const Vector2& rhs)
{
	if (trunc(1000. * lhs.X) == trunc(1000. * rhs.X) &&
		trunc(1000. * lhs.Y) == trunc(1000. * rhs.Y))
	{
		return true;
	}
	return false;
}

bool operator!=(const Vector2& lhs, const Vector2& rhs)
{
	return !(lhs == rhs);
}

bool operator==(const Vector3& lhs, const Vector3& rhs)
{
	if (trunc(1000. * lhs.X) == trunc(1000. * rhs.X) &&
		trunc(1000. * lhs.Y) == trunc(1000. * rhs.Y) &&
		trunc(1000. * lhs.Z) == trunc(1000. * rhs.Z))
	{
		return true;
	}
	return false;
}

bool operator!=(const Vector3& lhs, const Vector3& rhs)
{
	return !(lhs == rhs);
}