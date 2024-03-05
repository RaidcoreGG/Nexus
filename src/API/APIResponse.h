#ifndef APIRESPONSE_H
#define APIRESPONSE_H

#include "nlohmann/json.hpp"
using json = nlohmann::json;

struct APIResponse
{
	int Status;
	json Content;
	//void Headers;
};

#endif
