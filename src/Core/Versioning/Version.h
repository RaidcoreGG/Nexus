///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  MajorMinorBuildRevision.h
/// Description  :  Implementation of 64 Bit version format with 4x unsigned 16 Bit components.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <array>
#include <cstdint>
#include <regex>
#include <string>
#include <string_view>

const std::regex VERSION_REGEX(R"(v?\d+\.\d+(?:\.\d+){0,2})");

///----------------------------------------------------------------------------------------------------
/// Version_t Struct
///----------------------------------------------------------------------------------------------------
struct Version_t
{
	uint16_t Major;
	uint16_t Minor;
	uint16_t Build;
	uint16_t Revision;

	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	Version_t() = default;

	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	Version_t(uint16_t aMajor, uint16_t aMinor, uint16_t aBuild, uint16_t aRevision)
		: Major(aMajor)
		, Minor(aMinor)
		, Build(aBuild)
		, Revision(aRevision)
	{}

	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	inline Version_t(std::string_view aVersionStr)
	{
		if (!std::regex_match(std::string(aVersionStr), VERSION_REGEX))
		{
			throw std::invalid_argument("Invalid version string format: " + std::string(aVersionStr));
		}

		this->Major = 0;
		this->Minor = 0;
		this->Build = 0;
		this->Revision = 0;

		if (!aVersionStr.empty() && aVersionStr.front() == 'v')
		{
			aVersionStr.remove_prefix(1);
		}

		std::array<uint16_t, 4> parts{};
		size_t count = 0;

		while (!aVersionStr.empty() && count < parts.size())
		{
			const size_t pos = aVersionStr.find('.');
			if (pos == std::string_view::npos)
			{
				parts[count++] = static_cast<uint16_t>(std::stoul(std::string(aVersionStr)));
				break;
			}

			parts[count++] = static_cast<uint16_t>(std::stoul(std::string(aVersionStr.substr(0, pos))));
			aVersionStr.remove_prefix(pos + 1);
		}

		this->Major = parts[0];
		this->Minor = parts[1];

		if (count == 3)
		{
			this->Build = parts[2];
		}
		else if (count == 4)
		{
			this->Build = parts[2];
			this->Revision = parts[3];
		}
		else
		{
			throw std::invalid_argument("Unreachable code. Invalid version string format: " + std::string(aVersionStr));
		}
	}

	///----------------------------------------------------------------------------------------------------
	/// string:
	/// 	Builds a string representation of the version in the format "Major.Minor.Build.Revision".
	/// 	If Build or Revision are 0, they will be omitted from the string.
	///----------------------------------------------------------------------------------------------------
	inline std::string string() const
	{
		std::string str;
		str.append(std::to_string(this->Major));
		str.append(".");
		str.append(std::to_string(this->Minor));
		if (this->Build > 0 || this->Revision > 0)
		{
			str.append(".");
			str.append(std::to_string(this->Build));

			if (this->Revision > 0)
			{
				str.append(".");
				str.append(std::to_string(this->Revision));
			}
		}

		return str;
	}

	inline bool operator>(const Version_t& rhs)
	{
		if (this->Major    != rhs.Major)    { return this->Major    > rhs.Major;    }
		if (this->Minor    != rhs.Minor)    { return this->Minor    > rhs.Minor;    }
		if (this->Build    != rhs.Build)    { return this->Build    > rhs.Build;    }
		if (this->Revision != rhs.Revision) { return this->Revision > rhs.Revision; }

		return false;
	}

	inline bool operator<(const Version_t& rhs)
	{
		if (this->Major    != rhs.Major)    { return this->Major    < rhs.Major;    }
		if (this->Minor    != rhs.Minor)    { return this->Minor    < rhs.Minor;    }
		if (this->Build    != rhs.Build)    { return this->Build    < rhs.Build;    }
		if (this->Revision != rhs.Revision) { return this->Revision < rhs.Revision; }

		return false;
	}

	inline bool operator==(const Version_t& rhs)
	{
		if (this->Major    != rhs.Major)    { return false; }
		if (this->Minor    != rhs.Minor)    { return false; }
		if (this->Build    != rhs.Build)    { return false; }
		if (this->Revision != rhs.Revision) { return false; }

		return true;
	}

	inline bool operator!=(const Version_t& rhs)
	{
		return !(*this == rhs);
	}

	inline bool operator>=(const Version_t& rhs)
	{
		return *this > rhs || *this == rhs;
	}

	inline bool operator<=(const Version_t& rhs)
	{
		return *this < rhs || *this == rhs;
	}
};
