///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  PrefSettingBase.h
/// Description  :  Interface of a setting registered with the settings store.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <string>

#pragma warning(push, 0)
#include "nlohmann/json.hpp"
#pragma warning(pop)
using json = nlohmann::json;

///----------------------------------------------------------------------------------------------------
/// ISetting Interface
///----------------------------------------------------------------------------------------------------
class ISetting
{
	public:
	virtual ~ISetting() = default;

	///----------------------------------------------------------------------------------------------------
	/// GetKey:
	/// 	Returns the persisted key.
	///----------------------------------------------------------------------------------------------------
	virtual const std::string& GetKey() const = 0;

	///----------------------------------------------------------------------------------------------------
	/// Apply:
	/// 	Takes a stored value without notifying. Returns true if the value changed.
	///----------------------------------------------------------------------------------------------------
	virtual bool Apply(const json& aValue) = 0;

	///----------------------------------------------------------------------------------------------------
	/// NotifyChanged:
	/// 	Calls the change handler with the current value.
	///----------------------------------------------------------------------------------------------------
	virtual void NotifyChanged() = 0;
};
