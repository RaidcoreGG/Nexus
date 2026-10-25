///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  MajorMinorBuildRevision.h
/// Description  :  Implementation of 64 Bit version format with 4x signed 16 Bit components.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <array>
#include <cstdint>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>

const inline std::regex VERSION_REGEX(R"(v?\d+\.\d+(?:\.\d+){0,2})");

///----------------------------------------------------------------------------------------------------
/// Version_t Struct
///----------------------------------------------------------------------------------------------------
struct Version_t
{
	int16_t Major;
	int16_t Minor;
	int16_t Build;
	int16_t Revision;

	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	Version_t() = default;

	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	Version_t(int16_t aMajor, int16_t aMinor, int16_t aBuild, int16_t aRevision)
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

		/* Omitted components stay 0, so "1.2" equals "1.2.0.0". */
		std::array<int16_t, 4> parts{};
		size_t count = 0;

		while (!aVersionStr.empty() && count < parts.size())
		{
			const size_t pos = aVersionStr.find('.');
			const long value = std::stol(std::string(aVersionStr.substr(0, pos)));

			if (value > INT16_MAX)
			{
				throw std::out_of_range("Version component out of range: " + std::string(aVersionStr));
			}

			parts[count++] = static_cast<int16_t>(value);

			if (pos == std::string_view::npos)
			{
				break;
			}

			aVersionStr.remove_prefix(pos + 1);
		}

		this->Major    = parts[0];
		this->Minor    = parts[1];
		this->Build    = parts[2];
		this->Revision = parts[3];
	}

	///----------------------------------------------------------------------------------------------------
	/// string:
	/// 	Builds a string representation of the version in the format "Major.Minor.Build.Revision".
	/// 	Revision is omitted if it is 0 or negative.
	///----------------------------------------------------------------------------------------------------
	inline std::string string() const
	{
		std::string str;
		str.append(std::to_string(this->Major));
		str.append(".");
		str.append(std::to_string(this->Minor));
		str.append(".");
		str.append(std::to_string(this->Build));

		if (this->Revision > 0)
		{
			str.append(".");
			str.append(std::to_string(this->Revision));
		}

		return str;
	}

	///----------------------------------------------------------------------------------------------------
	/// Compare:
	/// 	Returns <0, 0 or >0. Negative components (legacy -1 "omitted") compare as 0.
	///----------------------------------------------------------------------------------------------------
	inline int Compare(const Version_t& rhs) const
	{
		const std::array<int16_t, 4> lhsParts{ this->Major, this->Minor, this->Build, this->Revision };
		const std::array<int16_t, 4> rhsParts{ rhs.Major, rhs.Minor, rhs.Build, rhs.Revision };

		for (size_t i = 0; i < lhsParts.size(); i++)
		{
			const int lhsVal = lhsParts[i] < 0 ? 0 : lhsParts[i];
			const int rhsVal = rhsParts[i] < 0 ? 0 : rhsParts[i];

			if (lhsVal != rhsVal) { return lhsVal - rhsVal; }
		}

		return 0;
	}

	inline bool operator> (const Version_t& rhs) const { return this->Compare(rhs) >  0; }
	inline bool operator< (const Version_t& rhs) const { return this->Compare(rhs) <  0; }
	inline bool operator==(const Version_t& rhs) const { return this->Compare(rhs) == 0; }
	inline bool operator!=(const Version_t& rhs) const { return this->Compare(rhs) != 0; }
	inline bool operator>=(const Version_t& rhs) const { return this->Compare(rhs) >= 0; }
	inline bool operator<=(const Version_t& rhs) const { return this->Compare(rhs) <= 0; }
};
