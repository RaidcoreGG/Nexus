///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  InstallRequestWindow.h
/// Description  :  Window listing addon installs requested through nexus:// links.
/// Authors      :  belst
///----------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <unordered_map>

#include "UI/Controls/CtlWindow.h"

namespace Raidcore::Nexus::GUI
{
	class CInstallRequestWindow : public virtual IWindow
	{
		public:
		void Render() override;

		private:
		// checkbox state
		std::unordered_map<uint32_t, bool> Selected;
	};
}
