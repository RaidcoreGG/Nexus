///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  VerU64_4XS16.h
/// Description  :  Implementation of 64 Bit version format with 4x signed 16 Bit components.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef VERU64_4XS16_H
#define VERU64_4XS16_H

#include <cstdint>

#include "VerBase.h"

///----------------------------------------------------------------------------------------------------
/// VerU64_4XS16_t Struct
/// 	Contains purely the data for the version format.
///----------------------------------------------------------------------------------------------------
struct VerU64_4XS16_t
{
	int16_t Major;
	int16_t Minor;
	int16_t Build;
	int16_t Revision;
};

///----------------------------------------------------------------------------------------------------
/// MajorMinorBuildRevision_t Struct
///----------------------------------------------------------------------------------------------------
struct MajorMinorBuildRevision_t : VerU64_4XS16_t, virtual IVersionBase
{
	MajorMinorBuildRevision_t() = default;

	inline MajorMinorBuildRevision_t(VerU64_4XS16_t aVersion)
	{
		this->Major = aVersion.Major;
		this->Minor = aVersion.Minor;
		this->Build = aVersion.Build;
		this->Revision = aVersion.Revision;
	}

	inline MajorMinorBuildRevision_t(uint16_t aMajor, uint16_t aMinor, uint16_t aBuild, uint16_t aRevision)
	{
		this->Major = aMajor;
		this->Minor = aMinor;
		this->Build = aBuild;
		this->Revision = aRevision;
	}

	inline std::string string() const override
	{
		std::string str;
		str.append(std::to_string(this->Major));
		str.append(".");
		str.append(std::to_string(this->Minor));
		str.append(".");
		str.append(std::to_string(this->Build));
		str.append(".");
		str.append(std::to_string(this->Revision));
		return str;
	}

	inline int32_t CompareTo(const IVersionBase& rhs) const override
	{
		const MajorMinorBuildRevision_t* other = dynamic_cast<const MajorMinorBuildRevision_t*>(&rhs);
		if (!other)
		{
			throw "Illegal comparsion of two different version formats.";
		}

		if (this->Major    != other->Major)    { return (this->Major    > other->Major)    ? 1 : -1; }
		if (this->Minor    != other->Minor)    { return (this->Minor    > other->Minor)    ? 1 : -1; }
		if (this->Build    != other->Build)    { return (this->Build    > other->Build)    ? 1 : -1; }
		if (this->Revision != other->Revision) { return (this->Revision > other->Revision) ? 1 : -1; }

		return 0;
	}
};

#endif
