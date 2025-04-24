///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  GbConst.cpp
/// Description  :  Constant data for game binds.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "GbConst.h"

#include <unordered_map>

std::string& NameFrom(EGameBinds aGameBind)
{
	static std::unordered_map<EGameBinds, std::string> s_GbNameLUT =
	{
		// Movement
		{ EGameBinds::MoveForward, "((MoveForward))" },
		{ EGameBinds::MoveBackward, "((MoveBackward))" },
		{ EGameBinds::MoveLeft, "((MoveLeft))" },
		{ EGameBinds::MoveRight, "((MoveRight))" },
		{ EGameBinds::MoveTurnLeft, "((MoveTurnLeft))" },
		{ EGameBinds::MoveTurnRight, "((MoveTurnRight))" },
		{ EGameBinds::MoveDodge, "((MoveDodge))" },
		{ EGameBinds::MoveAutoRun, "((MoveAutoRun))" },
		{ EGameBinds::MoveWalk, "((MoveWalk))" },
		{ EGameBinds::MoveJump, "((MoveJump))"},
		{ EGameBinds::MoveSwimUp, "((MoveSwimUp))" },
		{ EGameBinds::MoveSwimDown, "((MoveSwimDown))" },
		{ EGameBinds::MoveAboutFace, "((MoveAboutFace))" },

		// Skills
		{ EGameBinds::SkillWeaponSwap, "((SkillWeaponSwap))" },
		{ EGameBinds::SkillWeapon1, "((SkillWeapon1))" },
		{ EGameBinds::SkillWeapon2, "((SkillWeapon2))" },
		{ EGameBinds::SkillWeapon3, "((SkillWeapon3))" },
		{ EGameBinds::SkillWeapon4, "((SkillWeapon4))" },
		{ EGameBinds::SkillWeapon5, "((SkillWeapon5))" },
		{ EGameBinds::SkillHeal, "((SkillHeal))" },
		{ EGameBinds::SkillUtility1, "((SkillUtility1))" },
		{ EGameBinds::SkillUtility2, "((SkillUtility2))" },
		{ EGameBinds::SkillUtility3, "((SkillUtility3))" },
		{ EGameBinds::SkillElite, "((SkillElite))" },
		{ EGameBinds::SkillProfession1, "((SkillProfession1))" },
		{ EGameBinds::SkillProfession2, "((SkillProfession2))" },
		{ EGameBinds::SkillProfession3, "((SkillProfession3))" },
		{ EGameBinds::SkillProfession4, "((SkillProfession4))" },
		{ EGameBinds::SkillProfession5, "((SkillProfession5))" },
		{ EGameBinds::SkillProfession6, "((SkillProfession6))" },
		{ EGameBinds::SkillProfession7, "((SkillProfession7))" },
		{ EGameBinds::SkillSpecialAction, "((SkillSpecialAction))" },

		// Targeting
		{ EGameBinds::TargetAlert, "((TargetAlert))" },
		{ EGameBinds::TargetCall, "((TargetCall))" },
		{ EGameBinds::TargetTake, "((TargetTake))" },
		{ EGameBinds::TargetCallLocal, "((TargetCallLocal))" },
		{ EGameBinds::TargetTakeLocal, "((TargetTakeLocal))" },
		{ EGameBinds::TargetEnemyNearest, "((TargetEnemyNearest))" },
		{ EGameBinds::TargetEnemyNext, "((TargetEnemyNext))" },
		{ EGameBinds::TargetEnemyPrev, "((TargetEnemyPrev))" },
		{ EGameBinds::TargetAllyNearest, "((TargetAllyNearest))" },
		{ EGameBinds::TargetAllyNext, "((TargetAllyNext))" },
		{ EGameBinds::TargetAllyPrev, "((TargetAllyPrev))" },
		{ EGameBinds::TargetLock, "((TargetLock))" },
		{ EGameBinds::TargetSnapGroundTarget, "((TargetSnapGroundTarget))" },
		{ EGameBinds::TargetSnapGroundTargetToggle, "((TargetSnapGroundTargetToggle))" },
		{ EGameBinds::TargetAutoTargetingDisable, "((TargetAutoTargetingDisable))" },
		{ EGameBinds::TargetAutoTargetingToggle, "((TargetAutoTargetingToggle))" },
		{ EGameBinds::TargetAllyTargetingMode, "((TargetAllyTargetingMode))" },
		{ EGameBinds::TargetAllyTargetingModeToggle, "((TargetAllyTargetingModeToggle))" },

		// UI Binds
		{ EGameBinds::UiCommerce, "((UiCommerce))" }, // TradingPost
		{ EGameBinds::UiContacts, "((UiContacts))" },
		{ EGameBinds::UiGuild, "((UiGuild))" },
		{ EGameBinds::UiHero, "((UiHero))" },
		{ EGameBinds::UiInventory, "((UiInventory))" },
		{ EGameBinds::UiKennel, "((UiKennel))" }, // Pets
		{ EGameBinds::UiLogout, "((UiLogout))" },
		{ EGameBinds::UiMail, "((UiMail))" },
		{ EGameBinds::UiOptions, "((UiOptions))" },
		{ EGameBinds::UiParty, "((UiParty))" },
		{ EGameBinds::UiPvp, "((UiPvp))" },
		{ EGameBinds::UiPvpBuild, "((UiPvpBuild))" },
		{ EGameBinds::UiScoreboard, "((UiScoreboard))" },
		{ EGameBinds::UiSeasonalObjectivesShop, "((UiSeasonalObjectivesShop))" }, // Wizard's Vault
		{ EGameBinds::UiInformation, "((UiInformation))" },
		{ EGameBinds::UiChatToggle, "((UiChatToggle))" },
		{ EGameBinds::UiChatCommand, "((UiChatCommand))" },
		{ EGameBinds::UiChatFocus, "((UiChatFocus))" },
		{ EGameBinds::UiChatReply, "((UiChatReply))" },
		{ EGameBinds::UiToggle, "((UiToggle))" },
		{ EGameBinds::UiSquadBroadcastChatToggle, "((UiSquadBroadcastChatToggle))" },
		{ EGameBinds::UiSquadBroadcastChatCommand, "((UiSquadBroadcastChatCommand))" },
		{ EGameBinds::UiSquadBroadcastChatFocus, "((UiSquadBroadcastChatFocus))" },

		// Camera
		{ EGameBinds::CameraFree, "((CameraFree))" },
		{ EGameBinds::CameraZoomIn, "((CameraZoomIn))" },
		{ EGameBinds::CameraZoomOut, "((CameraZoomOut))" },
		{ EGameBinds::CameraReverse, "((CameraReverse))" },
		{ EGameBinds::CameraActionMode, "((CameraActionMode))" },
		{ EGameBinds::CameraActionModeDisable, "((CameraActionModeDisable))" },

		// Screenshots
		{ EGameBinds::ScreenshotNormal, "((ScreenshotNormal))" },
		{ EGameBinds::ScreenshotStereoscopic, "((ScreenshotStereoscopic))" },

		// Map
		{ EGameBinds::MapToggle, "((MapToggle))" },
		{ EGameBinds::MapFocusPlayer, "((MapFocusPlayer))" },
		{ EGameBinds::MapFloorDown, "((MapFloorDown))" },
		{ EGameBinds::MapFloorUp, "((MapFloorUp))" },
		{ EGameBinds::MapZoomIn, "((MapZoomIn))" },
		{ EGameBinds::MapZoomOut, "((MapZoomOut))" },

		// Mounts
		{ EGameBinds::SpumoniToggle, "((SpumoniToggle))" },
		{ EGameBinds::SpumoniMovement, "((SpumoniMovement))" },
		{ EGameBinds::SpumoniSecondaryMovement, "((SpumoniSecondaryMovement))" },
		{ EGameBinds::SpumoniMAM01, "((SpumoniMAM01))" }, // Raptor
		{ EGameBinds::SpumoniMAM02, "((SpumoniMAM02))" }, // Springer
		{ EGameBinds::SpumoniMAM03, "((SpumoniMAM03))" }, // Skimmer
		{ EGameBinds::SpumoniMAM04, "((SpumoniMAM04))" }, // Jackal
		{ EGameBinds::SpumoniMAM05, "((SpumoniMAM05))" }, // Griffon
		{ EGameBinds::SpumoniMAM06, "((SpumoniMAM06))" }, // RollerBeetle
		{ EGameBinds::SpumoniMAM07, "((SpumoniMAM07))" }, // Warclaw
		{ EGameBinds::SpumoniMAM08, "((SpumoniMAM08))" }, // Skyscale
		{ EGameBinds::SpumoniMAM09, "((SpumoniMAM09))" }, // SiegeTurtle

		// Spectator Binds
		{ EGameBinds::SpectatorNearestFixed, "((SpectatorNearestFixed))" },
		{ EGameBinds::SpectatorNearestPlayer, "((SpectatorNearestPlayer))" },
		{ EGameBinds::SpectatorPlayerRed1, "((SpectatorPlayerRed1))" },
		{ EGameBinds::SpectatorPlayerRed2, "((SpectatorPlayerRed2))" },
		{ EGameBinds::SpectatorPlayerRed3, "((SpectatorPlayerRed3))" },
		{ EGameBinds::SpectatorPlayerRed4, "((SpectatorPlayerRed4))" },
		{ EGameBinds::SpectatorPlayerRed5, "((SpectatorPlayerRed5))" },
		{ EGameBinds::SpectatorPlayerBlue1, "((SpectatorPlayerBlue1))" },
		{ EGameBinds::SpectatorPlayerBlue2, "((SpectatorPlayerBlue2))" },
		{ EGameBinds::SpectatorPlayerBlue3, "((SpectatorPlayerBlue3))" },
		{ EGameBinds::SpectatorPlayerBlue4, "((SpectatorPlayerBlue4))" },
		{ EGameBinds::SpectatorPlayerBlue5, "((SpectatorPlayerBlue5))" },
		{ EGameBinds::SpectatorFreeCamera, "((SpectatorFreeCamera))" },
		{ EGameBinds::SpectatorFreeCameraMode, "((SpectatorFreeCameraMode))" },
		{ EGameBinds::SpectatorFreeMoveForward, "((SpectatorFreeMoveForward))" },
		{ EGameBinds::SpectatorFreeMoveBackward, "((SpectatorFreeMoveBackward))" },
		{ EGameBinds::SpectatorFreeMoveLeft, "((SpectatorFreeMoveLeft))" },
		{ EGameBinds::SpectatorFreeMoveRight, "((SpectatorFreeMoveRight))" },
		{ EGameBinds::SpectatorFreeMoveUp, "((SpectatorFreeMoveUp))" },
		{ EGameBinds::SpectatorFreeMoveDown, "((SpectatorFreeMoveDown))" },

		// Squad Markers
		{ EGameBinds::SquadMarkerPlaceWorld1, "((SquadMarkerPlaceWorld1))" }, // Arrow
		{ EGameBinds::SquadMarkerPlaceWorld2, "((SquadMarkerPlaceWorld2))" }, // Circle
		{ EGameBinds::SquadMarkerPlaceWorld3, "((SquadMarkerPlaceWorld3))" }, // Heart
		{ EGameBinds::SquadMarkerPlaceWorld4, "((SquadMarkerPlaceWorld4))" }, // Square
		{ EGameBinds::SquadMarkerPlaceWorld5, "((SquadMarkerPlaceWorld5))" }, // Star
		{ EGameBinds::SquadMarkerPlaceWorld6, "((SquadMarkerPlaceWorld6))" }, // Swirl
		{ EGameBinds::SquadMarkerPlaceWorld7, "((SquadMarkerPlaceWorld7))" }, // Triangle
		{ EGameBinds::SquadMarkerPlaceWorld8, "((SquadMarkerPlaceWorld8))" }, // Cross
		{ EGameBinds::SquadMarkerClearAllWorld, "((SquadMarkerClearAllWorld))" },
		{ EGameBinds::SquadMarkerSetAgent1, "((SquadMarkerSetAgent1))" }, // Arrow
		{ EGameBinds::SquadMarkerSetAgent2, "((SquadMarkerSetAgent2))" }, // Circle
		{ EGameBinds::SquadMarkerSetAgent3, "((SquadMarkerSetAgent3))" }, // Heart
		{ EGameBinds::SquadMarkerSetAgent4, "((SquadMarkerSetAgent4))" }, // Square
		{ EGameBinds::SquadMarkerSetAgent5, "((SquadMarkerSetAgent5))" }, // Star
		{ EGameBinds::SquadMarkerSetAgent6, "((SquadMarkerSetAgent6))" }, // Swirl
		{ EGameBinds::SquadMarkerSetAgent7, "((SquadMarkerSetAgent7))" }, // Triangle
		{ EGameBinds::SquadMarkerSetAgent8, "((SquadMarkerSetAgent8))" }, // Cross
		{ EGameBinds::SquadMarkerClearAllAgent, "((SquadMarkerClearAllAgent))" },

		// Mastery Skills
		{ EGameBinds::MasteryAccess, "((MasteryAccess))" },
		{ EGameBinds::MasteryAccess01, "((MasteryAccess01))" }, // Fishing
		{ EGameBinds::MasteryAccess02, "((MasteryAccess02))" }, // Skiff
		{ EGameBinds::MasteryAccess03, "((MasteryAccess03))" }, // Jade Bot Waypoint
		{ EGameBinds::MasteryAccess04, "((MasteryAccess04))" }, // Rift Scan
		{ EGameBinds::MasteryAccess05, "((MasteryAccess05))" }, // Skyscale
		{ EGameBinds::MasteryAccess06, "((MasteryAccess06))" }, // // Homestead Doorway

		// Miscellaneous Binds
		{ EGameBinds::MiscAoELoot, "((MiscAoELoot))" },
		{ EGameBinds::MiscInteract, "((MiscInteract))" },
		{ EGameBinds::MiscShowEnemies, "((MiscShowEnemies))" },
		{ EGameBinds::MiscShowAllies, "((MiscShowAllies))" },
		{ EGameBinds::MiscCombatStance, "((MiscCombatStance))" }, // Stow/Draw
		{ EGameBinds::MiscToggleLanguage, "((MiscToggleLanguage))" },
		{ EGameBinds::MiscTogglePetCombat, "((MiscTogglePetCombat))" },
		{ EGameBinds::MiscToggleFullScreen, "((MiscToggleFullScreen))" },
		{ EGameBinds::MiscToggleDecorationMode, "((MiscToggleDecorationMode))" }, // Decoration Mode

		// Toys/Novelties
		{ EGameBinds::ToyUseDefault, "((ToyUseDefault))" },
		{ EGameBinds::ToyUseSlot1, "((ToyUseSlot1))" }, // Chair
		{ EGameBinds::ToyUseSlot2, "((ToyUseSlot2))" }, // Instrument
		{ EGameBinds::ToyUseSlot3, "((ToyUseSlot3))" }, // Held Item
		{ EGameBinds::ToyUseSlot4, "((ToyUseSlot4))" }, // Toy
		{ EGameBinds::ToyUseSlot5, "((ToyUseSlot5))" }, // Tonic
		//ToyUseSlot6 unused

		// Build Templates
		{ EGameBinds::Loadout1, "((Loadout1))" },
		{ EGameBinds::Loadout2, "((Loadout2))" },
		{ EGameBinds::Loadout3, "((Loadout3))" },
		{ EGameBinds::Loadout4, "((Loadout4))" },
		{ EGameBinds::Loadout5, "((Loadout5))" },
		{ EGameBinds::Loadout6, "((Loadout6))" },
		{ EGameBinds::Loadout7, "((Loadout7))" },
		{ EGameBinds::Loadout8, "((Loadout8))" },
		{ EGameBinds::Loadout9, "((Loadout9))" },

		// Equipment Templates
		{ EGameBinds::GearLoadout1, "((GearLoadout1))" },
		{ EGameBinds::GearLoadout2, "((GearLoadout2))" },
		{ EGameBinds::GearLoadout3, "((GearLoadout3))" },
		{ EGameBinds::GearLoadout4, "((GearLoadout4))" },
		{ EGameBinds::GearLoadout5, "((GearLoadout5))" },
		{ EGameBinds::GearLoadout6, "((GearLoadout6))" },
		{ EGameBinds::GearLoadout7, "((GearLoadout7))" },
		{ EGameBinds::GearLoadout8, "((GearLoadout8))" },
		{ EGameBinds::GearLoadout9, "((GearLoadout9))" }
	};

	auto it = s_GbNameLUT.find(aGameBind);

	if (it != s_GbNameLUT.end())
	{
		return it->second;
	}

	static std::string s_Nullstr = "";

	return s_Nullstr;
}

std::string& CategoryNameFrom(EGameBinds aGameBind)
{
	static constexpr const char* GKBCAT_MOVEMENT   = "((Movement))";
	static constexpr const char* GKBCAT_SKILLS     = "((Skills))";
	static constexpr const char* GKBCAT_TARGETING  = "((Targeting))";
	static constexpr const char* GKBCAT_UI         = "((User Interface))";
	static constexpr const char* GKBCAT_CAMERA     = "((Camera))";
	static constexpr const char* GKBCAT_SCREENSHOT = "((Screenshot))";
	static constexpr const char* GKBCAT_MAP        = "((Map))";
	static constexpr const char* GKBCAT_MOUNTS     = "((Mounts))";
	static constexpr const char* GKBCAT_SPECTATE   = "((Spectators))";
	static constexpr const char* GKBCAT_SQUAD      = "((Squad))";
	static constexpr const char* GKBCAT_MASTERY    = "((Mastery Skills))";
	static constexpr const char* GKBCAT_MISC       = "((Miscellaneous))";
	static constexpr const char* GKBCAT_TEMPLATES  = "((Templates))";

	static std::unordered_map<EGameBinds, std::string> s_GbCategoryNameLUT =
	{
		// Movement
		{ EGameBinds::MoveForward, GKBCAT_MOVEMENT },
		{ EGameBinds::MoveBackward, GKBCAT_MOVEMENT },
		{ EGameBinds::MoveLeft, GKBCAT_MOVEMENT },
		{ EGameBinds::MoveRight, GKBCAT_MOVEMENT },
		{ EGameBinds::MoveTurnLeft, GKBCAT_MOVEMENT },
		{ EGameBinds::MoveTurnRight, GKBCAT_MOVEMENT },
		{ EGameBinds::MoveDodge, GKBCAT_MOVEMENT },
		{ EGameBinds::MoveAutoRun, GKBCAT_MOVEMENT },
		{ EGameBinds::MoveWalk, GKBCAT_MOVEMENT },
		{ EGameBinds::MoveJump, GKBCAT_MOVEMENT},
		{ EGameBinds::MoveSwimUp, GKBCAT_MOVEMENT },
		{ EGameBinds::MoveSwimDown, GKBCAT_MOVEMENT },
		{ EGameBinds::MoveAboutFace, GKBCAT_MOVEMENT },

		// Skills
		{ EGameBinds::SkillWeaponSwap, GKBCAT_SKILLS },
		{ EGameBinds::SkillWeapon1, GKBCAT_SKILLS },
		{ EGameBinds::SkillWeapon2, GKBCAT_SKILLS },
		{ EGameBinds::SkillWeapon3, GKBCAT_SKILLS },
		{ EGameBinds::SkillWeapon4, GKBCAT_SKILLS },
		{ EGameBinds::SkillWeapon5, GKBCAT_SKILLS },
		{ EGameBinds::SkillHeal, GKBCAT_SKILLS },
		{ EGameBinds::SkillUtility1, GKBCAT_SKILLS },
		{ EGameBinds::SkillUtility2, GKBCAT_SKILLS },
		{ EGameBinds::SkillUtility3, GKBCAT_SKILLS },
		{ EGameBinds::SkillElite, GKBCAT_SKILLS },
		{ EGameBinds::SkillProfession1, GKBCAT_SKILLS },
		{ EGameBinds::SkillProfession2, GKBCAT_SKILLS },
		{ EGameBinds::SkillProfession3, GKBCAT_SKILLS },
		{ EGameBinds::SkillProfession4, GKBCAT_SKILLS },
		{ EGameBinds::SkillProfession5, GKBCAT_SKILLS },
		{ EGameBinds::SkillProfession6, GKBCAT_SKILLS },
		{ EGameBinds::SkillProfession7, GKBCAT_SKILLS },
		{ EGameBinds::SkillSpecialAction, GKBCAT_SKILLS },

		// Targeting
		{ EGameBinds::TargetAlert, GKBCAT_TARGETING },
		{ EGameBinds::TargetCall, GKBCAT_TARGETING },
		{ EGameBinds::TargetTake, GKBCAT_TARGETING },
		{ EGameBinds::TargetCallLocal, GKBCAT_TARGETING },
		{ EGameBinds::TargetTakeLocal, GKBCAT_TARGETING },
		{ EGameBinds::TargetEnemyNearest, GKBCAT_TARGETING },
		{ EGameBinds::TargetEnemyNext, GKBCAT_TARGETING },
		{ EGameBinds::TargetEnemyPrev, GKBCAT_TARGETING },
		{ EGameBinds::TargetAllyNearest, GKBCAT_TARGETING },
		{ EGameBinds::TargetAllyNext, GKBCAT_TARGETING },
		{ EGameBinds::TargetAllyPrev, GKBCAT_TARGETING },
		{ EGameBinds::TargetLock, GKBCAT_TARGETING },
		{ EGameBinds::TargetSnapGroundTarget, GKBCAT_TARGETING },
		{ EGameBinds::TargetSnapGroundTargetToggle, GKBCAT_TARGETING },
		{ EGameBinds::TargetAutoTargetingDisable, GKBCAT_TARGETING },
		{ EGameBinds::TargetAutoTargetingToggle, GKBCAT_TARGETING },
		{ EGameBinds::TargetAllyTargetingMode, GKBCAT_TARGETING },
		{ EGameBinds::TargetAllyTargetingModeToggle, GKBCAT_TARGETING },

		// UI Binds
		{ EGameBinds::UiCommerce, GKBCAT_UI }, // TradingPost
		{ EGameBinds::UiContacts, GKBCAT_UI },
		{ EGameBinds::UiGuild, GKBCAT_UI },
		{ EGameBinds::UiHero, GKBCAT_UI },
		{ EGameBinds::UiInventory, GKBCAT_UI },
		{ EGameBinds::UiKennel, GKBCAT_UI }, // Pets
		{ EGameBinds::UiLogout, GKBCAT_UI },
		{ EGameBinds::UiMail, GKBCAT_UI },
		{ EGameBinds::UiOptions, GKBCAT_UI },
		{ EGameBinds::UiParty, GKBCAT_UI },
		{ EGameBinds::UiPvp, GKBCAT_UI },
		{ EGameBinds::UiPvpBuild, GKBCAT_UI },
		{ EGameBinds::UiScoreboard, GKBCAT_UI },
		{ EGameBinds::UiSeasonalObjectivesShop, GKBCAT_UI }, // Wizard's Vault
		{ EGameBinds::UiInformation, GKBCAT_UI },
		{ EGameBinds::UiChatToggle, GKBCAT_UI },
		{ EGameBinds::UiChatCommand, GKBCAT_UI },
		{ EGameBinds::UiChatFocus, GKBCAT_UI },
		{ EGameBinds::UiChatReply, GKBCAT_UI },
		{ EGameBinds::UiToggle, GKBCAT_UI },
		{ EGameBinds::UiSquadBroadcastChatToggle, GKBCAT_UI },
		{ EGameBinds::UiSquadBroadcastChatCommand, GKBCAT_UI },
		{ EGameBinds::UiSquadBroadcastChatFocus, GKBCAT_UI },

		// Camera
		{ EGameBinds::CameraFree, GKBCAT_CAMERA },
		{ EGameBinds::CameraZoomIn, GKBCAT_CAMERA },
		{ EGameBinds::CameraZoomOut, GKBCAT_CAMERA },
		{ EGameBinds::CameraReverse, GKBCAT_CAMERA },
		{ EGameBinds::CameraActionMode, GKBCAT_CAMERA },
		{ EGameBinds::CameraActionModeDisable, GKBCAT_CAMERA },

		// Screenshots
		{ EGameBinds::ScreenshotNormal, GKBCAT_SCREENSHOT },
		{ EGameBinds::ScreenshotStereoscopic, GKBCAT_SCREENSHOT },

		// Map
		{ EGameBinds::MapToggle, GKBCAT_MAP },
		{ EGameBinds::MapFocusPlayer, GKBCAT_MAP },
		{ EGameBinds::MapFloorDown, GKBCAT_MAP },
		{ EGameBinds::MapFloorUp, GKBCAT_MAP },
		{ EGameBinds::MapZoomIn, GKBCAT_MAP },
		{ EGameBinds::MapZoomOut, GKBCAT_MAP },

		// Mounts
		{ EGameBinds::SpumoniToggle, GKBCAT_MOUNTS },
		{ EGameBinds::SpumoniMovement, GKBCAT_MOUNTS },
		{ EGameBinds::SpumoniSecondaryMovement, GKBCAT_MOUNTS },
		{ EGameBinds::SpumoniMAM01, GKBCAT_MOUNTS }, // Raptor
		{ EGameBinds::SpumoniMAM02, GKBCAT_MOUNTS }, // Springer
		{ EGameBinds::SpumoniMAM03, GKBCAT_MOUNTS }, // Skimmer
		{ EGameBinds::SpumoniMAM04, GKBCAT_MOUNTS }, // Jackal
		{ EGameBinds::SpumoniMAM05, GKBCAT_MOUNTS }, // Griffon
		{ EGameBinds::SpumoniMAM06, GKBCAT_MOUNTS }, // RollerBeetle
		{ EGameBinds::SpumoniMAM07, GKBCAT_MOUNTS }, // Warclaw
		{ EGameBinds::SpumoniMAM08, GKBCAT_MOUNTS }, // Skyscale
		{ EGameBinds::SpumoniMAM09, GKBCAT_MOUNTS }, // SiegeTurtle

		// Spectator Binds
		{ EGameBinds::SpectatorNearestFixed, GKBCAT_SPECTATE },
		{ EGameBinds::SpectatorNearestPlayer, GKBCAT_SPECTATE },
		{ EGameBinds::SpectatorPlayerRed1, GKBCAT_SPECTATE },
		{ EGameBinds::SpectatorPlayerRed2, GKBCAT_SPECTATE },
		{ EGameBinds::SpectatorPlayerRed3, GKBCAT_SPECTATE },
		{ EGameBinds::SpectatorPlayerRed4, GKBCAT_SPECTATE },
		{ EGameBinds::SpectatorPlayerRed5, GKBCAT_SPECTATE },
		{ EGameBinds::SpectatorPlayerBlue1, GKBCAT_SPECTATE },
		{ EGameBinds::SpectatorPlayerBlue2, GKBCAT_SPECTATE },
		{ EGameBinds::SpectatorPlayerBlue3, GKBCAT_SPECTATE },
		{ EGameBinds::SpectatorPlayerBlue4, GKBCAT_SPECTATE },
		{ EGameBinds::SpectatorPlayerBlue5, GKBCAT_SPECTATE },
		{ EGameBinds::SpectatorFreeCamera, GKBCAT_SPECTATE },
		{ EGameBinds::SpectatorFreeCameraMode, GKBCAT_SPECTATE },
		{ EGameBinds::SpectatorFreeMoveForward, GKBCAT_SPECTATE },
		{ EGameBinds::SpectatorFreeMoveBackward, GKBCAT_SPECTATE },
		{ EGameBinds::SpectatorFreeMoveLeft, GKBCAT_SPECTATE },
		{ EGameBinds::SpectatorFreeMoveRight, GKBCAT_SPECTATE },
		{ EGameBinds::SpectatorFreeMoveUp, GKBCAT_SPECTATE },
		{ EGameBinds::SpectatorFreeMoveDown, GKBCAT_SPECTATE },

		// Squad Markers
		{ EGameBinds::SquadMarkerPlaceWorld1, GKBCAT_SQUAD }, // Arrow
		{ EGameBinds::SquadMarkerPlaceWorld2, GKBCAT_SQUAD }, // Circle
		{ EGameBinds::SquadMarkerPlaceWorld3, GKBCAT_SQUAD }, // Heart
		{ EGameBinds::SquadMarkerPlaceWorld4, GKBCAT_SQUAD }, // Square
		{ EGameBinds::SquadMarkerPlaceWorld5, GKBCAT_SQUAD }, // Star
		{ EGameBinds::SquadMarkerPlaceWorld6, GKBCAT_SQUAD }, // Swirl
		{ EGameBinds::SquadMarkerPlaceWorld7, GKBCAT_SQUAD }, // Triangle
		{ EGameBinds::SquadMarkerPlaceWorld8, GKBCAT_SQUAD }, // Cross
		{ EGameBinds::SquadMarkerClearAllWorld, GKBCAT_SQUAD },
		{ EGameBinds::SquadMarkerSetAgent1, GKBCAT_SQUAD }, // Arrow
		{ EGameBinds::SquadMarkerSetAgent2, GKBCAT_SQUAD }, // Circle
		{ EGameBinds::SquadMarkerSetAgent3, GKBCAT_SQUAD }, // Heart
		{ EGameBinds::SquadMarkerSetAgent4, GKBCAT_SQUAD }, // Square
		{ EGameBinds::SquadMarkerSetAgent5, GKBCAT_SQUAD }, // Star
		{ EGameBinds::SquadMarkerSetAgent6, GKBCAT_SQUAD }, // Swirl
		{ EGameBinds::SquadMarkerSetAgent7, GKBCAT_SQUAD }, // Triangle
		{ EGameBinds::SquadMarkerSetAgent8, GKBCAT_SQUAD }, // Cross
		{ EGameBinds::SquadMarkerClearAllAgent, GKBCAT_SQUAD },

		// Mastery Skills
		{ EGameBinds::MasteryAccess, GKBCAT_MASTERY },
		{ EGameBinds::MasteryAccess01, GKBCAT_MASTERY }, // Fishing
		{ EGameBinds::MasteryAccess02, GKBCAT_MASTERY }, // Skiff
		{ EGameBinds::MasteryAccess03, GKBCAT_MASTERY }, // Jade Bot Waypoint
		{ EGameBinds::MasteryAccess04, GKBCAT_MASTERY }, // Rift Scan
		{ EGameBinds::MasteryAccess05, GKBCAT_MASTERY }, // Skyscale
		{ EGameBinds::MasteryAccess06, GKBCAT_MASTERY }, // Homestead Doorway

		// Miscellaneous Binds
		{ EGameBinds::MiscAoELoot, GKBCAT_MISC },
		{ EGameBinds::MiscInteract, GKBCAT_MISC },
		{ EGameBinds::MiscShowEnemies, GKBCAT_MISC },
		{ EGameBinds::MiscShowAllies, GKBCAT_MISC },
		{ EGameBinds::MiscCombatStance, GKBCAT_MISC }, // Stow/Draw
		{ EGameBinds::MiscToggleLanguage, GKBCAT_MISC },
		{ EGameBinds::MiscTogglePetCombat, GKBCAT_MISC },
		{ EGameBinds::MiscToggleFullScreen, GKBCAT_MISC },
		{ EGameBinds::MiscToggleDecorationMode, GKBCAT_MISC }, // Decoration Mode

		// Toys/Novelties
		{ EGameBinds::ToyUseDefault, GKBCAT_MISC },
		{ EGameBinds::ToyUseSlot1, GKBCAT_MISC }, // Chair
		{ EGameBinds::ToyUseSlot2, GKBCAT_MISC }, // Instrument
		{ EGameBinds::ToyUseSlot3, GKBCAT_MISC }, // Held Item
		{ EGameBinds::ToyUseSlot4, GKBCAT_MISC }, // Toy
		{ EGameBinds::ToyUseSlot5, GKBCAT_MISC }, // Tonic
		//ToyUseSlot6 unused

		// Build Templates
		{ EGameBinds::Loadout1, GKBCAT_TEMPLATES },
		{ EGameBinds::Loadout2, GKBCAT_TEMPLATES },
		{ EGameBinds::Loadout3, GKBCAT_TEMPLATES },
		{ EGameBinds::Loadout4, GKBCAT_TEMPLATES },
		{ EGameBinds::Loadout5, GKBCAT_TEMPLATES },
		{ EGameBinds::Loadout6, GKBCAT_TEMPLATES },
		{ EGameBinds::Loadout7, GKBCAT_TEMPLATES },
		{ EGameBinds::Loadout8, GKBCAT_TEMPLATES },
		{ EGameBinds::Loadout9, GKBCAT_TEMPLATES },

		// Equipment Templates
		{ EGameBinds::GearLoadout1, GKBCAT_TEMPLATES },
		{ EGameBinds::GearLoadout2, GKBCAT_TEMPLATES },
		{ EGameBinds::GearLoadout3, GKBCAT_TEMPLATES },
		{ EGameBinds::GearLoadout4, GKBCAT_TEMPLATES },
		{ EGameBinds::GearLoadout5, GKBCAT_TEMPLATES },
		{ EGameBinds::GearLoadout6, GKBCAT_TEMPLATES },
		{ EGameBinds::GearLoadout7, GKBCAT_TEMPLATES },
		{ EGameBinds::GearLoadout8, GKBCAT_TEMPLATES },
		{ EGameBinds::GearLoadout9, GKBCAT_TEMPLATES }
	};

	auto it = s_GbCategoryNameLUT.find(aGameBind);

	if (it != s_GbCategoryNameLUT.end())
	{
		return it->second;
	}

	static std::string s_Nullstr = "";

	return s_Nullstr;
}

unsigned short GameScanCodeToScanCode(unsigned short aGameScanCode)
{
	static std::unordered_map<unsigned short, unsigned short> s_GSCToSCLUT =
	{
		{ 0, 0x38 }, // LeftAlt
		{ 1, 0x1D }, // LeftCtrl
		{ 2, 0x2A }, // LeftShift
		{ 3, 0x28 }, // Quote
		{ 4, 0x2B }, // Hash
		{ 5, 0x3A }, // CapsLock
		{ 6, 0x33 }, // Colon
		{ 7, 0x0C }, // Minus
		{ 8, 0x0D }, // Equals
		{ 9, 0x01 }, // Escape
		{ 10, 0x1A }, // OpenBracket
		{ 11, 0xE045 }, // NumLock
		{ 12, 0x34 }, // Period
		{ 13, 0x1B }, // CloseBracket
		{ 14, 0x27 }, // Semicolon
		{ 15, 0x35 }, // Slash
		{ 16, 0xE037 }, // Print
		{ 17, 0x29 }, // Tilde
		{ 18, 0x0E }, // Backspace
		{ 19, 0xE053 }, // Delete
		{ 20, 0x1C }, // Enter
		{ 21, 0x39 }, // Space
		{ 22, 0x0F }, // Tab
		{ 23, 0xE04F }, // End
		{ 24, 0xE047 }, // Home
		{ 25, 0xE052 }, // Insert
		{ 26, 0xE051 }, // Next
		{ 27, 0xE049 }, // Prior
		{ 28, 0xE050 }, // ArrowDown
		{ 29, 0xE04B }, // ArrowLeft
		{ 30, 0xE04D }, // ArrowRight
		{ 31, 0xE048 }, // ArrowUp
		{ 32, 0x3B }, // F1
		{ 33, 0x3C }, // F2
		{ 34, 0x3D }, // F3
		{ 35, 0x3E }, // F4
		{ 36, 0x3F }, // F5
		{ 37, 0x40 }, // F6
		{ 38, 0x41 }, // F7
		{ 39, 0x42 }, // F8
		{ 40, 0x43 }, // F9
		{ 41, 0x44 }, // F10
		{ 42, 0x57 }, // F11
		{ 43, 0x58 }, // F12
		{ 48, 0x0B }, // _0
		{ 49, 0x02 }, // _1
		{ 50, 0x03 }, // _2
		{ 51, 0x04 }, // _3
		{ 52, 0x05 }, // _4
		{ 53, 0x06 }, // _5
		{ 54, 0x07 }, // _6
		{ 55, 0x08 }, // _7
		{ 56, 0x09 }, // _8
		{ 57, 0x0A }, // _9
		{ 65, 0x1E }, // A
		{ 66, 0x30 }, // B
		{ 67, 0x2E }, // C
		{ 68, 0x20 }, // D
		{ 69, 0x12 }, // E
		{ 70, 0x21 }, // F
		{ 71, 0x22 }, // G
		{ 72, 0x23 }, // H
		{ 73, 0x17 }, // I
		{ 74, 0x24 }, // J
		{ 75, 0x25 }, // K
		{ 76, 0x26 }, // L
		{ 77, 0x32 }, // M
		{ 78, 0x31 }, // N
		{ 79, 0x18 }, // O
		{ 80, 0x19 }, // P
		{ 81, 0x10 }, // Q
		{ 82, 0x13 }, // R
		{ 83, 0x1F }, // S
		{ 84, 0x14 }, // T
		{ 85, 0x16 }, // U
		{ 86, 0x2F }, // V
		{ 87, 0x11 }, // W
		{ 88, 0x2D }, // X
		{ 89, 0x15 }, // Y
		{ 90, 0x2C }, // Z
		{ 91, 0x4E }, // PlusNum
		{ 92, 0x53 }, // DecimalNum
		{ 93, 0xE035 }, // DivideNum
		{ 94, 0x37 }, // MultiplyNum
		{ 95, 0x52 }, // _0_NUM
		{ 96, 0x4F }, // _1_NUM
		{ 97, 0x50 }, // _2_NUM
		{ 98, 0x51 }, // _3_NUM
		{ 99, 0x4B }, // _4_NUM
		{ 100, 0x4C }, // _5_NUM
		{ 101, 0x4D }, // _6_NUM
		{ 102, 0x47 }, // _7_NUM
		{ 103, 0x48 }, // _8_NUM
		{ 104, 0x49 }, // _9_NUM
		{ 105, 0xE01C }, // EnterNum
		{ 106, 0x4A }, // MinusNum
		{ 109, 0xE038 }, // RightAlt
		{ 110, 0xE01D }, // RightCtrl
		{ 111, 0x56 }, // Backslash
		{ 135, 0x36 }, // RightShift
	};

	auto it = s_GSCToSCLUT.find(aGameScanCode);

	if (it != s_GSCToSCLUT.end())
	{
		return it->second;
	}

	return 0;
}
