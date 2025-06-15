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
	uint16_t Major;
	uint16_t Minor;
	uint16_t Build;
	uint16_t Revision;
};

///----------------------------------------------------------------------------------------------------
/// MajorMinorBuildRevision_t Struct
///----------------------------------------------------------------------------------------------------
struct MajorMinorBuildRevision_t : VerU64_4XS16_t, virtual IVersionBase
{
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

        if (this->Major    != other->Major)   { return (this->Major    > other->Major)    ? 1 : -1; }
        if (this->Minor    != other->Minor)   { return (this->Minor    > other->Minor)    ? 1 : -1; }
        if (this->Build    != other->Build)   { return (this->Build    > other->Build)    ? 1 : -1; }
        if (this->Revision != other->Revision){ return (this->Revision > other->Revision) ? 1 : -1; }

        return 0;
    }
};

#endif
