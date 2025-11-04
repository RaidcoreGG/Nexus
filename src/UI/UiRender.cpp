///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  UiRender.cpp
/// Description  :  Contains the implementation for render callbacks.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "UiRender.h"

#include "Engine/Cleanup/RefCleanerContext.h"

CUiRender::CUiRender()
{
	CRefCleanerContext::Get()->Register("CUiRender", this);
}

CUiRender::~CUiRender()
{
}

void CUiRender::Register(ERenderType aRenderType, GUI_RENDER aRenderCallback)
{
	if (!aRenderCallback) { return; }

	const std::lock_guard<std::mutex> lock(this->RenderMutex);

	std::vector<GUI_RENDER>* targetRegistry{};

	switch (aRenderType)
	{
		case ERenderType::PreRender:
		{
			targetRegistry = &this->Registry[static_cast<uint32_t>(ERenderType::PreRender)];
			break;
		}
		case ERenderType::Render:
		{
			targetRegistry = &this->Registry[static_cast<uint32_t>(ERenderType::Render)];
			break;
		}
		case ERenderType::PostRender:
		{
			targetRegistry = &this->Registry[static_cast<uint32_t>(ERenderType::PostRender)];
			break;
		}
		case ERenderType::OptionsRender:
		{
			targetRegistry = &this->Registry[static_cast<uint32_t>(ERenderType::OptionsRender)];
			break;
		}
		default:
		{
			// TODO: This should be a macro for a "no default case".
			throw "No valid case for switch variable 'aRenderType'";
		}
	}

	/* Sanity check. */
	if (!targetRegistry)
	{
		return;
	}

	/* Only add if it doesn't already exist. */
	if (std::find(targetRegistry->begin(), targetRegistry->end(), aRenderCallback) != targetRegistry->end())
	{
		return;
	}

	targetRegistry->push_back(aRenderCallback);
}

void CUiRender::Deregister(GUI_RENDER aRenderCallback)
{
	if (!aRenderCallback) { return; }

	const std::lock_guard<std::mutex> lock(this->RenderMutex);

	for (size_t i = 0; i < static_cast<uint32_t>(ERenderType::COUNT); i++)
	{
		std::vector<GUI_RENDER>& registry = this->Registry[i];
		registry.erase(std::remove(registry.begin(), registry.end(), aRenderCallback), registry.end());
	}
}

int CUiRender::CleanupRefs(void* aStartAddress, void* aEndAddress)
{
	int refCounter = 0;

	const std::lock_guard<std::mutex> lock(this->RenderMutex);

	for (size_t i = 0; i < static_cast<uint32_t>(ERenderType::COUNT); i++)
	{
		for (GUI_RENDER renderCb : this->Registry[i])
		{
			if (renderCb >= aStartAddress && renderCb <= aEndAddress)
			{
				this->Registry[i].erase(std::remove(this->Registry[i].begin(), this->Registry[i].end(), renderCb), this->Registry[i].end());
				refCounter++;
			}
		}
	}

	return refCounter;
}

const std::vector<GUI_RENDER>& CUiRender::GetRenderCallbacks(ERenderType aRenderType) const
{
	switch (aRenderType)
	{
		case ERenderType::PreRender:
		{
			return this->Registry[static_cast<uint32_t>(ERenderType::PreRender)];
		}
		case ERenderType::Render:
		{
			return this->Registry[static_cast<uint32_t>(ERenderType::Render)];
		}
		case ERenderType::PostRender:
		{
			return this->Registry[static_cast<uint32_t>(ERenderType::PostRender)];
		}
		case ERenderType::OptionsRender:
		{
			return this->Registry[static_cast<uint32_t>(ERenderType::OptionsRender)];
		}
		default:
		{
			// TODO: This should be a macro for a "no default case".
			throw "No valid case for switch variable 'aRenderType'";
		}
	}
}
