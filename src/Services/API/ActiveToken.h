#ifndef API_LOADEDTOKEN_H
#define API_LOADEDTOKEN_H

#include <string>
#include <vector>

struct ActiveToken_t
{
	std::string					Key;
	std::string					AccountName;
	std::vector<std::string>	Characters;
};

bool operator==(const ActiveToken_t& lhs, const ActiveToken_t& rhs);
bool operator!=(const ActiveToken_t& lhs, const ActiveToken_t& rhs);

#endif