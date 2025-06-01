#ifndef EUPDATEPROVIDER_H
#define EUPDATEPROVIDER_H

#include <string>

enum class EUpdateProvider
{
	None		= 0,	/* Does not support auto updating */
	Raidcore	= 1,	/* Provider is Raidcore (via API) */
	GitHub		= 2,	/* Provider is GitHub Releases */
	Direct		= 3,	/* Provider is direct file link */
	Self		= 4		/* Provider is self check, addon has to request manually and version will not be verified */
};

EUpdateProvider GetProvider(const std::string& aUrl);

#endif