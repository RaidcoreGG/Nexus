///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  UiInput.h
/// Description  :  Contains the input handling for the User Interface.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <atomic>
#include <cstdint>

#include "Core/Preferences/PrefContext.h"
#include "Engine/_Concepts/IWndProc.h"
#include "Util/Inputs.h"

///----------------------------------------------------------------------------------------------------
/// Raidcore::Nexus::GUI Namespace
///----------------------------------------------------------------------------------------------------
namespace Raidcore::Nexus::GUI
{
	///----------------------------------------------------------------------------------------------------
	/// CUiInput Class
	///----------------------------------------------------------------------------------------------------
	class CUiInput : public virtual IWndProc
	{
		public:
		enum class EInputType : uint8_t
		{
			Char,
			MouseWheel,
			MouseHWheel
		};

		struct InputEvent_t
		{
			EInputType Type;
			union
			{
				uint32_t Character;
				float WheelX;
				float WheelY;
			};
		};

		///----------------------------------------------------------------------------------------------------
		/// ctor
		///----------------------------------------------------------------------------------------------------
		CUiInput(Core::SettingsMgr* aSettings);

		///----------------------------------------------------------------------------------------------------
		/// dtor
		///----------------------------------------------------------------------------------------------------
		~CUiInput();

		///----------------------------------------------------------------------------------------------------
		/// WndProc:
		/// 	Returns 0 if message was processed.
		///----------------------------------------------------------------------------------------------------
		UINT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

		///----------------------------------------------------------------------------------------------------
		/// FlushInput:
		/// 	Flushes the input ring buffer to ImGui.
		///----------------------------------------------------------------------------------------------------
		void FlushInput();

		private:
		Core::SettingsMgr* Settings;

		EModifiers RequiredModifiers;
		bool       FilterClicks;

#define RING_SIZE 2048u
#define RING_MASK RING_SIZE - 1

		InputEvent_t Ring[RING_SIZE];

		std::atomic<uint32_t> WriteIndex{ 0 };
		std::atomic<uint32_t> ReadIndex{ 0 };

		///----------------------------------------------------------------------------------------------------
		/// PushInput:
		/// 	Pushes an input event to the ring buffer.
		///----------------------------------------------------------------------------------------------------
		bool PushInput(const InputEvent_t& aEvent);

		///----------------------------------------------------------------------------------------------------
		/// PopInput:
		/// 	Pops an input event from the ring buffer.
		///----------------------------------------------------------------------------------------------------
		bool PopInput(InputEvent_t& aEvent);
	};
}
