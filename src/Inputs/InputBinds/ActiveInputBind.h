#ifndef ACTIVEINPUTBIND_H
#define ACTIVEINPUTBIND_H

#include "InputBind.h"
#include "FuncDefs.h"

typedef enum class EInputBindHandlerType
{
	None,
	DownOnly,
	DownAndRelease
} EIBHType;

struct ActiveInputBind
{
	InputBind					Bind;
	EInputBindHandlerType		HandlerType;
	void*					Handler;
};

#endif
