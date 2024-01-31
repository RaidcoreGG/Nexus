#ifndef CACHEDRESPONSE_H
#define CACHEDRESPONSE_H

#include "nlohmann/json.hpp"
using json = nlohmann::json;

struct CachedResponse
{
	long long Timestamp;
	json Content;
};

#endif