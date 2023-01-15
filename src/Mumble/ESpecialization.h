#ifndef ESPECIALIZATION_H
#define ESPECIALIZATION_H

namespace Mumble
{
	enum class ESpecialization : int
	{
		None = 0,

		/* HoT */
		Dragonhunter = 27,
		Berserker = 18,
		Scrapper = 43,
		Druid = 5,
		Daredevil = 7,
		Tempest = 48,
		Chronomancer = 40,
		Reaper = 43,
		Herald = 52,

		/* PoF */
		Firebrand = 62,
		Spellbreaker = 61,
		Holosmith = 57,
		Soulbeast = 55,
		Deadeye = 58,
		Weaver = 56,
		Mirage = 59,
		Scourge = 60,
		Renegade = 63,

		/* EoD */
		Willbender = 65,
		Bladesworn = 68,
		Mechanist = 70,
		Untamed = 72,
		Specter = 71,
		Catalyst = 67,
		Virtuoso = 66,
		Harbinger = 64,
		Vindicator = 69
	};
}

#endif