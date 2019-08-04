#pragma once

//-----------------------------------------------------------------------------
#include "Resource.h"

//-----------------------------------------------------------------------------
enum class MusicType
{
	None,
	Intro,
	Title,
	Forest,
	City,
	Crypt,
	Dungeon,
	Boss,
	Travel,
	Moonwell,
	Death
};

//-----------------------------------------------------------------------------
struct MusicTrack
{
	MusicPtr music;
	MusicType type;

	static vector<MusicTrack*> tracks;
	static uint Load(ResourceManager& res_mgr, uint& errors);
	static void Cleanup();
};
