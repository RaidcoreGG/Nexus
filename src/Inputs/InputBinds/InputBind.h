#ifndef INPUTBIND_H
#define INPUTBIND_H

#include <string>

/* A structure holding information about a InputBind. */
struct InputBind
{
	unsigned short	Key;
	bool			Alt;
	bool			Ctrl;
	bool			Shift;

	bool IsBound();
};

bool operator==(const InputBind& lhs, const InputBind& rhs);
bool operator!=(const InputBind& lhs, const InputBind& rhs);
#endif