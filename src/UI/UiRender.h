///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  UiRender.h
/// Description  :  Contains the implementation for render callbacks.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <mutex>
#include <vector>

#include "Engine/Cleanup/RefCleanerBase.h"
#include "UiEnum.h"
#include "UiFuncDefs.h"

///----------------------------------------------------------------------------------------------------
/// CUiRender Class
///----------------------------------------------------------------------------------------------------
class CUiRender : public virtual IRefCleaner
{
	public:
	///----------------------------------------------------------------------------------------------------
	/// ctor
	///----------------------------------------------------------------------------------------------------
	CUiRender();

	///----------------------------------------------------------------------------------------------------
	/// dtor
	///----------------------------------------------------------------------------------------------------
	virtual ~CUiRender();

	///----------------------------------------------------------------------------------------------------
	/// Register:
	/// 	Registers the provided Render callback.
	///----------------------------------------------------------------------------------------------------
	void Register(ERenderType aRenderType, GUI_RENDER aRenderCallback);

	///----------------------------------------------------------------------------------------------------
	/// Deregister:
	/// 	Deregisters the provided Render callback.
	///----------------------------------------------------------------------------------------------------
	void Deregister(GUI_RENDER aRenderCallback);

	///----------------------------------------------------------------------------------------------------
	/// CleanupRefs:
	/// 	Removes all registered render callbacks and close-on-escape hooks that match the address space.
	///----------------------------------------------------------------------------------------------------
	int CleanupRefs(void* aStartAddress, void* aEndAddress) override;

	///----------------------------------------------------------------------------------------------------
	/// GetRenderCallbacks:
	/// 	Returns a copy of the specified callbacks.
	///----------------------------------------------------------------------------------------------------
	const std::vector<GUI_RENDER>& GetRenderCallbacks(ERenderType aRenderType) const;

	protected:
	std::mutex              RenderMutex;
	std::vector<GUI_RENDER> Registry[static_cast<uint32_t>(ERenderType::COUNT)];
};
