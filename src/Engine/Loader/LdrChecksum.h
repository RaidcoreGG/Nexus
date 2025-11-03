///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  LdrChecksum.h
/// Description  :  MD5 data wrapper implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <vector>

#define MD5_LENGTH 16

///----------------------------------------------------------------------------------------------------
/// MD5_t Struct
///----------------------------------------------------------------------------------------------------
struct MD5_t
{
	uint8_t Data[MD5_LENGTH];

	MD5_t() = default;

	inline MD5_t(const std::vector<uint8_t>& aVector)
	{
		if (aVector.size() == MD5_LENGTH)
		{
			memcpy(this->Data, aVector.data(), MD5_LENGTH);
		}
		else if (aVector.size() == 0)
		{
			memset(this->Data, 0, sizeof(this->Data));
		}
		else
		{
			throw std::invalid_argument("MD5_t requires a vector of exactly 16 bytes.");
		}
	}

	inline bool operator==(const MD5_t& other) const
	{
		return memcmp(this->Data, other.Data, sizeof(this->Data)) == 0;
	}

	inline bool operator!=(const MD5_t& other) const
	{
		return !(*this == other);
	}

	inline bool empty() const
	{
		for (uint8_t byte : this->Data)
		{
			if (byte != 0)
			{
				return false;
			}
		}

		return true;
	}

	inline std::string string() const
	{
		std::ostringstream oss;
		for (int i = 0; i < MD5_LENGTH; ++i)
		{
			oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(Data[i]);
		}
		return oss.str();
	}

	inline void clear()
	{
		memset(this, 0, sizeof(MD5_t));
	}
};
