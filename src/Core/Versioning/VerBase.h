///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  VerBase.h
/// Description  :  Interface for versioning formats.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef VERBASE_H
#define VERBASE_H

#include <string>

///----------------------------------------------------------------------------------------------------
/// IVersionBase Struct
///----------------------------------------------------------------------------------------------------
struct IVersionBase
{
	///----------------------------------------------------------------------------------------------------
	/// string:
	/// 	Converts the version to string.
	///----------------------------------------------------------------------------------------------------
	virtual std::string string() const = 0;

	///----------------------------------------------------------------------------------------------------
	/// CompareTo:
	/// 	Returns -1 (less), 0 (equal), 1 (greater).
	///----------------------------------------------------------------------------------------------------
	virtual int32_t CompareTo(const IVersionBase& rhs) const = 0;

	inline bool operator<(const IVersionBase& rhs) const
	{
		return this->CompareTo(rhs) < 0;
	}

	inline bool operator>(const IVersionBase& rhs) const
	{
		return this->CompareTo(rhs) > 0;
	}

	inline bool operator==(const IVersionBase& rhs) const
	{
		return this->CompareTo(rhs) == 0;
	}

	inline bool operator!=(const IVersionBase& rhs) const
	{
		return this->CompareTo(rhs) != 0;
	}

	inline bool operator<=(const IVersionBase& rhs) const
	{
		return this->CompareTo(rhs) <= 0;
	}

	inline bool operator>=(const IVersionBase& rhs) const
	{
		return this->CompareTo(rhs) >= 0;
	}
};

#endif
