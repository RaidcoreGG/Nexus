///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Migrations.h
/// Description  :  Upgrades files left by older versions.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#pragma once

namespace Migrations
{
	///----------------------------------------------------------------------------------------------------
	/// Run:
	/// 	Runs every migration. Called at startup before any component reads its files.
	///----------------------------------------------------------------------------------------------------
	void Run();
}
