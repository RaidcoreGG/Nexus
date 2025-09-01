///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AddonProc.cpp
/// Description  :  Addon action queue/processor implementation.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Addon.h"

void CAddon::Load()
{
	if (!this->IsRunning) { return; }

	std::lock_guard<std::mutex> lock(this->ProcessorMutex);
	this->QueuedActions.push(EAddonAction::Load);
	this->ConVar.notify_one();
}

void CAddon::Unload()
{
	if (!this->IsRunning) { return; }

	std::lock_guard<std::mutex> lock(this->ProcessorMutex);
	this->QueuedActions.push(EAddonAction::Unload);
	this->ConVar.notify_one();
}

void CAddon::Uninstall()
{
	if (!this->IsRunning) { return; }

	std::lock_guard<std::mutex> lock(this->ProcessorMutex);
	this->QueuedActions.push(EAddonAction::Uninstall);
	this->ConVar.notify_one();
}

void CAddon::CheckForUpdate()
{
	if (!this->IsRunning) { return; }

	std::lock_guard<std::mutex> lock(this->ProcessorMutex);
	this->QueuedActions.push(EAddonAction::CheckUpdate);
	this->ConVar.notify_one();
}

void CAddon::Update()
{
	if (!this->IsRunning) { return; }

	std::lock_guard<std::mutex> lock(this->ProcessorMutex);
	this->QueuedActions.push(EAddonAction::Update);
	this->ConVar.notify_one();
}

void CAddon::ProcessActions()
{
	while (this->IsRunning)
	{
		EAddonAction action = EAddonAction::None;

		/* Scope, just to retrieve a queued action. */
		{
			std::unique_lock<std::mutex> lockThread(this->ProcessorMutex);
			this->ConVar.wait(lockThread, [this] { return this->QueuedActions.size() > 0; });

			if (!this->IsRunning) { return; }

			action = this->QueuedActions.front();
			this->QueuedActions.pop();
		}

		this->Flags |= EAddonFlags::RunningAction;

		switch (action)
		{
			case EAddonAction::Create:
			{
				this->Logger->Trace(CH_ADDON, "CAddon::Create(): %s", this->Location.string().c_str());
				this->EnumInterfaces();
				this->EventApi->Raise(0, EV_ADDON_CREATED);
				break;
			}
			case EAddonAction::Destroy:
			{
				this->Flags |= EAddonFlags::Destroying;
				this->Logger->Trace(CH_ADDON, "CAddon::Destroy(): %s", this->Location.string().c_str());
				this->UnloadInternal();
				this->IsRunning = false; /* Just to be sure. */
				this->EventApi->Raise(0, EV_ADDON_DESTROYED);
				return; /* Return the thread entirely. */
			}
			case EAddonAction::EnumInterfaces:
			{
				this->EnumInterfaces();
				break;
			}
			case EAddonAction::Load:
			{
				this->LoadInternal();
				break;
			}
			case EAddonAction::Unload:
			{
				this->UnloadInternal();
				break;
			}
			case EAddonAction::Uninstall:
			{
				this->UninstallInternal();
				break;
			}
			case EAddonAction::CheckUpdate:
			{
				this->CheckForUpdateInternal();
				break;
			}
			case EAddonAction::Update:
			{
				this->UpdateInternal();
				break;
			}
		}

		this->Flags &= ~EAddonFlags::RunningAction;
	}
}
