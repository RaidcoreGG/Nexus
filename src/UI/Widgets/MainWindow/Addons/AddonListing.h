///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AddonListing.h
/// Description  :  Addon listing UI wrapper.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

#include <string>
#include <unordered_map>

#include "Host/GW2/Addons/Addon.h"
#include "Host/GW2/Addons/Library/LibAddon.h"
#include "UI/UiBinds.h"
#include "UI/UiRender.h"
#include "Util/Strings.h"

namespace Raidcore::UI
{
	struct AddonListing_t
	{
		GW2::CAddon*                                            Addon;

		bool                                               HasLibDef;
		GW2::LibraryAddon_t                                     LibraryDef;
		bool                                               IsInstalling;

		std::unordered_map<std::string, UI::InputBindPacked_t> InputBinds;
		UI::GUI_RENDER                                         OptionsRender;

		bool                                               IsHovered;

		inline std::string GetUniqueID() const
		{
			/* Unique id. */
			std::string id;

			if (this->GetSig() != 0)
			{
				/* Use addon signature. */
				id = std::format("0x{:08X}", this->GetSig());
			}
			else
			{
				if (this->Addon)
				{
					/* Use addon file location. */
					id = this->Addon->GetLocation().string();
				}
				else
				{
					/* This should not be possible. */
					throw "Unreachable code.";
				}
			}

			return id;
		}

		inline uint32_t GetSig() const
		{
			if (this->Addon)
			{
				return this->Addon->GetSignature();
			}

			if (this->HasLibDef)
			{
				return this->LibraryDef.Signature;
			}

			return 0;
		}

		inline std::string GetName() const
		{
			if (this->Addon)
			{
				return this->Addon->GetName();
			}

			if (this->HasLibDef)
			{
				return this->LibraryDef.Name;
			}

			return "";
		}

		inline std::string GetAuthor() const
		{
			if (this->Addon)
			{
				return this->Addon->GetAuthor();
			}

			if (this->HasLibDef)
			{
				return this->LibraryDef.Author;
			}

			return "";
		}

		inline std::string GetDesc() const
		{
			if (this->Addon)
			{
				return this->Addon->GetDescription();
			}

			if (this->HasLibDef)
			{
				return this->LibraryDef.Description;
			}

			return "";
		}

		inline std::string GetVersion() const
		{
			if (this->Addon)
			{
				return this->Addon->GetVersion();
			}

			return "";
		}
	};
}
