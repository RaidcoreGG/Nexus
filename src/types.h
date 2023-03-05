#ifndef TYPES_H
#define TYPES_H

#include <cmath>

struct Vector2
{
	float X;
	float Y;
};

bool operator==(const Vector2& lhs, const Vector2& rhs);

bool operator!=(const Vector2& lhs, const Vector2& rhs);

struct Vector3
{
	float X;
	float Y;
	float Z;
};

bool operator==(const Vector3& lhs, const Vector3& rhs);

bool operator!=(const Vector3& lhs, const Vector3& rhs);

#endif