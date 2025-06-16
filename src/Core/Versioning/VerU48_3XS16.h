///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  VerU48_3XS16.h
/// Description  :  Implementation of 48 Bit version format with 3x signed 16 Bit components.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef VERU48_3XS16_H
#define VERU48_3XS16_H

#include <cstdint>

#include "VerBase.h"

///----------------------------------------------------------------------------------------------------
/// VerU48_3XS16_t Struct
/// 	Contains purely the data for the version format.
///----------------------------------------------------------------------------------------------------
struct VerU48_3XS16_t
{
	int16_t Major;
	int16_t Minor;
	int16_t Patch;
};

///----------------------------------------------------------------------------------------------------
/// MajorMinorPatch_t Struct
///----------------------------------------------------------------------------------------------------
struct MajorMinorPatch_t : VerU48_3XS16_t, virtual IVersionBase
{
	inline std::string string() const override
	{
		std::string str;
		str.append(std::to_string(this->Major));
		str.append(".");
		str.append(std::to_string(this->Minor));
		str.append(".");
		str.append(std::to_string(this->Patch));
		return str;
	}

	inline int32_t CompareTo(const IVersionBase& rhs) const override
	{
		const MajorMinorPatch_t* other = dynamic_cast<const MajorMinorPatch_t*>(&rhs);
		if (!other)
		{
			throw "Illegal comparsion of two different version formats.";
		}

		if (this->Major != other->Major) { return (this->Major > other->Major) ? 1 : -1; }
		if (this->Minor != other->Minor) { return (this->Minor > other->Minor) ? 1 : -1; }
		if (this->Patch != other->Patch) { return (this->Patch > other->Patch) ? 1 : -1; }

		return 0;
	}
};

#endif
