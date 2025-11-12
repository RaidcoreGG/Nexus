#ifndef MUMBLE_H
#define MUMBLE_H

struct Vector2
{
	float X;
	float Y;
};

struct Vector3
{
	float X;
	float Y;
	float Z;
};

namespace Mumble
{
	/* enums */
	enum class EMapType : unsigned char
	{
		AutoRedirect,
		CharacterCreation,
		PvP,
		GvG,
		Instance,
		Public,
		Tournament,
		Tutorial,
		UserTournament,
		WvW_EternalBattlegrounds,
		WvW_BlueBorderlands,
		WvW_GreenBorderlands,
		WvW_RedBorderlands,
		WVW_FortunesVale,
		WvW_ObsidianSanctum,
		WvW_EdgeOfTheMists,
		Public_Mini,
		BigBattle,
		WvW_Lounge
	};

	enum class EMountIndex : unsigned char
	{
		None,
		Jackal,
		Griffon,
		Springer,
		Skimmer,
		Raptor,
		RollerBeetle,
		Warclaw,
		Skyscale,
		Skiff,
		SiegeTurtle
	};

	enum class EProfession : unsigned char
	{
		None,
		Guardian,
		Warrior,
		Engineer,
		Ranger,
		Thief,
		Elementalist,
		Mesmer,
		Necromancer,
		Revenant
	};

	enum class ERace : unsigned char
	{
		Asura,
		Charr,
		Human,
		Norn,
		Sylvari
	};

	enum class EUIScale : unsigned char
	{
		Small,
		Normal,
		Large,
		Larger
	};

	/* structs */
	struct Identity
	{
		char			Name[20];
		EProfession		Profession;
		unsigned		Specialization;
		ERace			Race;
		unsigned		MapID;
		unsigned		WorldID;
		unsigned		TeamColorID;
		bool			IsCommander;		// is the player currently tagged up
		float			FOV;
		EUIScale		UISize;
	};

	struct Compass
	{
		unsigned short	Width;
		unsigned short	Height;
		float			Rotation;			// radians
		Vector2			PlayerPosition; 	// continent
		Vector2			Center;				// continent
		float			Scale;
	};

	struct Context
	{
		unsigned char 	ServerAddress[28]; 	// contains sockaddr_in or sockaddr_in6
		unsigned 		MapID;
		EMapType 		MapType;
		unsigned 		ShardID;
		unsigned 		InstanceID;
		unsigned 		BuildID;
		unsigned 		IsMapOpen			: 1;
		unsigned 		IsCompassTopRight	: 1;
		unsigned 		IsCompassRotating	: 1;
		unsigned 		IsGameFocused		: 1;
		unsigned 		IsCompetitive		: 1;
		unsigned 		IsTextboxFocused	: 1;
		unsigned 		IsInCombat			: 1;
		// unsigned		UNUSED1				: 1;
		Compass 		Compass;
		unsigned 		ProcessID;
		EMountIndex 	MountIndex;
	};

	struct Data
	{
		unsigned		UIVersion;
		unsigned		UITick;
		Vector3			AvatarPosition;
		Vector3			AvatarFront;
		Vector3			AvatarTop;
		wchar_t			Name[256];
		Vector3			CameraPosition;
		Vector3			CameraFront;
		Vector3			CameraTop;
		wchar_t			Identity[256];
		unsigned		ContextLength;
		Context			Context;
		wchar_t			Description[2048];
	};
}

#endif
