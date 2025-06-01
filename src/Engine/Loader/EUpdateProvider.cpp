#include "EUpdateProvider.h"

#include "Util/Strings.h"

EUpdateProvider GetProvider(const std::string& aUrl)
{
	if (aUrl.empty())
	{
		return EUpdateProvider::None;
	}

	if (String::Contains(aUrl, "raidcore.gg"))
	{
		return EUpdateProvider::Raidcore;
	}

	if (String::Contains(aUrl, "github.com"))
	{
		return EUpdateProvider::GitHub;
	}

	return EUpdateProvider::Direct;
}