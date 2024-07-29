#include "InputBind.h"

#include <Windows.h>
#include <algorithm>
#include <map>

#include "InputBindHandler.h"

bool InputBind::IsBound()
{
	return *this != InputBind{};
}

bool operator==(const InputBind& lhs, const InputBind& rhs)
{
	return	lhs.Key		== rhs.Key &&
			lhs.Alt		== rhs.Alt &&
			lhs.Ctrl	== rhs.Ctrl &&
			lhs.Shift	== rhs.Shift;
}

bool operator!=(const InputBind& lhs, const InputBind& rhs)
{
	return	!(lhs == rhs);
}