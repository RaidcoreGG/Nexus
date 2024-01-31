#ifndef APIREQUEST_H
#define APIREQUEST_H

#include <string>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

typedef void (*API_RESPONSE_CALLBACK)(json aResponse);

struct APIRequest
{
	API_RESPONSE_CALLBACK Callback;
	int Attempts;
	std::string Query;
};

#endif
