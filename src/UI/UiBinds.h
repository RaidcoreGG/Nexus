///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  UiBinds.h
/// Description  :  Contains the implementation to display InputBinds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "Engine/Inputs/InputBinds/IbBind.h"
#include "Engine/Inputs/InputBinds/IbMapping.h"
#include "Host/GW2/Inputs/GameBinds/GbEnum.h"

namespace Raidcore::UI
{
	struct InputBindPacked_t
	{
		std::string        KeysText{};
		Engine::Input::IbMapping_t Bind    {};
	};

	struct InputBindCategory_t
	{
		std::string                                        Name      {};
		std::unordered_map<std::string, InputBindPacked_t> InputBinds{};
	};

	struct GameInputBindPacked_t
	{
		std::string        Name     {};
		std::string        KeysText {};
		Engine::Input::Bind_t Bind     {};

		std::string        KeysText2{};
		Engine::Input::Bind_t Bind2    {};
	};

	struct GameInputBindCategory_t
	{
		std::string                                           Name          {};
		std::unordered_map<GW2::EGameBinds, GameInputBindPacked_t> GameInputBinds{};
	};

	enum class EBindEditType
	{
		None,
		Nexus,
		Game,
		Game2
	};

	///----------------------------------------------------------------------------------------------------
	/// CUiBinds Class
	///----------------------------------------------------------------------------------------------------
	class CUiBinds
	{
		public:
		///----------------------------------------------------------------------------------------------------
		/// ctor
		///----------------------------------------------------------------------------------------------------
		CUiBinds();

		///----------------------------------------------------------------------------------------------------
		/// dtor
		///----------------------------------------------------------------------------------------------------
		virtual ~CUiBinds();

		///----------------------------------------------------------------------------------------------------
		/// GetInputBinds:
		/// 	Returns a copy of the display input binds.
		///----------------------------------------------------------------------------------------------------
		std::vector<InputBindCategory_t> GetInputBinds();

		///----------------------------------------------------------------------------------------------------
		/// GetInputBinds:
		/// 	Returns a copy of the display input binds.
		///----------------------------------------------------------------------------------------------------
		std::unordered_map<std::string, InputBindPacked_t> GetInputBinds(const std::string& aCategory);

		///----------------------------------------------------------------------------------------------------
		/// GetGameBinds:
		/// 	Returns a copy of the display game input binds.
		///----------------------------------------------------------------------------------------------------
		std::vector<GameInputBindCategory_t> GetGameBinds();

		protected:
		///----------------------------------------------------------------------------------------------------
		/// UpdateDisplayInputBinds:
		/// 	Refreshes the displayed input binds.
		///----------------------------------------------------------------------------------------------------
		void UpdateDisplayInputBinds();

		///----------------------------------------------------------------------------------------------------
		/// UpdateDisplayGameBinds:
		/// 	Refreshes the displayed game input binds.
		///----------------------------------------------------------------------------------------------------
		void UpdateDisplayGameBinds();

		private:
		mutable std::mutex                   DisplayBindsMutex{};
		std::vector<InputBindCategory_t>     DisplayInputBinds{};
		std::vector<GameInputBindCategory_t> DisplayGameBinds{};
	};
}
