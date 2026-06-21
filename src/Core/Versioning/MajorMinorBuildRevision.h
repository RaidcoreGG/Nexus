///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  MajorMinorBuildRevision.h
/// Description  :  Implementation of 64 Bit version format with 4x signed 16 Bit components.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <string>
#include <cstdint>

///----------------------------------------------------------------------------------------------------
/// MajorMinorBuildRevision_t Struct
///----------------------------------------------------------------------------------------------------
struct MajorMinorBuildRevision_t
{
	int16_t Major;
	int16_t Minor;
	int16_t Build;
	int16_t Revision;

	MajorMinorBuildRevision_t() = default;

	inline MajorMinorBuildRevision_t(uint16_t aMajor, uint16_t aMinor, uint16_t aBuild, uint16_t aRevision)
	{
		this->Major = aMajor;
		this->Minor = aMinor;
		this->Build = aBuild;
		this->Revision = aRevision;
	}

	inline std::string string() const
	{
		std::string str;
		str.append(std::to_string(this->Major));
		str.append(".");
		str.append(std::to_string(this->Minor));
		str.append(".");
		str.append(std::to_string(this->Build));
		if (this->Revision > -1)
		{
			str.append(".");
			str.append(std::to_string(this->Revision));
		}
		return str;
	}

	inline bool operator>(const MajorMinorBuildRevision_t& rhs)
	{
		if (this->Major    != rhs.Major)    { return this->Major    > rhs.Major;    }
		if (this->Minor    != rhs.Minor)    { return this->Minor    > rhs.Minor;    }
		if (this->Build    != rhs.Build)    { return this->Build    > rhs.Build;    }
		if (this->Revision != rhs.Revision) { return this->Revision > rhs.Revision; }

		return false;
	}

	inline bool operator<(const MajorMinorBuildRevision_t& rhs)
	{
		if (this->Major    != rhs.Major)    { return this->Major    < rhs.Major;    }
		if (this->Minor    != rhs.Minor)    { return this->Minor    < rhs.Minor;    }
		if (this->Build    != rhs.Build)    { return this->Build    < rhs.Build;    }
		if (this->Revision != rhs.Revision) { return this->Revision < rhs.Revision; }

		return false;
	}

	inline bool operator==(const MajorMinorBuildRevision_t& rhs)
	{
		if (this->Major    != rhs.Major)    { return false; }
		if (this->Minor    != rhs.Minor)    { return false; }
		if (this->Build    != rhs.Build)    { return false; }
		if (this->Revision != rhs.Revision) { return false; }

		return true;
	}

	inline bool operator!=(const MajorMinorBuildRevision_t& rhs)
	{
		return !(*this == rhs);
	}

	inline bool operator>=(const MajorMinorBuildRevision_t& rhs)
	{
		return *this > rhs || *this == rhs;
	}

	inline bool operator<=(const MajorMinorBuildRevision_t& rhs)
	{
		return *this < rhs || *this == rhs;
	}
};
