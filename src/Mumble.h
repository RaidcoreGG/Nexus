#ifndef MUMBLE_H
#define MUMBLE_H

#include <map>
#include <string>
#include <Windows.h>

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
struct Compass
{
	unsigned short	Width;
	unsigned short	Height;
	float			Rotation; // radians
	Vector2			PlayerPosition; // continent
	Vector2			Center; // continent
	float			Scale;
};

enum class MapType : unsigned
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
	WvW_EBG,
	WvW_BBL,
	WvW_GBL,
	WvW_RBL,
	WVW_FV,
	WvW_OS,
	WvW_EOTM,
	Public_Mini,
	BIG_BATTLE,
	WvW_Lounge,
	WvW
};
enum class MountIndex : unsigned char
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
enum class Profession : int
{
	Guardian		= 1,
	Warrior			= 2,
	Engineer		= 3,
	Ranger			= 4,
	Thief			= 5,
	Elementalist	= 6,
	Mesmer			= 7,
	Necromancer		= 8,
	Revenant		= 9
};
enum class Specialization : int
{
	None			= 0,

	/* HoT */
	Dragonhunter	= 27,
	Berserker		= 18,
	Scrapper		= 43,
	Druid			= 5,
	Daredevil		= 7,
	Tempest			= 48,
	Chronomancer	= 40,
	Reaper			= 43,
	Herald			= 52,

	/* PoF */
	Firebrand		= 62,
	Spellbreaker	= 61,
	Holosmith		= 57,
	Soulbeast		= 55,
	Deadeye			= 58,
	Weaver			= 56,
	Mirage			= 59,
	Scourge			= 60,
	Renegade		= 63,

	/* EoD */
	Willbender		= 65,
	Bladesworn		= 68,
	Mechanist		= 70,
	Untamed			= 72,
	Specter			= 71,
	Catalyst		= 67,
	Virtuoso		= 66,
	Harbinger		= 64,
	Vindicator		= 69
};
enum class Race : unsigned char
{
	Asura,
	Charr,
	Human,
	Norn,
	Sylvari
};
enum class UIScale
{
	Small,
	Normal,
	Large,
	Larger
};

struct Context
{
	unsigned char ServerAddress[28]; // contains sockaddr_in or sockaddr_in6
	unsigned MapID;
	MapType MapType;
	unsigned ShardID;
	unsigned InstanceID;
	unsigned BuildID;

	/* data beyond this point is not necessary for identification */
	unsigned IsMapOpen			: 1;
	unsigned IsCompassTopRight	: 1;
	unsigned IsCompassRotating	: 1;
	unsigned IsGameFocused		: 1;
	unsigned IsCompetitive		: 1;
	unsigned IsTextboxFocused	: 1;
	unsigned IsInCombat			: 1;
	// unsigned UNUSED1			: 1;
	Compass Compass;
	unsigned ProcessID;
	MountIndex MountIndex;
};

struct LinkedMem
{
	unsigned UIVersion;
	unsigned UITick;
	Vector3 AvatarPosition;
	Vector3 AvatarFront;
	Vector3 AvatarTop;
	wchar_t Name[256];
	Vector3 CameraPosition;
	Vector3 CameraFront;
	Vector3 CameraTop;
	wchar_t Identity[256];
	unsigned ContextLength;
	Context Context;
	wchar_t Description[2048];
};

class Mumble
{
public:
	static HANDLE Handle;
	static LinkedMem* Data;

	static LinkedMem* Create();
	static void Destroy();

private:
	static std::wstring GetMumbleName();
};

#endif