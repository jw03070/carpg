#include "Pch.h"
#include "GameCore.h"
#include "PlayerInfo.h"
#include "SaveState.h"
#include "NetChangePlayer.h"
#include "Net.h"

//=================================================================================================
PlayerInfo::PlayerInfo() : pc(nullptr), u(nullptr), clas(nullptr), left(LEFT_NO), update_flags(0), ready(false), loaded(false), warping(false)
{
}

//=================================================================================================
void PlayerInfo::Save(FileWriter& f)
{
	f << name;
	f << id;
	f << devmode;
	hd.Save(f);
	f << (u ? u->id : -1);
	f.WriteStringArray<int, word>(notes);
}

//=================================================================================================
void PlayerInfo::Load(FileReader& f)
{
	f >> name;
	if(LOAD_VERSION < V_0_12)
		f.Skip<int>(); // old class
	f >> id;
	f >> devmode;
	hd.Load(f);
	int unit_id;
	f >> unit_id;
	u = Unit::GetById(unit_id);
	clas = u->GetClass();
	f.ReadStringArray<int, word>(notes);
}

//=================================================================================================
int PlayerInfo::GetIndex() const
{
	int index = 0;
	for(PlayerInfo& info : net->players)
	{
		if(&info == this)
			return index;
		++index;
	}
	return -1;
}
