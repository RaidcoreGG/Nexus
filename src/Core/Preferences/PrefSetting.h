///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  PrefSetting.h
/// Description  :  Typed setting owned by the component that declares it.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <functional>
#include <mutex>
#include <string>

#include "PrefContext.h"

///----------------------------------------------------------------------------------------------------
/// CSetting Class
///----------------------------------------------------------------------------------------------------
template <typename T>
class CSetting : public ISetting
{
	public:
	using OnChange_t = std::function<void(const T&)>;

	CSetting(std::string aKey, T aDefault, OnChange_t aOnChange = nullptr)
		: Key(std::move(aKey))
		, Default(aDefault)
		, Value(std::move(aDefault))
		, OnChange(std::move(aOnChange))
	{
		CPreferences::Get()->Register(this);
	}

	~CSetting() override
	{
		CPreferences::Get()->Deregister(this);
	}

	CSetting(const CSetting&) = delete;
	CSetting& operator=(const CSetting&) = delete;

	///----------------------------------------------------------------------------------------------------
	/// Get:
	/// 	Returns the current value.
	///----------------------------------------------------------------------------------------------------
	T Get() const
	{
		const std::lock_guard<std::mutex> lock(this->Mutex);
		return this->Value;
	}

	///----------------------------------------------------------------------------------------------------
	/// GetDefault:
	/// 	Returns the default value.
	///----------------------------------------------------------------------------------------------------
	const T& GetDefault() const
	{
		return this->Default;
	}

	///----------------------------------------------------------------------------------------------------
	/// Set:
	/// 	Stores and persists a new value. Does nothing if the value is unchanged.
	///----------------------------------------------------------------------------------------------------
	void Set(T aValue)
	{
		{
			const std::lock_guard<std::mutex> lock(this->Mutex);

			if (this->Value == aValue)
			{
				return;
			}

			this->Value = aValue;
		}

		/* Outside the lock: the store locks before the setting when it applies loaded values. */
		CPreferences::Get()->Write(this->Key, aValue);

		this->NotifyChanged();
	}

	///----------------------------------------------------------------------------------------------------
	/// Reset:
	/// 	Sets the default value.
	///----------------------------------------------------------------------------------------------------
	void Reset()
	{
		this->Set(this->Default);
	}

	const std::string& GetKey() const override
	{
		return this->Key;
	}

	bool Apply(const json& aValue) override
	{
		T value;

		try
		{
			value = aValue.get<T>();
		}
		catch (const json::exception&)
		{
			LogWarning(CH_SETTINGS, "Setting \"%s\" has an invalid stored value, keeping the current one.", this->Key.c_str());
			return false;
		}

		const std::lock_guard<std::mutex> lock(this->Mutex);

		if (this->Value == value)
		{
			return false;
		}

		this->Value = std::move(value);
		return true;
	}

	void NotifyChanged() override
	{
		if (this->OnChange)
		{
			this->OnChange(this->Get());
		}
	}

	private:
	const std::string  Key;
	const T            Default;
	mutable std::mutex Mutex;
	T                  Value;
	OnChange_t         OnChange;
};
