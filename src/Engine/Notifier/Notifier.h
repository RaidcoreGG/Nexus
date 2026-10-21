///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Notifier.h
/// Description  :  Base classes for strongly typed engine notifications.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

///----------------------------------------------------------------------------------------------------
/// NOTIFY_CONSUMED:
/// 	Returned by a subscriber to stop propagation. Any other value passes the notification on.
///----------------------------------------------------------------------------------------------------
#define NOTIFY_CONSUMED 0

template<typename TNotify>
class INotifyHandler;

///----------------------------------------------------------------------------------------------------
/// NotifyHandlerRegistry Class
/// 	Attached subscribers of one notify interface, ordered by priority.
///----------------------------------------------------------------------------------------------------
template<typename TNotify>
class NotifyHandlerRegistry
{
	public:
	struct Entry
	{
		INotifyHandler<TNotify>* Base;
		TNotify*                 Subscriber;
		int32_t                  Priority;
	};

	///----------------------------------------------------------------------------------------------------
	/// View Class
	/// 	Holds the lock while iterating, so detaching on another thread waits for the notification.
	/// 	The lock is recursive and a snapshot is iterated, so subscribers may (de)attach from within.
	///----------------------------------------------------------------------------------------------------
	class View
	{
		public:
		View(NotifyHandlerRegistry& aRegistry)
			: Lock(aRegistry.Mutex)
			, Entries(aRegistry.Entries)
		{}

		auto begin() const { return this->Entries->begin(); }
		auto end() const { return this->Entries->end(); }

		private:
		std::unique_lock<std::recursive_mutex>    Lock;
		std::shared_ptr<const std::vector<Entry>> Entries;
	};

	static NotifyHandlerRegistry& Get()
	{
		static NotifyHandlerRegistry s_Registry{};
		return s_Registry;
	}

	void Add(INotifyHandler<TNotify>* aBase, TNotify* aSubscriber, int32_t aPriority)
	{
		const std::lock_guard<std::recursive_mutex> lock(this->Mutex);

		std::shared_ptr<std::vector<Entry>> entries = std::make_shared<std::vector<Entry>>(*this->Entries);

		auto it = std::upper_bound(entries->begin(), entries->end(), aPriority, [](int32_t aPrio, const Entry& aEntry)
		{
			return aPrio < aEntry.Priority;
		});

		entries->insert(it, Entry{ aBase, aSubscriber, aPriority });
		this->Entries = entries;
	}

	void Remove(INotifyHandler<TNotify>* aBase)
	{
		const std::lock_guard<std::recursive_mutex> lock(this->Mutex);

		std::shared_ptr<std::vector<Entry>> entries = std::make_shared<std::vector<Entry>>(*this->Entries);

		std::erase_if(*entries, [aBase](const Entry& aEntry)
		{
			return aEntry.Base == aBase;
		});

		this->Entries = entries;
	}

	bool Contains(INotifyHandler<TNotify>* aBase)
	{
		const std::lock_guard<std::recursive_mutex> lock(this->Mutex);

		return std::any_of(this->Entries->begin(), this->Entries->end(), [aBase](const Entry& aEntry)
		{
			return aEntry.Base == aBase;
		});
	}

	private:
	std::recursive_mutex                      Mutex;
	std::shared_ptr<const std::vector<Entry>> Entries = std::make_shared<const std::vector<Entry>>();
};

///----------------------------------------------------------------------------------------------------
/// INotifyHandler Class
/// 	Subscriber base. The subscriber decides when it receives notifications.
///----------------------------------------------------------------------------------------------------
template<typename TNotify>
class INotifyHandler
{
	protected:
	INotifyHandler() = default;

	~INotifyHandler()
	{
		assert(!NotifyHandlerRegistry<TNotify>::Get().Contains(this) && "Detach before destruction.");
	}

	///----------------------------------------------------------------------------------------------------
	/// Attach:
	/// 	Starts receiving notifications. Call once the subscriber is fully constructed.
	///----------------------------------------------------------------------------------------------------
	void Attach(int32_t aPriority = 0)
	{
		NotifyHandlerRegistry<TNotify>::Get().Add(this, static_cast<TNotify*>(this), aPriority);
	}

	///----------------------------------------------------------------------------------------------------
	/// Detach:
	/// 	Stops receiving notifications. Waits for notifications running on other threads.
	///----------------------------------------------------------------------------------------------------
	void Detach()
	{
		NotifyHandlerRegistry<TNotify>::Get().Remove(this);
	}
};

///----------------------------------------------------------------------------------------------------
/// INotifier Class
/// 	Source base.
///----------------------------------------------------------------------------------------------------
template<typename TNotify>
class INotifier
{
	protected:
	///----------------------------------------------------------------------------------------------------
	/// Subscribers:
	/// 	Attached subscribers in priority order. Keep the view only for the duration of one notification.
	///----------------------------------------------------------------------------------------------------
	typename NotifyHandlerRegistry<TNotify>::View Subscribers() const
	{
		return typename NotifyHandlerRegistry<TNotify>::View(NotifyHandlerRegistry<TNotify>::Get());
	}
};
