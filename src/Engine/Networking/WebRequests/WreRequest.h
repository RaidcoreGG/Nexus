#ifndef APIREQUEST_H
#define APIREQUEST_H

#include <string>
#include <condition_variable>

#include "WreCacheEntry.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

enum class ERequestType
{
	None,
	Get,
	Post
};

struct APIRequest_t
{
	ERequestType             Type;
	bool*                    IsComplete;
	std::condition_variable* CV;
	int                      Attempts;
	std::string              Query;
};

#endif
