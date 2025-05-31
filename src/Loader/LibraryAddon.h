#ifndef LIBRARYADDON_H
#define LIBRARYADDON_H

#include <string>

#include "EUpdateProvider.h"

struct LibraryAddon_t
{
	signed int		Signature;
	std::string		Name;
	std::string		Author;
	std::string		Description;
	EUpdateProvider	Provider;
	std::string		DownloadURL;
	std::string		ToSComplianceNotice;
	int				PolicyTier;
	bool			IsNew;

	bool			IsInstalling = false;
};

#endif