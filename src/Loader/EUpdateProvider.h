#ifndef EUPDATEPROVIDER_H
#define EUPDATEPROVIDER_H

#include <string>

enum class EUpdateProvider
{
	None		= 0,	/* Does not support auto updating */
	Raidcore	= 1,	/* Provider is Raidcore (via API) */
	GitHub		= 2,	/* Provider is GitHub Releases */
	Direct		= 3		/* Provider is direct file link */
};

EUpdateProvider GetProvider(const std::string& aUrl);

#endif