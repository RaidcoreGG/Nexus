///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  HkNotify.h
/// Description  :  Swap chain present and resize notifications.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <dxgi.h>

#include "Engine/Notifier/Notifier.h"

constexpr int32_t DXGIPRESENT_PRIO_TEXTURES = 200;
constexpr int32_t DXGIPRESENT_PRIO_UI       = 300;

constexpr int32_t DXGIRESIZE_PRIO_UI        = 100;

///----------------------------------------------------------------------------------------------------
/// IDxgiPresentNotifyHandler Interface
///----------------------------------------------------------------------------------------------------
class IDxgiPresentNotifyHandler : public INotifyHandler<IDxgiPresentNotifyHandler>
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// OnDxgiPresentNotify:
	/// 	Called every frame, before the game's present.
	///----------------------------------------------------------------------------------------------------
	virtual void OnDxgiPresentNotify(IDXGISwapChain* aSwapChain) = 0;
};

///----------------------------------------------------------------------------------------------------
/// IDxgiPresentNotifier Interface
///----------------------------------------------------------------------------------------------------
class IDxgiPresentNotifier : public INotifier<IDxgiPresentNotifyHandler>
{
	protected:
	void NotifyDxgiPresent(IDXGISwapChain* aSwapChain)
	{
		for (const auto& entry : this->Subscribers())
		{
			entry.Subscriber->OnDxgiPresentNotify(aSwapChain);
		}
	}
};

///----------------------------------------------------------------------------------------------------
/// IDxgiResizeNotifyHandler Interface
///----------------------------------------------------------------------------------------------------
class IDxgiResizeNotifyHandler : public INotifyHandler<IDxgiResizeNotifyHandler>
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// OnDxgiResizeNotify:
	/// 	Called before the swap chain buffers are resized.
	///----------------------------------------------------------------------------------------------------
	virtual void OnDxgiResizeNotify(IDXGISwapChain* aSwapChain, UINT aWidth, UINT aHeight) = 0;
};

///----------------------------------------------------------------------------------------------------
/// IDxgiResizeNotifier Interface
///----------------------------------------------------------------------------------------------------
class IDxgiResizeNotifier : public INotifier<IDxgiResizeNotifyHandler>
{
	protected:
	void NotifyDxgiResize(IDXGISwapChain* aSwapChain, UINT aWidth, UINT aHeight)
	{
		for (const auto& entry : this->Subscribers())
		{
			entry.Subscriber->OnDxgiResizeNotify(aSwapChain, aWidth, aHeight);
		}
	}
};
