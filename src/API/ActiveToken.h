#ifndef API_LOADEDTOKEN_H
#define API_LOADEDTOKEN_H

#include <string>
#include <vector>

struct ActiveToken
{
	std::string					Key;
	std::string					AccountName;
	std::vector<std::string>	Characters;
};

bool operator==(const ActiveToken& lhs, const ActiveToken& rhs);
bool operator!=(const ActiveToken& lhs, const ActiveToken& rhs);

#endif