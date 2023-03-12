#ifndef EADDONFLAGS_H
#define EADDONFLAGS_H

enum class EAddonFlags
{
	None = 0,
	HasOptions = 1 << 1,           /* should an options button be drawn and the event fired? */
	IsVolatile = 1 << 2,           /* is hooking functions or doing anything else that's volatile and game build dependant */
};

EAddonFlags operator|(EAddonFlags lhs, EAddonFlags rhs);

EAddonFlags operator&(EAddonFlags lhs, EAddonFlags rhs);

EAddonFlags operator==(EAddonFlags lhs, EAddonFlags rhs);

EAddonFlags operator!=(EAddonFlags lhs, EAddonFlags rhs);

#endif