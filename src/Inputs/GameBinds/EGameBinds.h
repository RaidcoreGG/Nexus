#ifndef EGAMEBINDS_H
#define EGAMEBINDS_H

#include <string>
#include <map>

enum class EGameBinds
{
	// Movement
	MoveForward		= 0,
	MoveBackward	= 1,
	MoveLeft		= 2,
	MoveRight		= 3,
	MoveTurnLeft	= 4,
	MoveTurnRight	= 5,
	MoveDodge		= 6,
	MoveAutoRun		= 7,
	MoveWalk		= 8,
	MoveJump		= 9,
	MoveSwimUp		= 10,
	MoveSwimDown	= 11,
	MoveAboutFace	= 12,

	// Skills
	SkillWeaponSwap		= 17,
	SkillWeapon1		= 18,
	SkillWeapon2		= 19,
	SkillWeapon3		= 20,
	SkillWeapon4		= 21,
	SkillWeapon5		= 22,
	SkillHeal			= 23,
	SkillUtility1		= 24,
	SkillUtility2		= 25,
	SkillUtility3		= 26,
	SkillElite			= 27,
	SkillProfession1	= 28,
	SkillProfession2	= 29,
	SkillProfession3	= 30,
	SkillProfession4	= 31,
	SkillProfession5	= 79,
	SkillProfession6	= 201,
	SkillProfession7	= 202,
	SkillSpecialAction	= 82,

	// Targeting
	TargetAlert						= 131,
	TargetCall						= 32,
	TargetTake						= 33,
	TargetCallLocal					= 199,
	TargetTakeLocal					= 200,
	TargetEnemyNearest				= 34,
	TargetEnemyNext					= 35,
	TargetEnemyPrev					= 36,
	TargetAllyNearest				= 37,
	TargetAllyNext					= 38,
	TargetAllyPrev					= 39,
	TargetLock						= 40,
	TargetSnapGroundTarget			= 80,
	TargetSnapGroundTargetToggle	= 115,
	TargetAutoTargetingDisable		= 116,
	TargetAutoTargetingToggle		= 117,
	TargetAllyTargetingMode			= 197,
	TargetAllyTargetingModeToggle	= 198,

	// UI Binds
	UiCommerce						= 41, // TradingPost
	UiContacts						= 42,
	UiGuild							= 43,
	UiHero							= 44,
	UiInventory						= 45,
	UiKennel						= 46, // Pets
	UiLogout						= 47,
	UiMail							= 71,
	UiOptions						= 48,
	UiParty							= 49,
	UiPvp							= 73,
	UiPvpBuild						= 75,
	UiScoreboard					= 50,
	UiSeasonalObjectivesShop		= 209, // Wizard's Vault
	UiInformation					= 51,
	UiChatToggle					= 70,
	UiChatCommand					= 52,
	UiChatFocus						= 53,
	UiChatReply						= 54,
	UiToggle						= 55,
	UiSquadBroadcastChatToggle		= 85,
	UiSquadBroadcastChatCommand		= 83,
	UiSquadBroadcastChatFocus		= 84,

	// Camera
	CameraFree					= 13,
	CameraZoomIn				= 14,
	CameraZoomOut				= 15,
	CameraReverse				= 16,
	CameraActionMode			= 78,
	CameraActionModeDisable		= 114,

	// Screenshots
	ScreenshotNormal		= 56,
	ScreenshotStereoscopic	= 57,

	// Map
	MapToggle		= 59,
	MapFocusPlayer	= 60,
	MapFloorDown	= 61,
	MapFloorUp		= 62,
	MapZoomIn		= 63,
	MapZoomOut		= 64,

	// Mounts
	SpumoniToggle				= 152,
	SpumoniMovement				= 130,
	SpumoniSecondaryMovement	= 153,
	SpumoniMAM01				= 155, // Raptor
	SpumoniMAM02				= 156, // Springer
	SpumoniMAM03				= 157, // Skimmer
	SpumoniMAM04				= 158, // Jackal
	SpumoniMAM05				= 159, // Griffon
	SpumoniMAM06				= 161, // RollerBeetle
	SpumoniMAM07				= 169, // Warclaw
	SpumoniMAM08				= 170, // Skyscale
	SpumoniMAM09				= 203, // SiegeTurtle

	// Spectator Binds
	SpectatorNearestFixed		= 102,
	SpectatorNearestPlayer		= 103,
	SpectatorPlayerRed1			= 104,
	SpectatorPlayerRed2			= 105,
	SpectatorPlayerRed3			= 106,
	SpectatorPlayerRed4			= 107,
	SpectatorPlayerRed5			= 108,
	SpectatorPlayerBlue1		= 109,
	SpectatorPlayerBlue2		= 110,
	SpectatorPlayerBlue3		= 111,
	SpectatorPlayerBlue4		= 112,
	SpectatorPlayerBlue5		= 113,
	SpectatorFreeCamera			= 120,
	SpectatorFreeCameraMode		= 127,
	SpectatorFreeMoveForward	= 121,
	SpectatorFreeMoveBackward	= 122,
	SpectatorFreeMoveLeft		= 123,
	SpectatorFreeMoveRight		= 124,
	SpectatorFreeMoveUp			= 125,
	SpectatorFreeMoveDown		= 126,

	// Squad Markers
	SquadMarkerPlaceWorld1		= 86, // Arrow
	SquadMarkerPlaceWorld2		= 87, // Circle
	SquadMarkerPlaceWorld3		= 88, // Heart
	SquadMarkerPlaceWorld4		= 89, // Square
	SquadMarkerPlaceWorld5		= 90, // Star
	SquadMarkerPlaceWorld6		= 91, // Swirl
	SquadMarkerPlaceWorld7		= 92, // Triangle
	SquadMarkerPlaceWorld8		= 93, // Cross
	SquadMarkerClearAllWorld	= 119,
	SquadMarkerSetAgent1		= 94, // Arrow
	SquadMarkerSetAgent2		= 95, // Circle
	SquadMarkerSetAgent3		= 96, // Heart
	SquadMarkerSetAgent4		= 97, // Square
	SquadMarkerSetAgent5		= 98, // Star
	SquadMarkerSetAgent6		= 99, // Swirl
	SquadMarkerSetAgent7		= 100, // Triangle
	SquadMarkerSetAgent8		= 101, // Cross
	SquadMarkerClearAllAgent	= 118,

	// Mastery Skills
	MasteryAccess		= 196,
	MasteryAccess01		= 204, // Fishing
	MasteryAccess02		= 205, // Skiff
	MasteryAccess03		= 206, // Jade Bot Waypoint
	MasteryAccess04		= 207, // Rift Scan
	MasteryAccess05		= 208, // Skyscale
	MasteryAccess06		= 211, // Homestead Doorway

	// Miscellaneous Binds
	MiscAoELoot					= 74,
	MiscInteract				= 65,
	MiscShowEnemies				= 66,
	MiscShowAllies				= 67,
	MiscCombatStance			= 68, // Stow/Draw
	MiscToggleLanguage			= 69,
	MiscTogglePetCombat			= 76,
	MiscToggleFullScreen		= 160,
	MiscToggleDecorationMode	= 210, // DecorateMode

	// Toys/Novelties
	ToyUseDefault	= 162,
	ToyUseSlot1		= 163, // Chair
	ToyUseSlot2		= 164, // Instrument
	ToyUseSlot3		= 165, // Held Item
	ToyUseSlot4		= 166, // Toy
	ToyUseSlot5		= 167, // Tonic
	//ToyUseSlot6 unused

	// Build Templates
	Loadout1 = 171,
	Loadout2 = 172,
	Loadout3 = 173,
	Loadout4 = 174,
	Loadout5 = 175,
	Loadout6 = 176,
	Loadout7 = 177,
	Loadout8 = 178,

	// Equipment Templates
	GearLoadout1 = 182,
	GearLoadout2 = 183,
	GearLoadout3 = 184,
	GearLoadout4 = 185,
	GearLoadout5 = 186,
	GearLoadout6 = 187,
	GearLoadout7 = 188,
	GearLoadout8 = 189
};

#endif
