#include "Pch.h"
#include "Base.h"
#include "Game.h"
#include "Terrain.h"
#include "EnemyGroup.h"
#include "CityGenerator.h"
#include "Inventory.h"
#include "Quest_Sawmill.h"
#include "Quest_Bandits.h"
#include "Quest_Orcs.h"
#include "Quest_Evil.h"
#include "Quest_Crazies.h"

extern const float TRAVEL_SPEED = 28.f;
extern MATRIX m1, m2, m3, m4;

//#define START_LOCATION L_VILLAGE
//#define START_LOCATION2 MAGE_TOWER

// z powodu zmian (po��czenie Location i Location2) musze tymczasowo u�ywa� tego w add_locations a potem w generate_world ustawia� odpowiedni obiekt
struct TmpLocation : public Location
{
	TmpLocation() : Location(false)
	{

	}

	void ApplyContext(LevelContext& ctx)
	{

	}

	void BuildRefidTable()
	{

	}
};

Location* Game::CreateLocation(LOCATION type, int levels)
{
	switch(type)
	{
	case L_CITY:
		return new City;
	case L_VILLAGE:
		return new Village;
	case L_CAVE:
		return new CaveLocation;
	case L_CAMP:
		return new Camp;
	case L_FOREST:
	case L_ENCOUNTER:
	case L_MOONWELL:
	case L_ACADEMY:
		return new OutsideLocation;
	case L_DUNGEON:
	case L_CRYPT:
		if(levels == -1)
			return new TmpLocation; // :(
		else
		{
			assert(levels != 0);
			if(levels == 1)
				return new SingleInsideLocation;
			else
				return new MultiInsideLocation(levels);
		}
	default:
		assert(0);
		return NULL;
	}
}

void Game::AddLocations(uint count, LOCATION type, float valid_dist, bool unique_name)
{
	for(uint i=0; i<count; ++i)
	{
		for(uint j=0; j<100; ++j)
		{
			VEC2 pt = random_VEC2(16.f,600.f-16.f);
			bool jest = false;

			for(vector<Location*>::iterator it = locations.begin(), end = locations.end(); it != end; ++it)
			{
				float dist = distance(pt, (*it)->pos);
				if(dist < valid_dist)
				{
					jest = true;
					break;
				}
			}

			if(!jest)
			{
				Location* l = CreateLocation(type);
				locations.push_back(l);
				l->pos = pt;
				l->type = type;
				l->state = LS_UNKNOWN;
				l->GenerateName();
				if(unique_name && locations.size() > 1)
				{
					bool jest;
					do
					{
						jest = false;

						for(vector<Location*>::iterator it = locations.begin(), end = locations.end()-1; it != end; ++it)
						{
							if((*it)->name == l->name)
							{
								jest = true;
								l->GenerateName();
								break;
							}
						}
					}
					while(jest);
				}
				goto added;
			}
		}

		// not added
		break;

added:
		;
	}
}

void Game::AddLocations(uint count, const LOCATION* types, uint type_count, float valid_dist, bool unique_name)
{
	for(uint i=0; i<count; ++i)
	{
		for(uint j=0; j<100; ++j)
		{
			VEC2 pt = random_VEC2(16.f,600.f-16.f);
			bool jest = false;

			for(vector<Location*>::iterator it = locations.begin(), end = locations.end(); it != end; ++it)
			{
				float dist = distance(pt, (*it)->pos);
				if(dist < valid_dist)
				{
					jest = true;
					break;
				}
			}

			if(!jest)
			{
				LOCATION type = types[rand2()%type_count];
				Location* l = CreateLocation(type);
				locations.push_back(l);
				l->pos = pt;
				l->type = type;
				l->state = LS_UNKNOWN;
				l->GenerateName();
				if(unique_name && locations.size() > 1)
				{
					bool jest;
					do
					{
						jest = false;

						for(vector<Location*>::iterator it = locations.begin(), end = locations.end()-1; it != end; ++it)
						{
							if((*it)->name == l->name)
							{
								jest = true;
								l->GenerateName();
								break;
							}
						}
					}
					while(jest);
				}
				goto added;
			}
		}

		// not added
		break;

added:
		;
	}
}

void Game::AddLocations(const LOCATION* types, uint count, float valid_dist, bool unique_name)
{
	for(uint i=0; i<count; ++i)
	{
		for(uint j=0; j<100; ++j)
		{
			VEC2 pt = random_VEC2(16.f,600.f-16.f);
			bool jest = false;

			for(vector<Location*>::iterator it = locations.begin(), end = locations.end(); it != end; ++it)
			{
				float dist = distance(pt, (*it)->pos);
				if(dist < valid_dist)
				{
					jest = true;
					break;
				}
			}

			if(!jest)
			{
				LOCATION type = types[i];
				Location* l = CreateLocation(type);
				locations.push_back(l);
				l->pos = pt;
				l->type = types[i];
				l->state = LS_UNKNOWN;
				l->GenerateName();
				if(unique_name && locations.size() > 1)
				{
					bool jest;
					do
					{
						jest = false;

						for(vector<Location*>::iterator it = locations.begin(), end = locations.end()-1; it != end; ++it)
						{
							if((*it)->name == l->name)
							{
								jest = true;
								l->GenerateName();
								break;
							}
						}
					}
					while(jest);
				}
				goto added;
			}
		}

		// not added
		break;

added:
		;
	}
}

void Game::GenerateWorld()
{
	if(next_seed != 0)
	{
		srand2(next_seed);
		next_seed = 0;
	}
	else if(force_seed != 0 && force_seed_all)
		srand2(force_seed);

	LOG(Format("Generating world, seed %u.", rand_r2()));

	// generuj miasta
	empty_locations = 0;
	uint ile_miast = random(3,5);
	AddLocations(ile_miast, L_CITY, 200.f, true);

	// losuj startowe miasto
	uint startowe = rand2()%ile_miast;

	// generuj wioski
	uint ile_wiosek = 5-ile_miast + random(10,15);
	AddLocations(ile_wiosek, L_VILLAGE, 100.f, true);

	cities = locations.size();

	// ustaw miasta i wioski jako znane
	for(vector<Location*>::iterator it = locations.begin(), end = locations.end(); it != end; ++it)
		(*it)->state = LS_KNOWN;

	// dodaj gwarantowan� liczb� lokalizacji
	const LOCATION gwarantowane[] = {
		L_MOONWELL,
		L_FOREST,
		L_CAVE,
		L_CRYPT, L_CRYPT,
		L_DUNGEON, L_DUNGEON, L_DUNGEON, L_DUNGEON, L_DUNGEON, L_DUNGEON, L_DUNGEON, L_DUNGEON
	};

	AddLocations(gwarantowane, countof(gwarantowane), 32.f, false);

	// generuj pozosta�e
	const LOCATION locs[] = {
		L_CAVE,
		L_DUNGEON,
		L_DUNGEON,
		L_DUNGEON,
		L_CRYPT,
		L_FOREST
	};
	AddLocations(100-locations.size(), locs, countof(locs), 32.f, false);

	{
		Location* loc = new OutsideLocation;
		locations.push_back(loc);
		loc->pos = VEC2(-1000,-1000);
		loc->name = txRandomEncounter;
		loc->state = LS_VISITED;
		loc->type = L_ENCOUNTER;
		encounter_loc = locations.size()-1;
	}

	int index = 0, gwarant_podziemia = 0, gwarant_krypta = 0;
#ifdef START_LOCATION
	int temp_location;
#endif

	// ujawnij pobliskie, generuj zawarto��
	for(vector<Location*>::iterator it = locations.begin(), end = locations.end(); it != end; ++it, ++index)
	{
		if(!*it)
			continue;
		if((*it)->state == LS_UNKNOWN && distance(locations[startowe]->pos, (*it)->pos) <= 100.f)
			(*it)->state = LS_KNOWN;

#ifdef START_LOCATION
#ifndef START_LOCATION2
		if((*it)->type == START_LOCATION)
			temp_location = index;
#endif
#endif

		if((*it)->type == L_CITY)
		{
			City* c = (City*)*it;
			c->citizens = random(12, 15);
			c->citizens_world = random(0,99)+c->citizens*200;
		}
		else if((*it)->type == L_VILLAGE)
		{
			Village* v = (Village*)*it;
			int ile = 0;
			if(rand2()%2 == 0)
				v->v_buildings[ile++] = B_BLACKSMITH;
			if(rand2()%2 == 0)
				v->v_buildings[ile++] = B_TRAINING_GROUND;
			if(ile != 2 && rand2()%2 == 0)
				v->v_buildings[ile++] = B_ALCHEMIST;
			if(ile == 0)
			{
				v->v_buildings[0] = B_NONE;
				v->v_buildings[1] = B_NONE;
			}
			else if(ile == 1)
				v->v_buildings[1] = B_NONE;
			v->citizens = 5+random((ile+1), (ile+1)*2);
			v->citizens_world = random(0,99)+v->citizens*100;
		}
		else if((*it)->type == L_DUNGEON || (*it)->type == L_CRYPT)
		{
			InsideLocation* inside;

			int cel;
			if((*it)->type == L_CRYPT)
			{
				if(gwarant_krypta == -1)
					cel = (rand2()%2 == 0 ? HERO_CRYPT : MONSTER_CRYPT);
				else if(gwarant_krypta == 0)
				{
					cel = HERO_CRYPT;
					++gwarant_krypta;
				}
				else if(gwarant_krypta == 1)
				{
					cel = MONSTER_CRYPT;
					gwarant_krypta = -1;
				}
			}
			else
			{
				if(gwarant_podziemia == -1)
				{
					switch(rand2()%14)
					{
					case 0:
					case 1:
						cel = HUMAN_FORT;
						break;
					case 2:
					case 3:
						cel = DWARF_FORT;
						break;
					case 4:
					case 5:
						cel = MAGE_TOWER;
						break;
					case 6:
					case 7:
						cel = BANDITS_HIDEOUT;
						break;
					case 8:
					case 9:
						cel = OLD_TEMPLE;
						break;
					case 10:
						cel = VAULT;
						break;
					case 11:
					case 12:
						cel = NECROMANCER_BASE;
						break;
					case 13:
						cel = LABIRYNTH;
						break;
					default:
						assert(0);
						cel = HUMAN_FORT;
						break;
					}
				}
				else
				{
					switch(gwarant_podziemia)
					{
					case 0:
						cel = HUMAN_FORT;
						break;
					case 1:
						cel = DWARF_FORT;
						break;
					case 2:
						cel = MAGE_TOWER;
						break;
					case 3:
						cel = BANDITS_HIDEOUT;
						break;
					case 4:
						cel = OLD_TEMPLE;
						break;
					case 5:
						cel = VAULT;
						break;
					case 6:
						cel = NECROMANCER_BASE;
						break;
					case 7:
						cel = LABIRYNTH;
						break;
					default:
						assert(0);
						cel = HUMAN_FORT;
						break;
					}

					++gwarant_podziemia;
					if(cel == 8)
						gwarant_podziemia = -1;
				}
			}

			BaseLocation& base = g_base_locations[cel];
			int poziomy = random(base.levels);
			Location* loc = *it;

			if(poziomy == 1)
				inside = new SingleInsideLocation;
			else
				inside = new MultiInsideLocation(poziomy);

			inside->type = loc->type;
			inside->state = loc->state;
			inside->pos = loc->pos;
			inside->name = loc->name;
			delete loc;
			*it = inside;

			inside->target = cel;
			if(cel == LABIRYNTH)
			{
				inside->st = random(8,15);
				inside->spawn = SG_NIEUMARLI;
			}
			else
			{
				inside->spawn = RandomSpawnGroup(base);
				if(distance(inside->pos, locations[startowe]->pos) < 100)
					inside->st = random(3, 6);
				else
					inside->st = random(3, 15);
			}

#ifdef START_LOCATION2
			if(inside->cel == START_LOCATION2)
				temp_location = index;
#endif
		}
		else if((*it)->type == L_CAVE || (*it)->type == L_FOREST || (*it)->type == L_ENCOUNTER)
		{
			if(distance((*it)->pos, locations[startowe]->pos) < 100)
				(*it)->st = random(3,6);
			else
				(*it)->st = random(3,13);
		}
		else if((*it)->type == L_MOONWELL)
			(*it)->st = 10;
	}
	
	// koniec generowania
#ifdef START_LOCATION
	current_location = temp_location;
#else
	current_location = startowe;
#endif
	world_pos = locations[current_location]->pos;

	LOG(Format("Randomness integrity: %d", rand2_tmp()));
}

const INT2 g_kierunek[4] = {
	INT2(0,-1),
	INT2(-1,0),
	INT2(0,1),
	INT2(1,0)
};

bool Game::EnterLocation(int level, int from_portal, bool close_portal)
{
	Location& l = *locations[current_location];
	if(l.type == L_ACADEMY)
	{
		ShowAcademyText();
		return false;
	}

	world_map->visible = false;
	game_gui->visible = true;

	bool reenter = (open_location == current_location);
	open_location = current_location;
	if(from_portal != -1)
		enter_from = ENTER_FROM_PORTAL+from_portal;
	else
		enter_from = ENTER_FROM_OUTSIDE;
	if(!reenter)
		light_angle = random(PI*2);

	location = &l;
	dungeon_level = level;
	location_event_handler = NULL;
	before_player = BP_NONE;
	arena_free = true; //zabezpieczenie :3
	unit_views.clear();
	Inventory::lock_id = LOCK_NO;
	Inventory::lock_give = false;

	bool first = false;
	int steps;

	if(l.state != LS_ENTERED && l.state != LS_CLEARED)
	{
		first = true;
		level_generated = true;
	}
	else
		level_generated = false;

	if(!reenter)
		InitQuadTree();

	if(IsOnline() && players > 1)
	{
		packet_data.resize(3);
		packet_data[0] = ID_CHANGE_LEVEL;
		packet_data[1] = (byte)current_location;
		packet_data[2] = 0;
		int ack = peer->Send((cstring)&packet_data[0], 3, HIGH_PRIORITY, RELIABLE_WITH_ACK_RECEIPT, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
		for(vector<PlayerInfo>::iterator it = game_players.begin(), end = game_players.end(); it != end; ++it)
		{
			if(it->id == my_id)
				it->state = PlayerInfo::IN_GAME;
			else
			{
				it->state = PlayerInfo::WAITING_FOR_RESPONSE;
				it->ack = ack;
				it->timer = 5.f;
			}
		}
		Net_FilterServerChanges();
	}

	switch(l.type)
	{
	case L_CITY:
	case L_VILLAGE:
		steps = 5;
		if(l.state != LS_ENTERED && l.state != LS_CLEARED)
		{
			if(l.type == L_CITY)
				steps += 11;
			else
				steps += 10;
		}
		if(first)
			steps += 5;
		else if(!reenter)
		{
			steps += 3;
			if(l.last_visit != worldtime)
				++steps;
		}
		break;
	case L_DUNGEON:
	case L_CRYPT:
	case L_CAVE:
		steps = 3;
		if(first)
			steps += 3;
		else if(!reenter)
			++steps;
		break;
	case L_FOREST:
	case L_CAMP:
	case L_MOONWELL:
		steps = 3;
		if(first)
			steps += 3;
		if(!reenter)
			steps += 3;
		break;
	case L_ENCOUNTER:
		steps = 6;
		break;
	default:
		assert(0);
		steps = 6;
		break;
	}

	LoadingStart(steps);
	LoadingStep(txEnteringLocation);
	
	if(l.state != LS_ENTERED && l.state != LS_CLEARED)
	{
		if(next_seed != 0)
		{
			srand2(next_seed);
			next_seed = 0;
			if(next_seed_extra)
			{
				if(l.type == L_CITY)
				{
					City* city = (City*)&l;
					city->citizens = next_seed_val[0];
				}
				else if(l.type == L_VILLAGE)
				{
					Village* village = (Village*)&l;
					village->citizens = next_seed_val[0];
					village->v_buildings[0] = (BUILDING)next_seed_val[1];
					village->v_buildings[1] = (BUILDING)next_seed_val[2];
				}
				next_seed_extra = false;
			}
		}
		else if(force_seed != 0 && force_seed_all)
			srand2(force_seed);

		// log what is required to generate location for testing
		if(l.type == L_CITY)
		{
			City* city = (City*)&l;
			LOG(Format("Generating location '%s', seed %u [%d].", l.name.c_str(), rand_r2(), city->citizens));
		}
		else if(l.type == L_VILLAGE)
		{
			Village* village = (Village*)&l;
			LOG(Format("Generating location '%s', seed %u [%d,%d,%d].", l.name.c_str(), rand_r2(), village->citizens, village->v_buildings[0], village->v_buildings[1]));
		}
		else
			LOG(Format("Generating location '%s', seed %u.", l.name.c_str(), rand_r2()));
		l.seed = rand_r2();
		if(!l.outside)
		{
			InsideLocation* inside = (InsideLocation*)location;
			if(inside->IsMultilevel())
			{
				MultiInsideLocation* multi = (MultiInsideLocation*)inside;
				multi->infos[dungeon_level].seed = l.seed;
			}
		}

		// nie odwiedzono, trzeba wygenerowa�
		l.state = LS_ENTERED;

		LoadingStep(txGeneratingMap);

		switch(l.type)
		{
		case L_CITY:
			GenerateCityMap(l);
			break;
		case L_VILLAGE:
			LOG(Format("Randomness integrity: %d", rand2_tmp()));
			GenerateVillageMap(l);
			LOG(Format("Randomness integrity: %d", rand2_tmp()));
			break;
		case L_DUNGEON:
		case L_CRYPT:
			{
				InsideLocation* inside = (InsideLocation*)location;
				inside->SetActiveLevel(0);
				if(inside->IsMultilevel())
				{
					MultiInsideLocation* multi = (MultiInsideLocation*)inside;
					if(multi->generated == 0)
						++multi->generated;
				}
				LOG(Format("Generating dungeon, target %d.", inside->target));
				GenerateDungeon(l);
			}
			break;
		case L_CAVE:
			GenerateCave(l);
			break;
		case L_FOREST:
			if(current_location == sekret_gdzie2)
				GenerateSecretLocation(l);
			else
				GenerateForest(l);
			break;
		case L_ENCOUNTER:
			GenerateEncounterMap(l);
			break;
		case L_CAMP:
			GenerateCamp(l);
			break;
		case L_MOONWELL:
			GenerateMoonwell(l);
			break;
		default:
			assert(0);
			break;
		}
	}
	else if(l.type != L_DUNGEON && l.type != L_CRYPT)
		LOG(Format("Entering location '%s'.", l.name.c_str()));

	city_ctx = NULL;

	switch(l.type)
	{
	case L_CITY:
	case L_VILLAGE:
		{
			// ustaw wska�niki
			City* city = (City*)location;
			city_ctx = city;
			arena_free = true;
			
			if(!reenter)
			{
				// ustaw kontekst
				ApplyContext(city, local_ctx);
				// ustaw teren
				ApplyTiles(city->h, city->tiles);
			}

			SetOutsideParams();

			// czy to pierwsza wizyta?
			if(first)
			{
				// stw�rz budynki
				LoadingStep(txGeneratingBuildings);
				SpawnBuildings(city->buildings);

				// generuj obiekty
				LoadingStep(txGeneratingObjects);
				SpawnCityObjects();
				
				// generuj jednostki
				LoadingStep(txGeneratingUnits);
				SpawnUnits(city);
				SpawnTmpUnits(city);

				// generuj przedmioty
				LoadingStep(txGeneratingItems);
				GenerateStockItems();
				GenerateCityPickableItems();

				ResetCollisionPointers();
			}
			else if(!reenter)
			{
				// resetowanie bohater�w/questowych postaci
				if(city->reset)
					RemoveTmpUnits(city);

				// usuwanie krwii/zw�ok
				int days;
				city->CheckUpdate(days, worldtime);
				if(days > 0)
					UpdateLocation(days, 100, false);

				for(vector<InsideBuilding*>::iterator it = city_ctx->inside_buildings.begin(), end = city_ctx->inside_buildings.end(); it != end; ++it)
				{
					if((*it)->ctx.require_tmp_ctx && !(*it)->ctx.tmp_ctx)
						(*it)->ctx.SetTmpCtx(tmp_ctx_pool.Get());
				}

				// odtw�rz jednostki
				LoadingStep(txGeneratingUnits);
				RespawnUnits();
				RepositionCityUnits();

				// odtw�rz fizyk�
				LoadingStep(txGeneratingPhysics);
				RespawnObjectColliders();
				RespawnBuildingPhysics();

				if(city->reset)
				{
					SpawnTmpUnits(city);
					city->reset = false;
				}

				// generuj przedmioty
				if(days > 0)
				{
					LoadingStep(txGeneratingItems);
					GenerateStockItems();
					if(days >= 10)
						GenerateCityPickableItems();
				}

				OnReenterLevel(local_ctx);
				for(vector<InsideBuilding*>::iterator it = city_ctx->inside_buildings.begin(), end = city_ctx->inside_buildings.end(); it != end; ++it)
					OnReenterLevel((*it)->ctx);
			}
			
			if(!reenter)
			{
				// stw�rz obiekt kolizji terenu
				LoadingStep(txRecreatingObjects);
				SpawnTerrainCollider();
				SpawnCityPhysics();
				SpawnOutsideBariers();
			}

			// pojawianie sie questowej jednostki
			if(location->active_quest && location->active_quest != (Quest_Dungeon*)ACTIVE_QUEST_HOLDER && !location->active_quest->done)
			{
				location->active_quest->done = true;
				if(location->active_quest->unit_to_spawn)
				{
					Unit* u = SpawnUnitInsideInn(*location->active_quest->unit_to_spawn, location->active_quest->unit_spawn_level);
					assert(u);
					if(u)
					{
						u->rot = random(MAX_ANGLE);
						if(location->active_quest->unit_dont_attack)
							u->dont_attack = true;
						if(location->active_quest->unit_auto_talk)
							u->auto_talk = 1;
						if(location->active_quest->unit_event_handler)
						{
							u->event_handler = location->active_quest->unit_event_handler;
							if(location->active_quest->send_spawn_event)
								u->event_handler->HandleUnitEvent(UnitEventHandler::SPAWN, u);
						}

						DEBUG_LOG(Format("Generated quest unit %s.", u->data->id.c_str(), u->GetName()));
					}
				}
			}

			// generuj minimap�
			LoadingStep(txGeneratingMinimap);
			CreateCityMinimap();

			// dodaj gracza i jego dru�yn�
			VEC3 spawn_pos;
			float spawn_dir;
			GetCityEntry(spawn_pos, spawn_dir);
			AddPlayerTeam(spawn_pos, spawn_dir, reenter, true);

			if(!reenter)
				GenerateQuestUnits();

			for(vector<Unit*>::iterator it = team.begin(), end = team.end(); it != end; ++it)
			{
				if((*it)->IsHero())
					(*it)->hero->lost_pvp = false;
			}

			CheckTeamItems();

			arena_free = true;

			if(!chlanie_wygenerowano && current_location == chlanie_gdzie && chlanie_stan == 2)
				SpawnDrunkmans();
		}
		break;
	case L_DUNGEON:
	case L_CRYPT:
	case L_CAVE:
		EnterLevel(first, reenter, false, from_portal, true);
		break;
	case L_FOREST:
		{
			// ustaw wska�niki
			OutsideLocation* forest = (OutsideLocation*)location;
			city_ctx = NULL;
			if(!reenter)
				ApplyContext(forest, local_ctx);

			int days;
			bool need_reset = forest->CheckUpdate(days, worldtime);

			// ustaw teren
			if(!reenter)
				ApplyTiles(forest->h, forest->tiles);

			SetOutsideParams();

			if(sekret_gdzie2 == current_location)
			{
				// czy to pierwsza wizyta?
				if(first)
				{
					// generuj obiekty
					LoadingStep(txGeneratingObjects);
					SpawnSecretLocationObjects();

					// generuj jednostki
					LoadingStep(txGeneratingUnits);
					SpawnSecretLocationUnits();
				}
				else if(!reenter)
				{
					// odtw�rz jednostki
					LoadingStep(txGeneratingUnits);
					RespawnUnits();

					// odtw�rz fizyk�
					LoadingStep(txGeneratingPhysics);
					RespawnObjectColliders();

					OnReenterLevel(local_ctx);
				}

				// stw�rz obiekt kolizji terenu
				if(!reenter)
				{
					LoadingStep(txRecreatingObjects);
					SpawnTerrainCollider();
					SpawnOutsideBariers();
				}

				// generuj minimap�
				LoadingStep(txGeneratingMinimap);
				CreateForestMinimap();

				// dodaj gracza i jego dru�yn�
				SpawnTeamSecretLocation();
			}
			else
			{
				VEC3 pos;
				float dir;
				GetOutsideSpawnPoint(pos, dir);

				// czy to pierwsza wizyta?
				if(first)
				{
					// generuj obiekty
					LoadingStep(txGeneratingObjects);
					SpawnForestObjects();

					// generuj jednostki
					LoadingStep(txGeneratingUnits);
					SpawnForestUnits(pos);
				}
				else if(!reenter)
				{
					if(days > 0)
						UpdateLocation(days, 100, false);

					if(need_reset)
					{
						// usu� �ywe jednostki
						for(vector<Unit*>::iterator it = local_ctx.units->begin(), end = local_ctx.units->end(); it != end; ++it)
						{
							if((*it)->IsAlive())
							{
								delete *it;
								*it = NULL;
							}
						}
						local_ctx.units->erase(std::remove_if(local_ctx.units->begin(), local_ctx.units->end(), is_null<Unit*>), local_ctx.units->end());
					}

					// sawmill quest
					if(current_location == quest_sawmill->target_loc)
					{
						if(quest_sawmill->sawmill_state == Quest_Sawmill::State::InBuild && quest_sawmill->build_state == Quest_Sawmill::BuildState::LumberjackLeft)
							GenerateSawmill(true);
						else if(quest_sawmill->sawmill_state == Quest_Sawmill::State::Working && quest_sawmill->build_state != Quest_Sawmill::BuildState::Finished)
							GenerateSawmill(false);
					}

					// odtw�rz jednostki
					LoadingStep(txGeneratingUnits);
					RespawnUnits();

					// odtw�rz fizyk�
					LoadingStep(txGeneratingPhysics);
					RespawnObjectColliders();

					if(need_reset)
					{
						// nowe jednostki
						SpawnForestUnits(pos);
					}

					OnReenterLevel(local_ctx);
				}

				// stw�rz obiekt kolizji terenu
				if(!reenter)
				{
					LoadingStep(txRecreatingObjects);
					SpawnTerrainCollider();
					SpawnOutsideBariers();
				}

				// questowe rzeczy
				if(forest->active_quest && forest->active_quest != (Quest_Dungeon*)ACTIVE_QUEST_HOLDER)
				{
					Quest_Event* event = forest->active_quest->GetEvent(current_location);
					if(event)
					{
						if(!event->done && dungeon_level == event->at_level)
						{
							event->done = true;
							Unit* spawned = NULL;
							if(event->unit_to_spawn)
							{
								spawned = SpawnUnitNearLocation(local_ctx, random(VEC3(90,0,90), VEC3(256-90,0,256-90)), *event->unit_to_spawn, NULL, event->unit_spawn_level);
								assert(spawned);
								spawned->dont_attack = event->unit_dont_attack;
								DEBUG_LOG(Format("Generated unit %s (%g,%g).", event->unit_to_spawn->id.c_str(), spawned->pos.x, spawned->pos.z));
							}
							if(event->spawn_item == Quest_Dungeon::Item_GiveStrongest)
							{
								Unit* best = NULL;
								for(vector<Unit*>::iterator it = local_ctx.units->begin(), end = local_ctx.units->end(); it != end; ++it)
								{
									if((*it)->IsAlive() && IsEnemy(**it, *pc->unit) && (!best || (*it)->level > best->level))
										best = *it;
								}
								assert(best);
								if(best)
								{
									best->AddItem(event->item_to_give[0], 1, true);
									DEBUG_LOG(Format("Item %s given to %s (%g,%g).", event->item_to_give[0]->id.c_str(), best->data->id.c_str(), best->pos.x, best->pos.z));
								}
							}
							else if(event->spawn_item == Quest_Dungeon::Item_GiveSpawned)
							{
								assert(spawned);
								if(spawned)
								{
									spawned->AddItem(event->item_to_give[0], 1, true);
									DEBUG_LOG(Format("Item %s given to %s (%g,%g).", event->item_to_give[0]->id.c_str(), spawned->data->id.c_str(), spawned->pos.x, spawned->pos.z));
								}
							}
							else if(event->spawn_item == Quest_Dungeon::Item_OnGround)
							{
								GroundItem* item = SpawnGroundItemInsideRadius(event->item_to_give[0], VEC2(128,128), 10.f);
								terrain->SetH(item->pos);
								DEBUG_LOG(Format("Generated item %s on ground (%g,%g).", event->item_to_give[0]->id.c_str(), item->pos.x, item->pos.z));
							}
						}
						location_event_handler = event->location_event_handler;
					}
				}

				// generuj minimap�
				LoadingStep(txGeneratingMinimap);
				CreateForestMinimap();

				// dodaj gracza i jego dru�yn�
				AddPlayerTeam(pos, dir, reenter, true);
			}
		}
		break;
	case L_ENCOUNTER:
		{
			// ustaw wska�niki
			OutsideLocation* enc = (OutsideLocation*)location;
			city_ctx = NULL;
			ApplyContext(enc, local_ctx);

			// ustaw teren
			ApplyTiles(enc->h, enc->tiles);

			SetOutsideParams();

			// generuj obiekty
			LoadingStep(txGeneratingObjects);
			SpawnEncounterObjects();

			// stw�rz obiekt kolizji terenu
			LoadingStep(txRecreatingObjects);
			SpawnTerrainCollider();
			SpawnOutsideBariers();

			// generuj jednostki
			LoadingStep(txGeneratingUnits);
			SpawnEncounterUnits();

			// generuj minimap�
			LoadingStep(txGeneratingMinimap);
			CreateForestMinimap();

			// dodaj gracza i jego dru�yn�
			SpawnEncounterTeam();
		}
		break;
	case L_CAMP:
		{
			// ustaw wska�niki
			OutsideLocation* camp = (OutsideLocation*)location;
			city_ctx = NULL;
			if(!reenter)
				ApplyContext(camp, local_ctx);

			int days;
			camp->CheckUpdate(days, worldtime);

			// ustaw teren
			if(!reenter)
				ApplyTiles(camp->h, camp->tiles);

			VEC3 pos;
			float dir;
			GetOutsideSpawnPoint(pos, dir);

			SetOutsideParams();

			// czy to pierwsza wizyta?
			if(first)
			{
				// generuj obiekty
				LoadingStep(txGeneratingObjects);
				SpawnCampObjects();

				// generuj jednostki
				LoadingStep(txGeneratingUnits);
				SpawnCampUnits();

				ResetCollisionPointers();
			}
			else if(!reenter)
			{
				if(days > 0)
					UpdateLocation(days, 100, false);

				// odtw�rz jednostki
				LoadingStep(txGeneratingUnits);
				RespawnUnits();

				// odtw�rz fizyk�
				LoadingStep(txGeneratingPhysics);
				RespawnObjectColliders();

				OnReenterLevel(local_ctx);
			}

			// stw�rz obiekt kolizji terenu
			if(!reenter)
			{
				LoadingStep(txRecreatingObjects);
				SpawnTerrainCollider();
				SpawnOutsideBariers();
			}

			// questowe rzeczy
			if(camp->active_quest && camp->active_quest != (Quest_Dungeon*)ACTIVE_QUEST_HOLDER)
			{
				Quest_Event* event = camp->active_quest->GetEvent(current_location);
				if(event)
				{
					if(!event->done && dungeon_level == event->at_level)
					{
						event->done = true;
						Unit* spawned = NULL;
						if(event->unit_to_spawn)
						{
							spawned = SpawnUnitNearLocation(local_ctx, random(VEC3(90,0,90), VEC3(256-90,0,256-90)), *event->unit_to_spawn, NULL, event->unit_spawn_level);
							assert(spawned);
							spawned->dont_attack = event->unit_dont_attack;
							DEBUG_LOG(Format("Generated unit %s (%g,%g).", event->unit_to_spawn->id.c_str(), spawned->pos.x, spawned->pos.z));
						}
						if(event->spawn_item == Quest_Dungeon::Item_GiveStrongest)
						{
							Unit* best = NULL;
							for(vector<Unit*>::iterator it = local_ctx.units->begin(), end = local_ctx.units->end(); it != end; ++it)
							{
								if((*it)->IsAlive() && IsEnemy(**it, *pc->unit) && (!best || (*it)->level > best->level))
									best = *it;
							}
							assert(best);
							if(best)
							{
								best->AddItem(event->item_to_give[0], 1, true);
								DEBUG_LOG(Format("Item %s given to %s (%g,%g).", event->item_to_give[0]->id.c_str(), best->data->id.c_str(), best->pos.x, best->pos.z));
							}
						}
						else if(event->spawn_item == Quest_Dungeon::Item_GiveSpawned)
						{
							assert(spawned);
							if(spawned)
							{
								spawned->AddItem(event->item_to_give[0], 1, true);
								DEBUG_LOG(Format("Item %s given to %s (%g,%g).", event->item_to_give[0]->id.c_str(), spawned->data->id.c_str(), spawned->pos.x, spawned->pos.z));
							}
						}
						else if(event->spawn_item == Quest_Dungeon::Item_OnGround)
						{
							GroundItem* item = SpawnGroundItemInsideRadius(event->item_to_give[0], VEC2(128,128), 20.f);
							DEBUG_LOG(Format("Item %s spawned at ground (%g,%g).", event->item_to_give[0]->id.c_str(), item->pos.x, item->pos.z));
						}
					}
					location_event_handler = event->location_event_handler;
				}
			}

			// generuj minimap�
			LoadingStep(txGeneratingMinimap);
			CreateForestMinimap();

			// dodaj gracza i jego dru�yn�
			AddPlayerTeam(pos, dir, reenter, true);

			// generate guards for bandits quest
			if(quest_bandits->bandits_state == Quest_Bandits::State::GenerateGuards && current_location == quest_bandits->target_loc)
			{
				quest_bandits->bandits_state = Quest_Bandits::State::GeneratedGuards;
				UnitData* ud = FindUnitData("guard_q_bandyci");
				int ile = random(4,5);
				pos += VEC3(sin(dir+PI)*8,0,cos(dir+PI)*8);
				for(int i=0; i<ile; ++i)
				{
					Unit* u = SpawnUnitNearLocation(local_ctx, pos, *ud, &leader->pos, 6, 4.f);
					u->assist = true;
				}
			}
		}
		break;
	case L_MOONWELL:
		{
			// ustaw wska�niki
			OutsideLocation* forest = (OutsideLocation*)location;
			city_ctx = NULL;
			if(!reenter)
				ApplyContext(forest, local_ctx);

			int days;
			bool need_reset = forest->CheckUpdate(days, worldtime);

			// ustaw teren
			if(!reenter)
				ApplyTiles(forest->h, forest->tiles);

			VEC3 pos;
			float dir;
			GetOutsideSpawnPoint(pos, dir);
			SetOutsideParams();

			// czy to pierwsza wizyta?
			if(first)
			{
				// generuj obiekty
				LoadingStep(txGeneratingObjects);
				SpawnMoonwellObjects();
				// generuj jednostki
				LoadingStep(txGeneratingUnits);
				SpawnMoonwellUnits(pos);
			}
			else if(!reenter)
			{
				if(days > 0)
					UpdateLocation(days, 100, false);

				if(need_reset)
				{
					// usu� �ywe jednostki
					for(vector<Unit*>::iterator it = local_ctx.units->begin(), end = local_ctx.units->end(); it != end; ++it)
					{
						if((*it)->IsAlive())
						{
							delete *it;
							*it = NULL;
						}
					}
					local_ctx.units->erase(std::remove_if(local_ctx.units->begin(), local_ctx.units->end(), is_null<Unit*>), local_ctx.units->end());
				}

				// odtw�rz jednostki
				LoadingStep(txGeneratingUnits);
				RespawnUnits();

				// odtw�rz fizyk�
				LoadingStep(txGeneratingPhysics);
				RespawnObjectColliders();

				if(need_reset)
				{
					// nowe jednostki
					SpawnMoonwellUnits(pos);
				}

				OnReenterLevel(local_ctx);
			}

			// stw�rz obiekt kolizji terenu
			if(!reenter)
			{
				LoadingStep(txRecreatingObjects);
				SpawnTerrainCollider();
				SpawnOutsideBariers();
			}

			// generuj minimap�
			LoadingStep(txGeneratingMinimap);
			CreateForestMinimap();

			// dodaj gracza i jego dru�yn�
			AddPlayerTeam(pos, dir, reenter, true);
		}
		break;
	default:
		assert(0);
		break;
	}

	SetTerrainTextures();

	LoadingStep(txLoadingComplete);
	l.last_visit = worldtime;
	CheckIfLocationCleared();
	local_ctx_valid = true;
	cam.Reset();
	player_rot_buf = 0.f;
	SetMusic();

	if(close_portal)
	{
		delete location->portal;
		location->portal = NULL;
	}

	if(IsOnline())
	{
		net_mode = NM_SERVER_SEND;
		net_state = 0;
		if(players > 1)
		{
			PrepareLevelData(net_stream);
			LOG(Format("Generated location packet: %d.", net_stream.GetNumberOfBytesUsed()));
		}
		else
			game_players[0].state = PlayerInfo::IN_GAME;

		info_box->Show(txWaitingForPlayers);
	}
	else
	{
		clear_color = clear_color2;
		game_state = GS_LEVEL;
		load_screen->visible = false;
		main_menu->visible = false;
		game_gui->visible = true;
	}

	if(location->outside)
		OnEnterLocation();

	OnEnterLevelOrLocation();

#ifdef IS_DEV
	ValidateTeamItems();
#endif

	LOG(Format("Randomness integrity: %d", rand2_tmp()));
	LOG("Entered location.");

	return true;
}

void Game::ApplyTiles(float* _h, TerrainTile* _tiles)
{
	assert(_h && _tiles);

	const uint _s = 16 * 8;
	
	TEX splat = terrain->GetSplatTexture();
	D3DLOCKED_RECT lock;
	V( splat->LockRect(0, &lock, NULL, 0) );
	byte* bits = (byte*)lock.pBits;
	for(uint y=0; y<256; ++y)
	{
		DWORD* row = (DWORD*)(bits + lock.Pitch * y);
		for(uint x=0; x<256; ++x, ++row)
		{
			TerrainTile& t = _tiles[x/2+y/2*_s];
			if(t.alpha == 0)
				*row = terrain_tile_info[t.t].mask;
			else
			{
				const TerrainTileInfo& tti1 = terrain_tile_info[t.t];
				const TerrainTileInfo& tti2 = terrain_tile_info[t.t2];
				if(tti1.shift > tti2.shift)
					*row = tti2.mask + ((255-t.alpha) << tti1.shift);
				else
					*row = tti1.mask + (t.alpha << tti2.shift);
			}
		}
	}
	V( splat->UnlockRect(0) );
	splat->GenerateMipSubLevels();

	terrain->SetHeightMap(_h);
	terrain->Rebuild();
	terrain->CalculateBox();
}

Object* Game::SpawnObject(LevelContext& ctx, Obj* obj, const VEC3& pos, float rot, float scale, VEC3* out_point, int variant)
{
	int obj_id, obj_t;

	if(IS_SET(obj->flagi, OBJ_STOL))
	{
		Obj* stol = FindObject(rand2()%2 == 0 ? "table" : "table2");

		// st�
		{
			Object& o = Add1(ctx.objects);
			o.mesh = stol->ani;
			o.rot = VEC3(0,rot,0);
			o.pos = pos;
			o.scale = 1;
			o.base = stol;
			obj_id = ctx.objects->size()-1;
			obj_t = 0;

			SpawnObjectExtras(ctx, stol, pos, rot, &o, NULL);
		}

		// sto�ki
		Obj* stolek = FindObject("stool");
		int ile = random(2, 4);
		int d[4] = {0,1,2,3};
		for(int i=0; i<4; ++i)
			std::swap(d[rand2()%4], d[rand2()%4]);

		for(int i=0; i<ile; ++i)
		{
			float sdir, slen;
			switch(d[i])
			{
			case 0:
				sdir = 0.f;
				slen = stol->size.y+0.3f;
				break;
			case 1:
				sdir = PI/2;
				slen = stol->size.x+0.3f;
				break;
			case 2:
				sdir = PI;
				slen = stol->size.y+0.3f;
				break;
			case 3:
				sdir = PI*3/2;
				slen = stol->size.x+0.3f;
				break;
			default:
				assert(0);
				break;
			}

			sdir += rot;

			Useable* u = new Useable;
			ctx.useables->push_back(u);
			u->type = U_STOLEK;
			u->pos = pos + VEC3(sin(sdir)*slen, 0, cos(sdir)*slen);
			u->rot = sdir;
			u->user = NULL;
			if(IsOnline())
				u->netid = useable_netid_counter++;

			SpawnObjectExtras(ctx, stolek, u->pos, u->rot, u, NULL);
		}
	}
	else if(IS_SET(obj->flagi, OBJ_BUDYNEK))
	{
		int roti;
		if(equal(rot, 0))
			roti = 0;
		else if(equal(rot, PI/2))
			roti = 1;
		else if(equal(rot, PI))
			roti = 2;
		else if(equal(rot, PI*3/2))
			roti = 3;
		else
		{
			assert(0);
			roti = 0;
			rot = 0.f;
		}

		Object& o = Add1(ctx.objects);
		o.mesh = obj->ani;
		o.rot = VEC3(0,rot,0);
		o.pos = pos;
		o.scale = scale;
		o.base = obj;
		obj_id = ctx.objects->size()-1;
		obj_t = 0;

		ProcessBuildingObjects(ctx, NULL, NULL, o.mesh, NULL, rot, roti, pos, B_NONE, NULL, false, out_point);
	}
	else
	{
		void* obj_ptr = NULL;
		void** result_ptr = NULL;

		if(IS_SET(obj->flagi, OBJ_UZYWALNY))
		{
			int typ;
			if(IS_SET(obj->flagi, OBJ_LAWA))
				typ = U_LAWA;
			else if(IS_SET(obj->flagi, OBJ_KOWADLO))
				typ = U_KOWADLO;
			else if(IS_SET(obj->flagi, OBJ_KRZESLO))
				typ = U_KRZESLO;
			else if(IS_SET(obj->flagi, OBJ_KOCIOLEK))
				typ = U_KOCIOLEK;
			else if(IS_SET(obj->flagi, OBJ_ZYLA_ZELAZA))
				typ = U_ZYLA_ZELAZA;
			else if(IS_SET(obj->flagi, OBJ_ZYLA_ZLOTA))
				typ = U_ZYLA_ZLOTA;
			else if(IS_SET(obj->flagi, OBJ_TRON))
				typ = U_TRON;
			else if(IS_SET(obj->flagi, OBJ_STOLEK))
				typ = U_STOLEK;
			else if(IS_SET(obj->flagi2, OBJ2_LAWA_DIR))
				typ = U_LAWA_DIR;
			else
			{
				assert(0);
				typ = U_KRZESLO;
			}

			Useable* u = new Useable;
			u->type = typ;
			u->pos = pos;
			u->rot = rot;
			u->user = NULL;
			if(variant == -1)
			{
				Obj* base_obj = u->GetBase()->obj;
				if(IS_SET(base_obj->flagi2, OBJ2_VARIANT))
				{
					// extra code for bench
					if(typ == U_LAWA || typ == U_LAWA_DIR)
					{
						switch(location->type)
						{
						case L_CITY:
						case L_VILLAGE:
							variant = 0;
							break;
						case L_DUNGEON:
						case L_CRYPT:
							variant = rand2()%2;
							break;
						default:
							variant = rand2()%2+2;
							break;
						}
					}
					else
						variant = random<int>(base_obj->variant->count-1);
				}
			}
			u->variant = variant;
			obj_ptr = u;

			if(IsOnline())
				u->netid = useable_netid_counter++;

			obj_id = ctx.useables->size();
			ctx.useables->push_back(u);
			obj_t = 1;
		}
		else if(IS_SET(obj->flagi, OBJ_SKRZYNIA))
		{
			Chest* chest = new Chest;
			chest->ani = new AnimeshInstance(obj->ani);
			chest->rot = rot;
			chest->pos = pos;
			chest->handler = NULL;
			chest->looted = false;
			obj_id = ctx.chests->size();
			obj_t = 2;
			ctx.chests->push_back(chest);
			if(IsOnline())
				chest->netid = chest_netid_counter++;
		}
		else
		{
			Object& o = Add1(ctx.objects);
			o.mesh = obj->ani;
			o.rot = VEC3(0,rot,0);
			o.pos = pos;
			o.scale = scale;
			o.base = obj;
			obj_id = ctx.objects->size()-1;
			obj_t = 0;
			obj_ptr = &o;
			result_ptr = &o.ptr;
		}

		SpawnObjectExtras(ctx, obj, pos, rot, obj_ptr, (btCollisionObject**)result_ptr, scale);
	}

	if(obj_t == 0)
		return &(*ctx.objects)[obj_id];
	else if(obj_t == 1)
		return (Object*)((*ctx.useables)[obj_id]); // meh
	else
		return (Object*)((*ctx.chests)[obj_id]); // meh^2
}

void Game::SpawnBuildings(vector<CityBuilding>& _buildings)
{
	City* city = (City*)location;

	const uint _s = 16 * 8;
	const int mur1 = int(0.15f*_s);
	const int mur2 = int(0.85f*_s);

	// budynki
	for(vector<CityBuilding>::iterator it = _buildings.begin(), end = _buildings.end(); it != end; ++it)
	{
		Object& o = Add1(local_ctx.objects);

		switch(it->rot)
		{
		case 0:
			o.rot.y = 0.f;
			break;
		case 1:
			o.rot.y = PI*3/2;
			break;
		case 2:
			o.rot.y = PI;
			break;
		case 3:
			o.rot.y = PI/2;
			break;
		}

		o.pos = VEC3(float(it->pt.x+buildings[it->type].shift[it->rot].x)*2, 1.f, float(it->pt.y+buildings[it->type].shift[it->rot].y)*2);
		terrain->SetH(o.pos);
		o.rot.x = o.rot.z = 0.f;
		o.scale = 1.f;
		o.base = NULL;
		o.mesh = buildings[it->type].mesh;
	}

	// create walls, towers & gates
	if(location->type != L_VILLAGE)
	{
		const int mid = int(0.5f*_s);
		Obj* oWall = FindObject("wall"),
		   * oTower = FindObject("tower"),
		   * oGate = FindObject("gate"),
		   * oGrate = FindObject("grate");

		// walls
		for(int i=mur1; i<mur2; i += 3)
		{
			// north
			if(!IS_SET(city->gates, GATE_SOUTH) || i < mid-1 || i > mid)
			{
				Object& o = Add1(local_ctx.objects);
				o.pos = VEC3(float(i)*2+1.f, 1.f, int(0.15f*_s)*2+1.f);
				o.rot = VEC3(0,PI,0);
				o.scale = 1.f;
				o.base = oWall;
				o.mesh = oWall->ani;
				SpawnObjectExtras(local_ctx, o.base, o.pos, o.rot.y, NULL, NULL, 1.f, 0);
			}

			// south
			if(!IS_SET(city->gates, GATE_NORTH) || i < mid-1 || i > mid)
			{
				Object& o = Add1(local_ctx.objects);
				o.pos = VEC3(float(i)*2+1.f, 1.f, int(0.85f*_s)*2+1.f);
				o.rot = VEC3(0,0,0);
				o.scale = 1.f;
				o.base = oWall;
				o.mesh = oWall->ani;
				SpawnObjectExtras(local_ctx, o.base, o.pos, o.rot.y, NULL, NULL, 1.f, 0);
			}

			// west
			if(!IS_SET(city->gates, GATE_WEST) || i < mid-1 || i > mid)
			{
				Object& o = Add1(local_ctx.objects);
				o.pos = VEC3(int(0.15f*_s)*2+1.f, 1.f, float(i)*2+1.f);
				o.rot = VEC3(0,PI*3/2,0);
				o.scale = 1.f;
				o.base = oWall;
				o.mesh = oWall->ani;
				SpawnObjectExtras(local_ctx, o.base, o.pos, o.rot.y, NULL, NULL, 1.f, 0);
			}

			// east
			if(!IS_SET(city->gates, GATE_EAST) || i < mid-1 || i > mid)
			{
				Object& o = Add1(local_ctx.objects);
				o.pos = VEC3(int(0.85f*_s)*2+1.f, 1.f, float(i)*2+1.f);
				o.rot = VEC3(0,PI/2,0);
				o.scale = 1.f;
				o.base = oWall;
				o.mesh = oWall->ani;
				SpawnObjectExtras(local_ctx, o.base, o.pos, o.rot.y, NULL, NULL, 1.f, 0);
			}
		}

		// towers
		{
			// north east
			Object& o = Add1(local_ctx.objects);
			o.pos = VEC3(int(0.85f*_s)*2+1.f,1.f,int(0.85f*_s)*2+1.f);
			o.rot = VEC3(0,0,0);		
			o.scale = 1.f;
			o.base = oTower;
			o.mesh = oTower->ani;
			SpawnObjectExtras(local_ctx, o.base, o.pos, o.rot.y, NULL, NULL, 1.f, 0);
		}
		{
			// south east
			Object& o = Add1(local_ctx.objects);
			o.pos = VEC3(int(0.85f*_s)*2+1.f,1.f,int(0.15f*_s)*2+1.f);
			o.rot = VEC3(0,PI/2,0);
			o.scale = 1.f;
			o.base = oTower;
			o.mesh = oTower->ani;
			SpawnObjectExtras(local_ctx, o.base, o.pos, o.rot.y, NULL, NULL, 1.f, 0);
		}
		{
			// south west
			Object& o = Add1(local_ctx.objects);
			o.pos = VEC3(int(0.15f*_s)*2+1.f,1.f,int(0.15f*_s)*2+1.f);
			o.rot = VEC3(0,PI,0);
			o.scale = 1.f;
			o.base = oTower;
			o.mesh = oTower->ani;
			SpawnObjectExtras(local_ctx, o.base, o.pos, o.rot.y, NULL, NULL, 1.f, 0);
		}
		{
			// north west
			Object& o = Add1(local_ctx.objects);
			o.pos = VEC3(int(0.15f*_s)*2+1.f,1.f,int(0.85f*_s)*2+1.f);
			o.rot = VEC3(0,PI*3/2,0);
			o.scale = 1.f;
			o.base = oTower;
			o.mesh = oTower->ani;
			SpawnObjectExtras(local_ctx, o.base, o.pos, o.rot.y, NULL, NULL, 1.f, 0);
		}

		// gates
		if(IS_SET(city->gates, GATE_NORTH))
		{
			Object& o = Add1(local_ctx.objects);
			o.rot.x = o.rot.z = 0.f;
			o.scale = 1.f;
			o.base = oGate;
			o.mesh = oGate->ani;
			o.rot.y = 0;
			o.pos = VEC3(0.5f*_s*2+1.f,1.f,0.85f*_s*2);
			SpawnObjectExtras(local_ctx, o.base, o.pos, o.rot.y, NULL, NULL, 1.f, 0);

			Object& o2 = Add1(local_ctx.objects);
			o2.pos = o.pos;
			o2.rot = o.rot;
			o2.scale = 1.f;
			o2.base = oGrate;
			o2.mesh = oGrate->ani;
			SpawnObjectExtras(local_ctx, o2.base, o2.pos, o2.rot.y, NULL, NULL, 1.f, 0);
		}

		if(IS_SET(city->gates, GATE_SOUTH))
		{
			Object& o = Add1(local_ctx.objects);
			o.rot.x = o.rot.z = 0.f;
			o.scale = 1.f;
			o.base = oGate;
			o.mesh = oGate->ani;
			o.rot.y = PI;
			o.pos = VEC3(0.5f*_s*2+1.f,1.f,0.15f*_s*2);
			SpawnObjectExtras(local_ctx, o.base, o.pos, o.rot.y, NULL, NULL, 1.f, 0);

			Object& o2 = Add1(local_ctx.objects);
			o2.pos = o.pos;
			o2.rot = o.rot;
			o2.scale = 1.f;
			o2.base = oGrate;
			o2.mesh = oGrate->ani;
			SpawnObjectExtras(local_ctx, o2.base, o2.pos, o2.rot.y, NULL, NULL, 1.f, 0);
		}

		if(IS_SET(city->gates, GATE_WEST))
		{
			Object& o = Add1(local_ctx.objects);
			o.rot.x = o.rot.z = 0.f;
			o.scale = 1.f;
			o.base = oGate;
			o.mesh = oGate->ani;
			o.rot.y = PI*3/2;
			o.pos = VEC3(0.15f*_s*2,1.f,0.5f*_s*2+1.f);
			SpawnObjectExtras(local_ctx, o.base, o.pos, o.rot.y, NULL, NULL, 1.f, 0);

			Object& o2 = Add1(local_ctx.objects);
			o2.pos = o.pos;
			o2.rot = o.rot;
			o2.scale = 1.f;
			o2.base = oGrate;
			o2.mesh = oGrate->ani;
			SpawnObjectExtras(local_ctx, o2.base, o2.pos, o2.rot.y, NULL, NULL, 1.f, 0);
		}

		if(IS_SET(city->gates, GATE_EAST))
		{
			Object& o = Add1(local_ctx.objects);
			o.rot.x = o.rot.z = 0.f;
			o.scale = 1.f;
			o.base = oGate;
			o.mesh = oGate->ani;
			o.rot.y = PI/2;
			o.pos = VEC3(0.85f*_s*2,1.f,0.5f*_s*2+1.f);
			SpawnObjectExtras(local_ctx, o.base, o.pos, o.rot.y, NULL, NULL, 1.f, 0);

			Object& o2 = Add1(local_ctx.objects);
			o2.pos = o.pos;
			o2.rot = o.rot;
			o2.scale = 1.f;
			o2.base = oGrate;
			o2.mesh = oGrate->ani;
			SpawnObjectExtras(local_ctx, o2.base, o2.pos, o2.rot.y, NULL, NULL, 1.f, 0);
		}
	}

	// obiekty i fizyka budynk�w
	for(vector<CityBuilding>::iterator it = _buildings.begin(), end = _buildings.end(); it != end; ++it)
	{
		const Building& b = buildings[it->type];

		int r = it->rot;
		if(r == 1)
			r = 3;
		else if(r == 3)
			r = 1;

		ProcessBuildingObjects(local_ctx, city, NULL, b.mesh, b.inside_mesh, dir_to_rot(r), r, VEC3(float(it->pt.x+b.shift[it->rot].x)*2, 0.f, float(it->pt.y+b.shift[it->rot].y)*2), it->type, &*it);
	}
}

void Game::ProcessBuildingObjects(LevelContext& ctx, City* city, InsideBuilding* inside, Animesh* mesh, Animesh* inside_mesh, float rot, int roti, const VEC3& shift, BUILDING type,
								  CityBuilding* building, bool recreate, VEC3* out_point)
{
	if(mesh->attach_points.empty())
	{
		building->walk_pt = shift;
		assert(!inside);
		return;
	}

	// o_x_[!N!]nazwa_???
	// x (o - obiekt, r - obr�cony obiekt, p - fizyka, s - strefa, c - posta�, m - maska �wiat�a, d - detal wok� obiektu, l - limited rot object)
	// N - wariant (tylko obiekty)
	string token;
	MATRIX m1, m2;
	bool have_exit = false, have_spawn = false;
	bool is_inside = (inside != NULL);
	VEC3 spawn_point;
	static vector<const Animesh::Point*> details;

	for(vector<Animesh::Point>::const_iterator it2 = mesh->attach_points.begin(), end2 = mesh->attach_points.end(); it2 != end2; ++it2)
	{
		const Animesh::Point& pt = *it2;
		if(pt.name.length() < 5 || pt.name[0] != 'o')
			continue;

		bool ok = false;
		char c = pt.name[2];
		if(c == 'o' || c == 'r' || c == 'p' || c == 's' || c == 'c' || c == 'l')
		{
			uint poss = pt.name.find_first_of('_', 4);
			if(poss == string::npos)
			{
				assert(0);
				continue;
			}
			token = pt.name.substr(4, poss-4);
			for(uint k=0, len = token.length(); k < len; ++k)
			{
				if(token[k] == '#')
					token[k] = '_';
			}
		}
		else if(c == 'm')
		{
			// light mask
			// nothing to do here
		}
		else if(c == 'd')
		{
			if(!recreate)
				details.push_back(&pt);
			continue;
		}
		else
			continue;

		VEC3 pos(0,0,0);
		if(roti != 0)
		{
			D3DXMatrixRotationY(&m1, rot);
			D3DXMatrixMultiply(&m2, &pt.mat, &m1);
			D3DXVec3TransformCoord(&pos, &pos, &m2);
		}
		else
			D3DXVec3TransformCoord(&pos, &pos, &pt.mat);
		pos += shift;

		if(c == 'o' || c == 'r' || c == 'l')
		{
			// obiekt / obr�cony obiekt
			if(!recreate)
			{
				cstring name;
				int variant = -1;
				if(token[0] == '!')
				{
					// p�ki co tylko 0-9
					variant = int(token[1]-'0');
					assert(in_range(variant, 0, 9));
					assert(token[2] == '!');
					name = token.c_str()+3;
				}
				else
					name = token.c_str();

				Obj* obj = FindObjectTry(name);
				assert(obj);

				if(obj)
				{
					if(ctx.type == LevelContext::Outside)
						terrain->SetH(pos);
					float r;
					switch(c)
					{
					case 'o':
					default:
						r = random(MAX_ANGLE);
						break;
					case 'r':
						r = clip(pt.rot.y+rot);
						break;
					case 'l':
						r = PI/2*(rand2()%4);
						break;
					}
					SpawnObject(ctx, obj, pos, r, 1.f, NULL, variant);
				}
			}
		}
		else if(c == 'p')
		{
			// fizyka
			if(token == "circle" || token == "circlev")
			{
				CollisionObject& cobj = Add1(ctx.colliders);
				cobj.type = CollisionObject::SPHERE;
				cobj.radius = pt.size.x;
				cobj.pt.x = pos.x;
				cobj.pt.y = pos.z;
				cobj.ptr = (token == "circle" ? CAM_COLLIDER : NULL);

				if(ctx.type == LevelContext::Outside)
				{
					terrain->SetH(pos);
					pos.y += 2.f;
				}

				btCylinderShape* shape = new btCylinderShape(btVector3(pt.size.x, 4.f, pt.size.z));
				shapes.push_back(shape);
				btCollisionObject* co = new btCollisionObject;
				co->setCollisionShape(shape);
				co->setCollisionFlags(CG_WALL);
				co->getWorldTransform().setOrigin(ToVector3(pos));
				phy_world->addCollisionObject(co);
			}
			else if(token == "square" || token == "squarev"|| token == "squarevp")
			{
				CollisionObject& cobj = Add1(ctx.colliders);
				cobj.type = CollisionObject::RECTANGLE;
				cobj.pt.x = pos.x;
				cobj.pt.y = pos.z;
				cobj.w = pt.size.x;
				cobj.h = pt.size.z;
				cobj.ptr = (token == "square" ? CAM_COLLIDER : NULL);

				btBoxShape* shape;
				if(token != "squarevp")
				{
					shape = new btBoxShape(btVector3(pt.size.x, 16.f, pt.size.z));
					if(ctx.type == LevelContext::Outside)
					{
						terrain->SetH(pos);
						pos.y += 8.f;
					}
					else
						pos.y = 0.f;
				}
				else
				{
					shape = new btBoxShape(btVector3(pt.size.x, pt.size.y, pt.size.z));
					if(ctx.type == LevelContext::Outside)
						pos.y += terrain->GetH(pos);
				}
				shapes.push_back(shape);
				btCollisionObject* co = new btCollisionObject;
				co->setCollisionShape(shape);
				co->setCollisionFlags(CG_WALL);
				co->getWorldTransform().setOrigin(ToVector3(pos));
				phy_world->addCollisionObject(co);

				if(roti != 0)
				{
					cobj.type = CollisionObject::RECTANGLE_ROT;
					cobj.rot = rot;
					cobj.radius = sqrt(cobj.w*cobj.w+cobj.h*cobj.h);
					co->getWorldTransform().setRotation(btQuaternion(rot,0,0));
				}
			}
			else if(token == "squarecam")
			{
				if(ctx.type == LevelContext::Outside)
					pos.y += terrain->GetH(pos);

				btBoxShape* shape = new btBoxShape(btVector3(pt.size.x, pt.size.y, pt.size.z));				
				shapes.push_back(shape);
				btCollisionObject* co = new btCollisionObject;
				co->setCollisionShape(shape);
				co->setCollisionFlags(CG_WALL);
				co->getWorldTransform().setOrigin(ToVector3(pos));
				phy_world->addCollisionObject(co);
				if(roti != 0)
					co->getWorldTransform().setRotation(btQuaternion(rot,0,0));

				float w = pt.size.x, h = pt.size.z;
				if(roti == 1 || roti == 3)
					std::swap(w, h);

				CameraCollider& cc = Add1(cam_colliders);
				cc.box.v1.x = pos.x - w;
				cc.box.v2.x = pos.x + w;
				cc.box.v1.z = pos.z - h;
				cc.box.v2.z = pos.z + h;
				cc.box.v1.y = pos.y - pt.size.y;
				cc.box.v2.y = pos.y + pt.size.y;
			}
			else if(token == "xsphere")
			{
				inside->xsphere_pos = pos;
				inside->xsphere_radius = pt.size.x;
			}
			else
				assert(0);
		}
		else if(c == 's')
		{
			// strefa
			if(!recreate)
			{
				if(token == "enter")
				{
					assert(!inside);

					inside = new InsideBuilding;
					inside->level_shift = city->inside_offset;
					inside->offset = VEC2(512.f*city->inside_offset.x+256.f, 512.f*city->inside_offset.y+256.f);
					if(city->inside_offset.x > city->inside_offset.y)
					{
						--city->inside_offset.x;
						++city->inside_offset.y;
					}
					else
					{
						city->inside_offset.x += 2;
						city->inside_offset.y = 0;
					}
					float w, h;
					if(roti == 0 || roti == 2)
					{
						w = pt.size.x;
						h = pt.size.z;
					}
					else
					{
						w = pt.size.z;
						h = pt.size.x;
					}
					inside->enter_area.v1.x = pos.x - w;
					inside->enter_area.v1.y = pos.z - h;
					inside->enter_area.v2.x = pos.x + w;
					inside->enter_area.v2.y = pos.z + h;
					VEC2 mid = inside->enter_area.Midpoint();
					inside->enter_y = terrain->GetH(mid.x, mid.y)+0.1f;
					inside->type = type;
					inside->outside_rot = rot;
					inside->top = -1.f;
					inside->xsphere_radius = -1.f;
					ApplyContext(inside, inside->ctx);
					inside->ctx.building_id = (int)city->inside_buildings.size();

					city->inside_buildings.push_back(inside);

					assert(inside_mesh);

					if(inside_mesh)
					{
						VEC3 o_pos = VEC3(inside->offset.x, 0.f, inside->offset.y);

						Object& o = Add1(inside->ctx.objects);
						o.base = NULL;
						o.mesh = inside_mesh;
						o.pos = o_pos;
						o.rot = VEC3(0,0,0);
						o.scale = 1.f;
						o.require_split = true;

						// nie mo�na przekaza� o.pos bo funkcja doda nowe obiekty i ta referencja b�dzie nie wa�na
						ProcessBuildingObjects(inside->ctx, city, inside, inside_mesh, NULL, 0.f, 0, o_pos, B_NONE, NULL);
					}

					have_exit = true;
				}
				else if(token == "exit")
				{
					assert(inside);

					inside->exit_area.v1.x = pos.x - pt.size.x;
					inside->exit_area.v1.y = pos.z - pt.size.z;
					inside->exit_area.v2.x = pos.x + pt.size.x;
					inside->exit_area.v2.y = pos.z + pt.size.z;

					have_exit = true;
				}
				else if(token == "spawn")
				{
					if(is_inside)
						inside->inside_spawn = pos;
					else
					{
						spawn_point = pos;
						terrain->SetH(spawn_point);
					}

					have_spawn = true;
				}
				else if(token == "top")
				{
					assert(is_inside);
					inside->top = pos.y;
				}
				else if(token == "door" || token == "doorc" || token == "doorl" || token == "door2")
				{
					Door* door = new Door;
					door->pos = pos;
					door->rot = clip(pt.rot.y+rot);
					door->state = Door::Open;
					door->door2 = (token == "door2");
					door->ani = new AnimeshInstance(door->door2 ? aDrzwi2 : aDrzwi);
					door->ani->groups[0].speed = 2.f;
					door->phy = new btCollisionObject;
					door->phy->setCollisionShape(shape_door);
					door->locked = LOCK_NONE;
					door->netid = door_netid_counter++;
					
					btTransform& tr = door->phy->getWorldTransform();
					VEC3 pos = door->pos;
					pos.y += 1.319f;
					tr.setOrigin(ToVector3(pos));
					tr.setRotation(btQuaternion(door->rot, 0, 0));
					phy_world->addCollisionObject(door->phy);

					if(token != "door") // door2 are closed now, this is intended
					{
						door->state = Door::Closed;
						if(token == "doorl")
							door->locked = 1;
					}
					else
					{
						btVector3& pos = door->phy->getWorldTransform().getOrigin();
						pos.setY(pos.y() - 100.f);
						door->ani->SetToEnd(door->ani->ani->anims[0].name.c_str());
					}

					ctx.doors->push_back(door);
				}
				else if(token == "arena")
				{
					assert(inside);

					inside->arena1.v1.x = pos.x - pt.size.x;
					inside->arena1.v1.y = pos.z - pt.size.y;
					inside->arena1.v2.x = pos.x + pt.size.x;
					inside->arena1.v2.y = pos.z + pt.size.y;
				}
				else if(token == "arena2")
				{
					assert(inside);

					inside->arena2.v1.x = pos.x - pt.size.x;
					inside->arena2.v1.y = pos.z - pt.size.y;
					inside->arena2.v2.x = pos.x + pt.size.x;
					inside->arena2.v2.y = pos.z + pt.size.y;
				}
				else if(token == "viewer")
				{
					// ten punkt jest u�ywany w SpawnArenaViewers
				}
				else if(token == "point")
				{
					if(building)
					{
						building->walk_pt = pos;
						terrain->SetH(building->walk_pt);
					}
					else if(out_point)
						*out_point = pos;
				}
				else
					assert(0);
			}
		}
		else if(c == 'c')
		{
			if(!recreate)
			{
				UnitData* ud = FindUnitData(token.c_str(), false);
				assert(ud);
				if(ud)
				{
					Unit* u = SpawnUnitNearLocation(ctx, pos, *ud, NULL, -2);
					u->rot = clip(pt.rot.y+rot);
				}
			}
		}
		else if(c == 'm')
		{
			LightMask& mask = Add1(inside->masks);
			mask.size = VEC2(pt.size.x, pt.size.z);
			mask.pos = VEC2(pos.x, pos.z);
		}
	}

	if(!details.empty() && type != B_NONE) // inside location have type B_NONE
	{
		int c = rand2()%80 + details.size()*2, count;
		if(c < 10)
			count = 0;
		else if(c < 30)
			count = 1;
		else if(c < 60)
			count = 2;
		else if(c < 90)
			count = 3;
		else
			count = 4;
		if(count > 0)
		{
			std::random_shuffle(details.begin(), details.end(), myrand);
			while(count && !details.empty())
			{
				const Animesh::Point& pt = *details.back();
				details.pop_back();
				--count;

				uint poss = pt.name.find_first_of('_', 4);
				if(poss == string::npos)
				{
					assert(0);
					continue;
				}
				token = pt.name.substr(4, poss-4);
				for(uint k=0, len = token.length(); k < len; ++k)
				{
					if(token[k] == '#')
						token[k] = '_';
				}

				VEC3 pos(0,0,0);
				if(roti != 0)
				{
					D3DXMatrixRotationY(&m1, rot);
					D3DXMatrixMultiply(&m2, &pt.mat, &m1);
					D3DXVec3TransformCoord(&pos, &pos, &m2);
				}
				else
					D3DXVec3TransformCoord(&pos, &pos, &pt.mat);
				pos += shift;

				cstring name;
				int variant = -1;
				if(token[0] == '!')
				{
					// p�ki co tylko 0-9
					variant = int(token[1]-'0');
					assert(in_range(variant, 0, 9));
					assert(token[2] == '!');
					name = token.c_str()+3;
				}
				else
					name = token.c_str();

				Obj* obj = FindObjectTry(name);
				assert(obj);

				if(obj)
				{
					if(ctx.type == LevelContext::Outside)
						terrain->SetH(pos);
					SpawnObject(ctx, obj, pos, clip(pt.rot.y+rot), 1.f, NULL, variant);
				}
			}
		}
		details.clear();
	}

	if(!recreate)
	{
		if(is_inside || inside)
			assert(have_exit && have_spawn);

		if(!is_inside && inside)
			inside->outside_spawn = spawn_point;
	}

	if(inside)
		inside->ctx.masks = (!inside->masks.empty() ? &inside->masks : NULL);
}

void Game::SpawnUnits(City* _city)
{
	assert(_city);

	for(vector<CityBuilding>::iterator it = _city->buildings.begin(), end = _city->buildings.end(); it != end; ++it)
	{
		UnitData* ud;
		if(it->type == B_ALCHEMIST)
			ud = FindUnitData("alchemist");
		else if(it->type == B_BLACKSMITH)
			ud = FindUnitData("blacksmith");
		else if(it->type == B_MERCHANT)
			ud = FindUnitData("merchant");
		else if(it->type == B_BARRACKS)
			ud = FindUnitData("guard_captain");
		else if(it->type == B_TRAINING_GROUND)
			ud = FindUnitData("trainer");
		else if(it->type == B_ARENA)
			ud = FindUnitData("arena_master");
		else if(it->type == B_FOOD_SELLER)
			ud = FindUnitData("food_seller");
		else
			continue;

		Unit* u = CreateUnit(*ud, -2);

		switch(it->rot)
		{
		case 0:
			u->rot = 0.f;
			break;
		case 1:
			u->rot = PI*3/2;
			break;
		case 2:
			u->rot = PI;
			break;
		case 3:
			u->rot = PI/2;
			break;
		}

		u->pos = VEC3(float(it->unit_pt.x)*2+1, 0, float(it->unit_pt.y)*2+1);
		terrain->SetH(u->pos);
		UpdateUnitPhysics(u, u->pos);
		u->visual_pos = u->pos;

		if(it->type == B_ARENA)
			_city->arena_pos = u->pos;

		local_ctx.units->push_back(u);

		AIController* ai = new AIController;
		ai->Init(u);
		ais.push_back(ai);
	}

	// pijacy w karczmie
	UnitData* mieszkaniec;
	if(locations[current_location]->type == L_CITY)
		mieszkaniec = FindUnitData("citizen");
	else
		mieszkaniec = FindUnitData("villager");
	for(int i=0, ile = random(1, city_ctx->citizens/3); i<ile; ++i)
	{
		Unit* u = SpawnUnitInsideInn(*mieszkaniec, -2);
		if(u)
			u->rot = random(MAX_ANGLE);
	}

	// w�druj�cy mieszka�cy
	const uint _s = 16 * 8;
	const int a = int(0.15f*_s)+3;
	const int b = int(0.85f*_s)-3;

	for(int i=0; i<city_ctx->citizens; ++i)
	{
		for(int j=0; j<50; ++j)
		{
			INT2 pt(random(a,b), random(a,b));
			if(_city->tiles[pt(_s)].IsRoadOrPath())
			{
				SpawnUnitNearLocation(local_ctx, VEC3(2.f*pt.x+1,0,2.f*pt.y+1), *mieszkaniec, NULL, -2, 2.f);
				break;
			}
		}
	}

	// stra�nicy
	UnitData* guard = FindUnitData("guard_move");
	for(int i=0, ile = city_ctx->type == L_VILLAGE ? 3 : 6; i<ile; ++i)
	{
		for(int j=0; j<50; ++j)
		{
			INT2 pt(random(a,b), random(a,b));
			if(_city->tiles[pt(_s)].IsRoadOrPath())
			{
				SpawnUnitNearLocation(local_ctx, VEC3(2.f*pt.x+1,0,2.f*pt.y+1), *guard, NULL, -2, 2.f);
				break;
			}
		}
	}
}

void Game::RespawnUnits()
{
	RespawnUnits(local_ctx);
	if(city_ctx)
	{
		for(vector<InsideBuilding*>::iterator it = city_ctx->inside_buildings.begin(), end = city_ctx->inside_buildings.end(); it != end; ++it)
			RespawnUnits((*it)->ctx);
	}
}

void Game::RespawnUnits(LevelContext& ctx)
{
	for(vector<Unit*>::iterator it = ctx.units->begin(), end = ctx.units->end(); it != end; ++it)
	{
		Unit* u = *it;
		if(u->player)
			continue;

		// model
		u->action = A_NONE;
		u->talking = false;
		u->ani = new AnimeshInstance(u->data->ani ? (Animesh*)u->data->ani : aHumanBase);
		u->ani->ptr = u;
		if(u->IsAlive())
		{
			u->ani->Play(NAMES::ani_stand, PLAY_PRIO1, 0);
			u->animation = u->current_animation = ANI_STAND;
		}
		else
		{
			u->ani->Play(NAMES::ani_die, PLAY_PRIO1, 0);
			u->animation = u->current_animation = ANI_DIE;
		}
		u->ani->SetToEnd();
		if(u->human_data)
			u->human_data->ApplyScale(u->ani->ani);

		// fizyka
		btCapsuleShape* caps = new btCapsuleShape(u->GetUnitRadius(), max(MIN_H, u->GetUnitHeight()));
		u->cobj = new btCollisionObject;
		VEC3 pos = u->pos;
		pos.y += u->GetUnitHeight();
		btVector3 bpos(ToVector3(u->IsAlive() ? pos : VEC3(1000,1000,1000)));
		bpos.setY(u->pos.y + max(MIN_H, u->GetUnitHeight())/2);
		u->cobj->getWorldTransform().setOrigin(bpos);
		u->cobj->setCollisionShape(caps);
		u->cobj->setUserPointer(u);
		u->cobj->setCollisionFlags(CG_UNIT);
		phy_world->addCollisionObject(u->cobj);

		// ai
		AIController* ai = new AIController;
		ai->Init(u);
		ais.push_back(ai);
	}
}

void Game::GenerateStockItems()
{
	Location& loc = *locations[current_location];

	assert(loc.type == L_CITY || loc.type == L_VILLAGE);

	bool have_smith, have_alchemist;
	int price_limit, price_limit2, count_mod;

	if(loc.type == L_CITY)
	{
		have_smith = true;
		have_alchemist = true;
		price_limit = random(2000,2500);
		price_limit2 = 99999;
		count_mod = 0;
	}
	else
	{
		Village* vil = (Village*)&loc;
		have_smith = (vil->v_buildings[0] == B_BLACKSMITH || vil->v_buildings[1] == B_BLACKSMITH);
		have_alchemist = (vil->v_buildings[0] == B_ALCHEMIST || vil->v_buildings[1] == B_ALCHEMIST);
		price_limit = random(500,1000);
		price_limit2 = random(1250,2500);
		count_mod = -random(1,3);
	}

	// kupiec
	GenerateMerchantItems(chest_merchant, price_limit);

	const Item* item;
	bool is_city = (loc.type == L_CITY);

	// kowal
	if(have_smith)
	{
		chest_blacksmith.clear();
		for(int i=0, ile=random(12,18)+count_mod; i<ile; ++i)
		{
			switch(rand2()%4)
			{
			case 0: // bro�
				while((item = g_weapons[rand2() % g_weapons.size()])->value > price_limit2 || IS_SET(item->flags, ITEM_NOT_SHOP|ITEM_NOT_BLACKSMITH))
					;
				InsertItemBare(chest_blacksmith, item);
				break;
			case 1: // �uk
				while((item = g_bows[rand2() % g_bows.size()])->value > price_limit2 || IS_SET(item->flags, ITEM_NOT_SHOP|ITEM_NOT_BLACKSMITH))
					;
				InsertItemBare(chest_blacksmith, item);
				break;
			case 2: // tarcza
				while((item = g_shields[rand2() % g_shields.size()])->value > price_limit2 || IS_SET(item->flags, ITEM_NOT_SHOP|ITEM_NOT_BLACKSMITH))
					;
				InsertItemBare(chest_blacksmith, item);
				break;
			case 3: // pancerz
				while((item = g_armors[rand2() % g_armors.size()])->value > price_limit2 || IS_SET(item->flags, ITEM_NOT_SHOP|ITEM_NOT_BLACKSMITH))
					;
				InsertItemBare(chest_blacksmith, item);
				break;
			}
		}
		// basic equipment
		ParseStockScript(FindStockScript("blacksmith"), 5, is_city, chest_blacksmith);
		SortItems(chest_blacksmith);
	}

	// alchemik
	if(have_alchemist)
	{
		chest_alchemist.clear();
		for(int i=0, ile=random(8,12)+count_mod; i<ile; ++i)
		{
			while(IS_SET((item = g_consumeables[rand2() % g_consumeables.size()])->flags, ITEM_NOT_SHOP|ITEM_NOT_ALCHEMIST))
				;
			int ile2 = price_limit/item->value;
			InsertItemBare(chest_alchemist, item, random(3,6));
		}
		SortItems(chest_alchemist);
	}

	// barman
	chest_innkeeper.clear();
	ParseStockScript(FindStockScript("innkeeper"), 5, is_city, chest_innkeeper);
	const ItemList* lis2 = FindItemList("normal_food").lis;
	for(uint i=0, count=random(10,20)+count_mod; i<count; ++i)
		InsertItemBare(chest_innkeeper, lis2->Get());
	SortItems(chest_innkeeper);

	// sprzedawca jedzenia
	chest_food_seller.clear();
	const ItemList* lis = FindItemList("food_and_drink").lis;
	for(const Item* item : lis->items)
	{
		uint value = random(50,100);
		if(location->type == L_VILLAGE)
			value /= 2;
		InsertItemBare(chest_food_seller, item, value/item->value);
	}
	SortItems(chest_food_seller);
}

void Game::GenerateMerchantItems(vector<ItemSlot>& items, int price_limit)
{
	const Item* item;
	items.clear();
	InsertItemBare(items, FindItem("p_nreg"), random(5,10));
	InsertItemBare(items, FindItem("p_hp"), random(5,10));
	for(int i=0, ile=random(15,20); i<ile; ++i)
	{
		switch(rand2()%6)
		{
		case 0: // bro�
			while((item = g_weapons[rand2() % g_weapons.size()])->value > price_limit || IS_SET(item->flags, ITEM_NOT_SHOP|ITEM_NOT_MERCHANT))
				;
			InsertItemBare(items, item);
			break;
		case 1: // �uk
			while((item = g_bows[rand2() % g_bows.size()])->value > price_limit || IS_SET(item->flags, ITEM_NOT_SHOP|ITEM_NOT_MERCHANT))
				;
			InsertItemBare(items, item);
			break;
		case 2: // tarcza
			while((item = g_shields[rand2() % g_shields.size()])->value > price_limit || IS_SET(item->flags, ITEM_NOT_SHOP|ITEM_NOT_MERCHANT))
				;
			InsertItemBare(items, item);
			break;
		case 3: // pancerz
			while((item = g_armors[rand2() % g_armors.size()])->value > price_limit || IS_SET(item->flags, ITEM_NOT_SHOP|ITEM_NOT_MERCHANT))
				;
			InsertItemBare(items, item);
			break;
		case 4: // jadalne
			while((item = g_consumeables[rand2() % g_consumeables.size()])->value > price_limit/5 || IS_SET(item->flags, ITEM_NOT_SHOP|ITEM_NOT_MERCHANT))
				;
			InsertItemBare(items, item, random(2,5));
			break;
		case 5: // inne
			while((item = g_others[rand2() % g_others.size()])->value > price_limit || IS_SET(item->flags, ITEM_NOT_SHOP|ITEM_NOT_MERCHANT))
				;
			InsertItemBare(items, item);
			break;
		}
	}
	SortItems(items);
}

// dru�yna opu�ci�a lokacje
void Game::LeaveLocation(bool clear)
{
	if(clear)
	{
		if(open_location != -1)
			LeaveLevel(true);
		return;
	}

	if(open_location == -1)
		return;

	LOG("Leaving location.");

	pvp_response.ok = false;
	zawody_wygenerowano = false;

	if(IsLocal() && (quest_crazies->check_stone || (quest_crazies->crazies_state >= Quest_Crazies::State::PickedStone && quest_crazies->crazies_state < Quest_Crazies::State::End)))
		CheckCraziesStone();

	// zawody w piciu
	if(chlanie_stan >= 3)
	{
		for(vector<Unit*>::iterator it = chlanie_ludzie.begin(), end = chlanie_ludzie.end(); it != end; ++it)
		{
			Unit& u = **it;
			u.busy = Unit::Busy_No;
			u.look_target = NULL;
			u.event_handler = NULL;
		}

		InsideBuilding* inn = city_ctx->FindInn();
		Unit* innkeeper = inn->FindUnit(FindUnitData("innkeeper"));

		innkeeper->talking = false;
		innkeeper->ani->need_update = true;
		innkeeper->busy = Unit::Busy_No;
		chlanie_stan = 1;
		chlanie_ludzie.clear();
		AddNews(txContestNoWinner);
	}

	if(IsLocal())
	{
		// zawody
		if(zawody_stan != IS_BRAK)
			CleanTournament();
		// arena
		if(!arena_free)
			CleanArena();
	}

	// clear blood & bodies from orc base
	if(quest_orcs2->orcs_state == Quest_Orcs2::State::ClearDungeon && current_location == quest_orcs2->target_loc)
	{
		quest_orcs2->orcs_state = Quest_Orcs2::State::End;
		UpdateLocation(31, 100, false);
	}

	if(city_ctx && !exit_mode && IsLocal())
	{
		// opuszczanie miasta
		BuyTeamItems();
	}

	LeaveLevel();

	if(open_location != -1)
	{
		// usu� questowe postacie
		RemoveQuestUnits(true);

		for(vector<Unit*>::iterator it = to_remove.begin(), end = to_remove.end(); it != end; ++it)
		{
			Unit* unit = *it;
			RemoveElement(GetContext(*unit).units, unit);
			delete unit;
		}
		to_remove.clear();

		Location& loc = *locations[open_location];
		if(loc.type == L_ENCOUNTER)
		{
			OutsideLocation* outside = (OutsideLocation*)location;
			outside->bloods.clear();
			DeleteElements(outside->chests);
			DeleteElements(outside->items);
			outside->objects.clear();
			DeleteElements(outside->useables);
			for(vector<Unit*>::iterator it = outside->units.begin(), end = outside->units.end(); it != end; ++it)
				delete *it;
			outside->units.clear();
		}
	}
	
	if(atak_szalencow)
	{
		atak_szalencow = false;
		if(IsOnline())
			PushNetChange(NetChange::CHANGE_FLAGS);
	}

	if(!IsLocal())
		pc = NULL;
	else
	{
		// usu� tymczasowe bufy
		for(vector<Unit*>::iterator it = team.begin(), end = team.end(); it != end; ++it)
			(*it)->EndEffects();
	}

	open_location = -1;
	city_ctx = NULL;
}

void Game::GenerateDungeon(Location& _loc)
{
	assert(_loc.type == L_DUNGEON || _loc.type == L_CRYPT);

	InsideLocation* inside = (InsideLocation*)&_loc;
	BaseLocation& base = g_base_locations[inside->target];
	InsideLocationLevel& lvl = inside->GetLevelData();
	
	assert(!lvl.map);

	if(!IS_SET(base.options, BLO_LABIRYNTH))
	{
		OpcjeMapy opcje;
		opcje.korytarz_szansa = base.corridor_chance;
		opcje.w = opcje.h = base.size + base.size_lvl * dungeon_level;
		opcje.rozmiar_korytarz = base.corridor_size;
		opcje.rozmiar_pokoj = base.room_size;
		opcje.rooms = &lvl.rooms;
		opcje.polacz_korytarz = base.join_corridor;
		opcje.polacz_pokoj = base.join_room;
		opcje.ksztalt = (IS_SET(base.options, BLO_ROUND) ? OpcjeMapy::OKRAG : OpcjeMapy::PROSTOKAT);
		opcje.schody_gora = (inside->HaveUpStairs() ? OpcjeMapy::LOSOWO : OpcjeMapy::BRAK);
		opcje.schody_dol = (inside->HaveDownStairs() ? OpcjeMapy::LOSOWO : OpcjeMapy::BRAK);
		opcje.kraty_szansa = base.bars_chance;

		// ostatni poziom krypty
		if(inside->type == L_CRYPT && !inside->HaveDownStairs())
		{
			opcje.schody_gora = OpcjeMapy::NAJDALEJ;
			Room& r = Add1(lvl.rooms);
			r.target = POKOJ_CEL_SKARBIEC;
			r.corridor = false;
			r.size = INT2(7,7);
			r.pos.x = r.pos.y = (opcje.w-7)/2;
			inside->special_room = 0;
		}
		else if(inside->type == L_DUNGEON && (inside->target == THRONE_FORT || inside->target == THRONE_VAULT) && !inside->HaveDownStairs())
		{
			// sala tronowa
			opcje.schody_gora = OpcjeMapy::NAJDALEJ;
			Room& r = Add1(lvl.rooms);
			r.target = POKOJ_CEL_TRON;
			r.corridor = false;
			r.size = INT2(13, 7);
			r.pos.x = (opcje.w-13)/2;
			r.pos.y = (opcje.w-7)/2;
			inside->special_room = 0;
		}
		else if(current_location == sekret_gdzie && sekret_stan == SS2_WRZUCONO_KAMIEN && !inside->HaveDownStairs())
		{
			// sekret
			opcje.schody_gora = OpcjeMapy::NAJDALEJ;
			Room& r = Add1(lvl.rooms);
			r.target = POKOJ_CEL_PORTAL_STWORZ;
			r.corridor = false;
			r.size = INT2(7,7);
			r.pos.x = r.pos.y = (opcje.w-7)/2;
			inside->special_room = 0;
		}
		else if(current_location == quest_evil->target_loc && quest_evil->evil_state == Quest_Evil::State::GeneratedCleric)
		{
			// schody w krypcie 0 jak najdalej od �rodka
			opcje.schody_gora = OpcjeMapy::NAJDALEJ;
		}

		if(quest_orcs2->orcs_state == Quest_Orcs2::State::Accepted && current_location == quest_orcs->target_loc && dungeon_level == location->GetLastLevel())
		{
			opcje.stop = true;
			bool first = true;
			int proby = 0;
			vector<int> mozliwe_pokoje;

			do
			{
				if(!generuj_mape2(opcje, !first))
				{
					assert(0);
					throw Format("Failed to generate dungeon map (%d)!", opcje.blad);
				}

				first = false;

				// szukaj pokoju dla celi
				int index = 0;
				for(vector<Room>::iterator it = lvl.rooms.begin(), end = lvl.rooms.end(); it != end; ++it, ++index)
				{
					if(!it->corridor && it->target == POKOJ_CEL_BRAK && it->connected.size() == 1)
						mozliwe_pokoje.push_back(index);
				}

				if(mozliwe_pokoje.empty())
				{
					++proby;
					if(proby == 100)
						throw "Failed to generate special room in dungeon!";
					else
						continue;
				}

				int id = mozliwe_pokoje[rand2()%mozliwe_pokoje.size()];
				lvl.rooms[id].target = POKOJ_CEL_WIEZIENIE;
				// dodaj drzwi
				INT2 pt = pole_laczace(id, lvl.rooms[id].connected.front());
				Pole& p = opcje.mapa[pt.x+pt.y*opcje.w];
				p.type = DRZWI;
				p.flags |= Pole::F_SPECJALNE;

				if(!kontynuuj_generowanie_mapy(opcje))
				{
					assert(0);
					throw Format( "Failed to generate dungeon map 2 (%d)!", opcje.blad);
				}
			}
			while(false);
		}
		else
		{
			if(!generuj_mape2(opcje))
			{
				assert(0);
				throw Format("Failed to generate dungeon map (%d)!", opcje.blad);
			}
		}

		lvl.w = lvl.h = opcje.w;
		lvl.map = opcje.mapa;
		lvl.staircase_up = opcje.schody_gora_pozycja;
		lvl.staircase_up_dir = opcje.schody_gora_kierunek;
		lvl.staircase_down = opcje.schody_dol_pozycja;
		lvl.staircase_down_dir = opcje.schody_dol_kierunek;
		lvl.staircase_down_in_wall = opcje.schody_dol_w_scianie;

		// inna tekstura pokoju w krypcie
		if(inside->type == L_CRYPT && !inside->HaveDownStairs())
		{
			Room& r = lvl.rooms[0];
			for(int y=0; y<r.size.y; ++y)
			{
				for(int x=0; x<r.size.x; ++x)
					lvl.map[r.pos.x+x+(r.pos.y+y)*lvl.w].flags |= Pole::F_DRUGA_TEKSTURA;
			}
		}

		// portal
		if(inside->from_portal)
		{
powtorz:
			int id = rand2() % lvl.rooms.size();
			while(lvl.rooms[id].corridor)
				id = (id + 1) % lvl.rooms.size();

			Room& r = lvl.rooms[id];
			vector<std::pair<INT2, int> > good_pts;

			for(int y=1; y<r.size.y-1; ++y)
			{
				for(int x=1; x<r.size.x-1; ++x)
				{
					if(lvl.At(r.pos + INT2(x, y)).type == PUSTE)
					{
						// opcje:
						// ___ #__
						// _?_ #?_
						// ### ###
#define P(xx,yy) (lvl.At(r.pos+INT2(x+xx,y+yy)).type == PUSTE)
#define B(xx,yy) (lvl.At(r.pos+INT2(x+xx,y+yy)).type == SCIANA)
						
						int dir = -1;

						// __#
						// _?#
						// __#
						if(P(-1,0) && B(1,0) && B(1,-1) && B(1,1) && ((P(-1,-1) && P(0,-1)) || (P(-1,1) && P(0,1)) || (B(-1,-1) && B(0,-1) && B(-1,1) && B(0,1))))
						{
							dir = 1;
						}
						// #__
						// #?_
						// #__
						else if(P(1,0) && B(-1,0) && B(-1,1) && B(-1,-1) && ((P(0,-1) && P(1,-1)) || (P(0,1) && P(1,1)) || (B(0,-1) && B(1,-1) && B(0,1) && B(1,1))))
						{
							dir = 3;
						}
						// ### 
						// _?_
						// ___
						else if(P(0,1) && B(0,-1) && B(-1,-1) && B(1,-1) && ((P(-1,0) && P(-1,1)) || (P(1,0) && P(1,1)) || (B(-1,0) && B(-1,1) && B(1,0) && B(1,1))))
						{
							dir = 2;
						}
						// ___
						// _?_
						// ###
						else if(P(0,-1) && B(0,1) && B(-1,1) && B(1,1) && ((P(-1,0) && P(-1,-1)) || (P(1,0) && P(1,-1)) || (B(-1,0) && B(-1,-1) && B(1,0) && B(1,-1))))
						{
							dir = 0;
						}

						if(dir != -1)
							good_pts.push_back(std::pair<INT2, int>(r.pos+INT2(x,y), dir));
#undef P
#undef B
					}
				}
			}

			if(good_pts.empty())
				goto powtorz;

			std::pair<INT2, int>& pt = good_pts[rand2()%good_pts.size()];

			const VEC3 pos(2.f*pt.first.x+1,0,2.f*pt.first.y+1);
			float rot = clip(dir_to_rot(pt.second)+PI);

			inside->portal->pos = pos;
			inside->portal->rot = rot;

			lvl.GetRoom(pt.first)->target = POKOJ_CEL_PORTAL;
		}
	}
	else
	{
		INT2 pokoj_pos;
		generate_labirynth(lvl.map, INT2(base.size, base.size), base.room_size, lvl.staircase_up, lvl.staircase_up_dir, pokoj_pos, base.bars_chance);

		lvl.w = lvl.h = base.size;
		Room& r = Add1(lvl.rooms);
		r.corridor = false;
		r.target = 0;
		r.pos = pokoj_pos;
		r.size = base.room_size;
	}
}

void Game::GenerateDungeonObjects2()
{
	InsideLocation* inside = (InsideLocation*)location;
	InsideLocationLevel& lvl = inside->GetLevelData();
	BaseLocation& base = g_base_locations[inside->target];

	// schody w g�r�
	if(inside->HaveUpStairs())
	{
		Object& o = Add1(local_ctx.objects);
		o.mesh = aSchodyGora;
		o.pos = pt_to_pos(lvl.staircase_up);
		o.rot = VEC3(0, dir_to_rot(lvl.staircase_up_dir), 0);
		o.scale = 1;
		o.base = NULL;
	}
	else
		SpawnObject(local_ctx, FindObject("portal"), inside->portal->pos, inside->portal->rot);

	// schody w d�
	if(inside->HaveDownStairs())
	{
		Object& o = Add1(local_ctx.objects);
		o.mesh = (lvl.staircase_down_in_wall ? aSchodyDol2 : aSchodyDol);
		o.pos = pt_to_pos(lvl.staircase_down);
		o.rot = VEC3(0, dir_to_rot(lvl.staircase_down_dir), 0);
		o.scale = 1;
		o.base = NULL;
	}

	// kratki, drzwi
	for(int y=0; y<lvl.h; ++y)
	{
		for(int x=0; x<lvl.w; ++x)
		{
			POLE p = lvl.map[x + y*lvl.w].type;
			if(p == KRATKA || p == KRATKA_PODLOGA)
			{
				Object& o = Add1(local_ctx.objects);
				o.mesh = aKratka;
				o.rot = VEC3(0,0,0);
				o.pos = VEC3(float(x*2),0,float(y*2));
				o.scale = 1;
				o.base = NULL;
			}
			if(p == KRATKA || p == KRATKA_SUFIT)
			{
				Object& o = Add1(local_ctx.objects);
				o.mesh = aKratka;
				o.rot = VEC3(0,0,0);
				o.pos = VEC3(float(x*2),4,float(y*2));
				o.scale = 1;
				o.base = NULL;
			}
			if(p == DRZWI)
			{
				Object& o = Add1(local_ctx.objects);
				o.mesh = aNaDrzwi;
				if(IS_SET(lvl.map[x+y*lvl.w].flags, Pole::F_DRUGA_TEKSTURA))
					o.mesh = aNaDrzwi2;
				o.pos = VEC3(float(x*2)+1,0,float(y*2)+1);
				o.scale = 1;
				o.base = NULL;

				if(czy_blokuje2(lvl.map[x - 1 + y*lvl.w].type))
				{
					o.rot = VEC3(0,0,0);
					int mov = 0;
					if(lvl.rooms[lvl.map[x + (y - 1)*lvl.w].room].corridor)
						++mov;
					if(lvl.rooms[lvl.map[x + (y + 1)*lvl.w].room].corridor)
						--mov;
					if(mov == 1)
						o.pos.z += 0.8229f;
					else if(mov == -1)
						o.pos.z -= 0.8229f;
				}
				else
				{
					o.rot = VEC3(0,PI/2,0);
					int mov = 0;
					if(lvl.rooms[lvl.map[x - 1 + y*lvl.w].room].corridor)
						++mov;
					if(lvl.rooms[lvl.map[x + 1 + y*lvl.w].room].corridor)
						--mov;
					if(mov == 1)
						o.pos.x += 0.8229f;
					else if(mov == -1)
						o.pos.x -= 0.8229f;
				}

				if(rand2()%100 < base.door_chance || IS_SET(lvl.map[x+y*lvl.w].flags, Pole::F_SPECJALNE))
				{
					Door* door = new Door;
					local_ctx.doors->push_back(door);
					door->pt = INT2(x,y);
					door->pos = o.pos;
					door->rot = o.rot.y;
					door->state = Door::Closed;
					door->ani = new AnimeshInstance(aDrzwi);
					door->ani->groups[0].speed = 2.f;
					door->phy = new btCollisionObject;
					door->phy->setCollisionShape(shape_door);
					door->locked = LOCK_NONE;
					door->netid = door_netid_counter++;
					btTransform& tr = door->phy->getWorldTransform();
					VEC3 pos = door->pos;
					pos.y += 1.319f;
					tr.setOrigin(ToVector3(pos));
					tr.setRotation(btQuaternion(door->rot, 0, 0));
					phy_world->addCollisionObject(door->phy);

					if(IS_SET(lvl.map[x+y*lvl.w].flags, Pole::F_SPECJALNE))
						door->locked = LOCK_ORKOWIE;
					else if(rand2()%100 < base.door_open)
					{
						door->state = Door::Open;
						btVector3& pos = door->phy->getWorldTransform().getOrigin();
						pos.setY(pos.y() - 100.f);
						door->ani->SetToEnd(door->ani->ani->anims[0].name.c_str());
					}
				}
				else
					lvl.map[x+y*lvl.w].type = OTWOR_NA_DRZWI;
			}
		}
	}
}

void Game::SpawnCityPhysics()
{
	City* city = (City*)locations[current_location];
	TerrainTile* tiles = city->tiles;
	const uint _s = 16 * 8;

	for(int z=0; z<City::size; ++z)
	{
		for(int x=0; x<City::size; ++x)
		{
			if(tiles[x+z*_s].mode == TM_BUILDING_BLOCK)
			{
				btCollisionObject* cobj = new btCollisionObject;
				cobj->setCollisionShape(shape_block);
				cobj->getWorldTransform().setOrigin(btVector3(2.f*x+1.f,terrain->GetH(2.f*x+1.f,2.f*x+1),2.f*z+1.f));
				cobj->setCollisionFlags(CG_WALL);
				phy_world->addCollisionObject(cobj);
			}
		}
	}
}

void Game::RespawnBuildingPhysics()
{
	for(vector<CityBuilding>::iterator it = city_ctx->buildings.begin(), end = city_ctx->buildings.end(); it != end; ++it)
	{
		const Building& b = buildings[it->type];

		int r = it->rot;
		if(r == 1)
			r = 3;
		else if(r == 3)
			r = 1;

		ProcessBuildingObjects(local_ctx, city_ctx, NULL, b.mesh, NULL, dir_to_rot(r), r, VEC3(float(it->pt.x+b.shift[it->rot].x)*2, 1.f, float(it->pt.y+b.shift[it->rot].y)*2), B_NONE, &*it, true);
	}
	
	for(vector<InsideBuilding*>::iterator it = city_ctx->inside_buildings.begin(), end = city_ctx->inside_buildings.end(); it != end; ++it)
		ProcessBuildingObjects((*it)->ctx, city_ctx, *it, buildings[(*it)->type].inside_mesh, NULL, 0.f, 0, VEC3((*it)->offset.x, 0.f, (*it)->offset.y), B_NONE, NULL, true);
}

struct OutsideObject
{
	cstring name;
	Obj* obj;
	VEC2 scale;
};

OutsideObject outside_objects[] = {
	"tree", NULL, VEC2(3,5),
	"tree2", NULL, VEC2(3,5),
	"tree3", NULL, VEC2(3,5),
	"grass", NULL, VEC2(1.f,1.5f),
	"grass", NULL, VEC2(1.f,1.5f),
	"plant", NULL, VEC2(1.0f,2.f),
	"plant", NULL, VEC2(1.0f,2.f),
	"rock", NULL, VEC2(1.f,1.f),
	"fern", NULL, VEC2(1,2)
};
const uint n_outside_objects = countof(outside_objects);

void Game::SpawnCityObjects()
{
	if(!outside_objects[0].obj)
	{
		for(uint i=0; i<n_outside_objects; ++i)
			outside_objects[i].obj = FindObject(outside_objects[i].name);
	}

	// well
	if(g_have_well)
	{
		VEC3 pos = pt_to_pos(g_well_pt);
		terrain->SetH(pos);
		SpawnObject(local_ctx, FindObject("coveredwell"), pos, PI/2*(rand2()%4), 1.f, NULL);
	}

	TerrainTile* tiles = ((City*)location)->tiles;

	for(int i=0; i<512; ++i)
	{
		INT2 pt(random(1,OutsideLocation::size-2), random(1, OutsideLocation::size-2));
		if(tiles[pt.x+pt.y*OutsideLocation::size].mode == TM_NORMAL &&
			tiles[pt.x-1+pt.y*OutsideLocation::size].mode == TM_NORMAL &&
			tiles[pt.x+1+pt.y*OutsideLocation::size].mode == TM_NORMAL &&
			tiles[pt.x+(pt.y-1)*OutsideLocation::size].mode == TM_NORMAL &&
			tiles[pt.x+(pt.y+1)*OutsideLocation::size].mode == TM_NORMAL &&
			tiles[pt.x-1+(pt.y-1)*OutsideLocation::size].mode == TM_NORMAL &&
			tiles[pt.x-1+(pt.y+1)*OutsideLocation::size].mode == TM_NORMAL &&
			tiles[pt.x+1+(pt.y-1)*OutsideLocation::size].mode == TM_NORMAL &&
			tiles[pt.x+1+(pt.y+1)*OutsideLocation::size].mode == TM_NORMAL)
		{
			VEC3 pos(random(2.f)+2.f*pt.x,0,random(2.f)+2.f*pt.y);
			pos.y = terrain->GetH(pos);
			OutsideObject& o = outside_objects[rand2()%n_outside_objects];
			SpawnObject(local_ctx, o.obj, pos, random(MAX_ANGLE), random2(o.scale));
		}
	}
}

#include "perlin.h"

void Game::GenerateForest(Location& loc)
{
	OutsideLocation* outside = (OutsideLocation*)&loc;

	// stw�rz map� je�li jeszcze nie ma
	const int _s = OutsideLocation::size;
	outside->tiles = new TerrainTile[_s*_s];
	outside->h = new float[(_s+1)*(_s+1)];
	memset(outside->tiles, 0, sizeof(TerrainTile)*_s*_s);

	Perlin perlin2(4,4,1);

	for(int i=0, y=0; y<_s; ++y)
	{
		for(int x=0; x<_s; ++x, ++i)
		{
			int v = int((perlin2.Get(1.f/256*float(x), 1.f/256*float(y))+1.f)*50);
			TERRAIN_TILE& t = outside->tiles[i].t;
			if(v < 42)
				t = TT_GRASS;
			else if(v < 58)
				t = TT_GRASS2;
			else
				t = TT_GRASS3;
		}
	}

	LoadingStep(txGeneratingTerrain);

	Perlin perlin(4,4,1);

	// losuj teren
	terrain->SetHeightMap(outside->h);
	terrain->RandomizeHeight(0.f,5.f);
	float* h = terrain->GetHeightMap();

	for(uint y=0; y<_s; ++y)
	{
		for(uint x=0; x<_s; ++x)
		{
			if(x < 15 || x > _s-15 || y < 15 || y > _s-15)
				h[x+y*(_s+1)] += random(10.f,15.f);
		}
	}

	terrain->RoundHeight();
	terrain->RoundHeight();

	for(uint y=0; y<_s; ++y)
	{
		for(uint x=0; x<_s; ++x)
			h[x+y*(_s+1)] += (perlin.Get(1.f/256*x, 1.f/256*y)+1.f)*5;
	}

	terrain->RoundHeight();
	terrain->RemoveHeightMap();
}

OutsideObject trees[] = {
	"tree", NULL, VEC2(2,6),
	"tree2", NULL, VEC2(2,6),
	"tree3", NULL, VEC2(2,6)
};
const uint n_trees = countof(trees);

OutsideObject trees2[] = {
	"tree", NULL, VEC2(4,8),
	"tree2", NULL, VEC2(4,8),
	"tree3", NULL, VEC2(4,8),
	"withered_tree", NULL, VEC2(1,4)
};
const uint n_trees2 = countof(trees2);

OutsideObject misc[] = {
	"grass", NULL, VEC2(1.f,1.5f),
	"grass", NULL, VEC2(1.f,1.5f),
	"plant", NULL, VEC2(1.0f,2.f),
	"plant", NULL, VEC2(1.0f,2.f),
	"rock", NULL, VEC2(1.f,1.f),
	"fern", NULL, VEC2(1,2)
};
const uint n_misc = countof(misc);

void Game::SpawnForestObjects(int road_dir)
{
	if(!trees[0].obj)
	{
		for(uint i=0; i<n_trees; ++i)
			trees[i].obj = FindObject(trees[i].name);
		for(uint i=0; i<n_trees2; ++i)
			trees2[i].obj = FindObject(trees2[i].name);
		for(uint i=0; i<n_misc; ++i)
			misc[i].obj = FindObject(misc[i].name);
	}

	TerrainTile* tiles = ((OutsideLocation*)location)->tiles;

	// obelisk
	if(rand2()%(road_dir == -1 ? 10 : 15) == 0)
	{
		VEC3 pos;
		if(road_dir == -1)
			pos = VEC3(127.f,0,127.f);
		else if(road_dir == 0)
			pos = VEC3(127.f,0,rand2()%2 == 0 ? 127.f-32.f : 127.f+32.f);
		else
			pos = VEC3(rand2()%2 == 0 ? 127.f-32.f : 127.f+32.f,0,127.f);
		terrain->SetH(pos);
		pos.y -= 1.f;
		SpawnObject(local_ctx, FindObject("obelisk"), pos, 0.f);
	}
	else if(rand2()%16 == 0)
	{
		// tree with rocks around it
		VEC3 pos(random(48.f,208.f), 0, random(48.f,208.f));
		pos.y = terrain->GetH(pos)-1.f;
		SpawnObject(local_ctx, trees2[3].obj, pos, random(MAX_ANGLE), 4.f);
		for(int i=0; i<12; ++i)
		{
			VEC3 pos2 = pos + VEC3(sin(PI*2*i/12)*8.f, 0, cos(PI*2*i/12)*8.f);
			pos2.y = terrain->GetH(pos2);
			SpawnObject(local_ctx, misc[4].obj, pos2, random(MAX_ANGLE));
		}
	}

	// drzewa
	for(int i=0; i<1024; ++i)
	{
		INT2 pt(random(1,OutsideLocation::size-2), random(1, OutsideLocation::size-2));
		TERRAIN_TILE co = tiles[pt.x+pt.y*OutsideLocation::size].t;
		if(co == TT_GRASS)
		{
			VEC3 pos(random(2.f)+2.f*pt.x,0,random(2.f)+2.f*pt.y);
			pos.y = terrain->GetH(pos);
			OutsideObject& o = trees[rand2()%n_trees];
			SpawnObject(local_ctx, o.obj, pos, random(MAX_ANGLE), random2(o.scale));
		}
		else if(co == TT_GRASS3)
		{
			VEC3 pos(random(2.f)+2.f*pt.x,0,random(2.f)+2.f*pt.y);
			pos.y = terrain->GetH(pos);
			int co;
			if(rand2()%12 == 0)
				co = 3;
			else
				co = rand2()%3;
			OutsideObject& o = trees2[co];
			SpawnObject(local_ctx, o.obj, pos, random(MAX_ANGLE), random2(o.scale));
		}
	}

	// inne
	for(int i=0; i<512; ++i)
	{
		INT2 pt(random(1,OutsideLocation::size-2), random(1, OutsideLocation::size-2));
		if(tiles[pt.x+pt.y*OutsideLocation::size].t != TT_SAND)
		{
			VEC3 pos(random(2.f)+2.f*pt.x,0,random(2.f)+2.f*pt.y);
			pos.y = terrain->GetH(pos);
			OutsideObject& o = misc[rand2()%n_misc];
			SpawnObject(local_ctx, o.obj, pos, random(MAX_ANGLE), random2(o.scale));
		}
	}
}

void Game::GetOutsideSpawnPoint(VEC3& pos, float& dir)
{
	const float dist = 40.f;

	if(world_dir < PI/4 || world_dir > 7.f/4*PI)
	{
		// east
		dir = PI/2;
		if(world_dir < PI/4)
			pos = VEC3(256.f-dist,0,lerp(128.f,256.f-dist, world_dir/(PI/4)));
		else
			pos = VEC3(256.f-dist,0,lerp(dist,128.f,(world_dir-(7.f/4*PI))/(PI/4)));
	}
	else if(world_dir < 3.f/4*PI)
	{
		// north
		dir = 0;
		pos = VEC3(lerp(dist,256.f-dist,1.f-((world_dir-(1.f/4*PI))/(PI/2))),0,256.f-dist);
	}
	else if(world_dir < 5.f/4*PI)
	{
		// west
		dir = 3.f/2*PI;
		pos = VEC3(dist,0,lerp(dist,256.f-dist,1.f-((world_dir-(3.f/4*PI))/(PI/2))));
	}
	else
	{
		// south
		dir = PI;
		pos = VEC3(lerp(dist,256.f-dist,(world_dir-(5.f/4*PI))/(PI/2)),0,dist);
	}
}

struct TGroup
{
	vector<EnemyEntry*> enemies;
	int total;
};

void Game::SpawnForestUnits(const VEC3& team_pos)
{
	// zbierz grupy
	EnemyGroup* groups[4];
	groups[0] = &FindEnemyGroup("cave_wolfs");
	groups[1] = &FindEnemyGroup("cave_spiders");
	groups[2] = &FindEnemyGroup("cave_rats");
	groups[3] = &FindEnemyGroup("animals");
	UnitData* ud_hunter = FindUnitData("wild_hunter");

	const int level = GetDungeonLevel();
	static TGroup ee[4];
	static vector<VEC2> poss;
	OutsideLocation* outside = (OutsideLocation*)location;
	poss.push_back(VEC2(team_pos.x, team_pos.z));

	// ustal wrog�w
	for(int i=0; i<4; ++i)
	{
		ee[i].total = 0;
		for(int j=0; j<groups[i]->count; ++j)
		{
			if(groups[i]->enemies[j].unit->level.x <= level)
			{
				ee[i].total += groups[i]->enemies[j].count;
				ee[i].enemies.push_back(&groups[i]->enemies[j]);
			}
		}
	}

	for(int added=0, tries=50; added<8 && tries>0; --tries)
	{
		VEC2 pos = outside->GetRandomPos();

		bool ok = true;
		for(vector<VEC2>::iterator it = poss.begin(), end = poss.end(); it != end; ++it)
		{
			if(distance(pos, *it) < 24.f)
			{
				ok = false;
				break;
			}
		}

		if(ok)
		{
			// losuj grupe
			TGroup& group = ee[rand2()%4];
			if(group.enemies.empty())
				continue;

			poss.push_back(pos);
			++added;

			VEC3 pos3(pos.x, 0, pos.y);

			// postaw jednostki
			int levels = level * 2;
			if(rand2()%5 == 0 && ud_hunter->level.x <= level)
			{
				int enemy_level = random2(ud_hunter->level.x, min3(ud_hunter->level.y, levels, level));
				if(SpawnUnitNearLocation(local_ctx, pos3, *ud_hunter, NULL, enemy_level, 6.f))
					levels -= enemy_level;
			}
			while(levels > 0)
			{
				int k = rand2()%group.total, l = 0;
				UnitData* ud;

				DEBUG_DO(ud = NULL);

				for(vector<EnemyEntry*>::iterator it = group.enemies.begin(), end = group.enemies.end(); it != end; ++it)
				{
					l += (*it)->count;
					if(k < l)
					{
						ud = (*it)->unit;
						break;
					}
				}

				assert(ud);

				if(ud->level.x > levels)
					break;

				int enemy_level = random2(ud->level.x, min3(ud->level.y, levels, level));
				if(!SpawnUnitNearLocation(local_ctx, pos3, *ud, NULL, enemy_level, 6.f))
					break;
				levels -= enemy_level;
			}
		}
	}

	poss.clear();
	for(int i=0; i<4; ++i)
		ee[i].enemies.clear();
}

void Game::RepositionCityUnits()
{
	const uint _s = 16 * 8;
	const int a = int(0.15f*_s)+3;
	const int b = int(0.85f*_s)-3;

	UnitData* citizen;
	if(city_ctx->type == L_VILLAGE)
		citizen = FindUnitData("villager");
	else
		citizen = FindUnitData("citizen");
	UnitData* guard = FindUnitData("guard_move");
	InsideBuilding* inn = city_ctx->FindInn();

	for(vector<Unit*>::iterator it = local_ctx.units->begin(), end = local_ctx.units->end(); it != end; ++it)
	{
		Unit& u = **it;
		if(u.IsAlive() && u.IsAI())
		{
			if(u.ai->goto_inn)
				WarpToArea(inn->ctx, (rand2()%5 == 0 ? inn->arena2 : inn->arena1), u.GetUnitRadius(), u.pos);
			else if(u.data == citizen || u.data == guard)
			{
				for(int j=0; j<50; ++j)
				{
					INT2 pt(random(a,b), random(a,b));
					if(city_ctx->tiles[pt(_s)].IsRoadOrPath())
					{
						WarpUnit(u, VEC3(2.f*pt.x+1,0,2.f*pt.y+1));
						break;
					}
				}
			}
		}
	}
}

void Game::Event_RandomEncounter(int)
{
	world_state = WS_TRAVEL;
	dialog_enc = NULL;
	if(IsOnline())
		PushNetChange(NetChange::CLOSE_ENCOUNTER);
	current_location = encounter_loc;
	EnterLocation();
}

void Game::Event_StartEncounter(int)
{
	dialog_enc = NULL;
	PushNetChange(NetChange::CLOSE_ENCOUNTER);
}

void Game::GenerateEncounterMap(Location& loc)
{
	OutsideLocation* outside = (OutsideLocation*)&loc;

	// stw�rz map� je�li jeszcze nie ma
	const int _s = OutsideLocation::size;
	if(!outside->tiles)
	{
		outside->tiles = new TerrainTile[_s*_s];
		outside->h = new float[(_s+1)*(_s+1)];
		memset(outside->tiles, 0, sizeof(TerrainTile)*_s*_s);
	}

	// 0 - w prawo, 1 - w g�re, 2 - w lewo, 3 - w d�
	enc_kierunek = rand2()%4;

	Perlin perlin2(4,4,1);

	for(int i=0, y=0; y<_s; ++y)
	{
		for(int x=0; x<_s; ++x, ++i)
		{
			int v = int((perlin2.Get(1.f/256*float(x), 1.f/256*float(y))+1.f)*50);
			TERRAIN_TILE& t = outside->tiles[i].t;
			if(v < 42)
				t = TT_GRASS;
			else if(v < 58)
				t = TT_GRASS2;
			else
				t = TT_GRASS3;
		}
	}

	LoadingStep(txGeneratingTerrain);

	Perlin perlin(4,4,1);

	// losuj teren
	terrain->SetHeightMap(outside->h);
	terrain->RandomizeHeight(0.f,5.f);
	float* h = terrain->GetHeightMap();

	for(uint y=0; y<_s; ++y)
	{
		for(uint x=0; x<_s; ++x)
		{
			if(x < 15 || x > _s-15 || y < 15 || y > _s-15)
				h[x+y*(_s+1)] += random(10.f,15.f);
		}
	}

	if(enc_kierunek == 0 || enc_kierunek == 2)
	{
		for(int y=62; y<66; ++y)
		{
			for(int x=0; x<_s; ++x)
			{
				outside->tiles[x+y*_s].Set(TT_SAND, TM_ROAD);
				h[x+y*(_s+1)] = 1.f;
			}
		}
		for(int x=0; x<_s; ++x)
		{
			h[x+61*(_s+1)] = 1.f;
			h[x+66*(_s+1)] = 1.f;
			h[x+67*(_s+1)] = 1.f;
		}
	}
	else
	{
		for(int y=0; y<_s; ++y)
		{
			for(int x=62; x<66; ++x)
			{
				outside->tiles[x+y*_s].Set(TT_SAND, TM_ROAD);
				h[x+y*(_s+1)] = 1.f;
			}
		}
		for(int y=0; y<_s; ++y)
		{
			h[61+y*(_s+1)] = 1.f;
			h[66+y*(_s+1)] = 1.f;
			h[67+y*(_s+1)] = 1.f;
		}
	}

	terrain->RoundHeight();
	terrain->RoundHeight();

	for(uint y=0; y<_s; ++y)
	{
		for(uint x=0; x<_s; ++x)
			h[x+y*(_s+1)] += (perlin.Get(1.f/256*x, 1.f/256*y)+1.f)*4;
	}

	if(enc_kierunek == 0 || enc_kierunek == 2)
	{
		for(int y=61; y<=67; ++y)
		{
			for(int x=1; x<_s-1; ++x)
				h[x+y*(_s+1)] = (h[x+y*(_s+1)]+h[x+1+y*(_s+1)]+h[x-1+y*(_s+1)]+h[x+(y-1)*(_s+1)]+h[x+(y+1)*(_s+1)])/5;
		}
	}
	else
	{
		for(int y=1; y<_s-1; ++y)
		{
			for(int x=61; x<=67; ++x)
				h[x+y*(_s+1)] = (h[x+y*(_s+1)]+h[x+1+y*(_s+1)]+h[x-1+y*(_s+1)]+h[x+(y-1)*(_s+1)]+h[x+(y+1)*(_s+1)])/5;
		}
	}

	terrain->RoundHeight();
	terrain->RemoveHeightMap();
}

void Game::SpawnEncounterUnits()
{
	VEC3 look_pt;
	switch(enc_kierunek)
	{
	case 0:
		look_pt = VEC3(133.f,0.f,128.f);
		break;
	case 1:
		look_pt = VEC3(128.f,0.f,133.f);
		break;
	case 2:
		look_pt = VEC3(123.f,0.f,128.f);
		break;
	case 3:
		look_pt = VEC3(128.f,0.f,123.f);
		break;
	}

	UnitData* esencial = NULL;
	cstring group_name = NULL, group_name2 = NULL;
	bool dont_attack = false, od_tylu = false, kamien = false;
	int ile, poziom, ile2, poziom2;
	DialogEntry* rozmowa = NULL;
	Quest* quest = NULL;
	far_encounter = false;

	if(enc_tryb == 0)
	{
		switch(losowi_wrogowie)
		{
		case -1:
			if(rand2()%3 != 0)
				esencial = FindUnitData("wild_hunter");
			group_name = "animals";
			break;
		case SG_BANDYCI:
			group_name = "bandits";
			dont_attack = true;
			rozmowa = dialog_bandyci;
			break;
		case SG_GOBLINY:
			group_name = "goblins";
			break;
		case SG_ORKOWIE:
			group_name = "orcs";
			break;
		}

		ile = random(3,5);
		poziom = random(6,12);
	}
	else if(enc_tryb == 1)
	{
		switch(spotkanie)
		{
		case 0: // mag
			esencial = FindUnitData("crazy_mage");
			group_name = NULL;
			ile = 1;
			poziom = random(10,16);
			rozmowa = dialog_szalony_mag;
			break;
		case 1: // szale�cy
			group_name = "crazies";
			ile = random(2,4);
			poziom = random(2,15);
			rozmowa = dialog_szaleni;
			break;
		case 2: // kupiec
			{
				esencial = FindUnitData("merchant");
				group_name = "guards";
				ile = random(2,4);
				poziom = random(3,8);
				GenerateMerchantItems(chest_merchant, 1000);
			}
			break;
		case 3: // bohaterowie
			group_name = "heroes";
			ile = random(2,4);
			poziom = random(2,15);
			break;
		case 4: // bandyci i w�z
			{
				far_encounter = true;
				group_name = "bandits";
				ile = random(4,6);
				poziom = random(5,10);
				group_name2 = "guards2";
				ile2 = random(2,3);
				poziom2 = random(3,8);
				SpawnObjectNearLocation(local_ctx, FindObject("wagon"), VEC2(128,128), random(MAX_ANGLE));
				Chest* chest = (Chest*)SpawnObjectNearLocation(local_ctx, FindObject("chest"), VEC2(128,128), random(MAX_ANGLE), 6.f);
				if(chest)
				{
					int gold;
					GenerateTreasure(5, 5, chest->items, gold);
					InsertItemBare(chest->items, &gold_item, (uint)gold);
					SortItems(chest->items);
				}
				guards_enc_reward = false;
			}
			break;
		case 5: // bohaterowie walcz�
			far_encounter = true;
			group_name = "heroes";
			ile = random(2,4);
			poziom = random(2,15);
			switch(rand2()%4)
			{
			case 0:
				group_name2 = "bandits";
				ile2 = random(3,5);
				poziom2 = random(6,12);
				break;
			case 1:
				group_name2 = "orcs";
				ile2 = random(3,5);
				poziom2 = random(6,12);
				break;
			case 2:
				group_name2 = "goblins";
				ile2 = random(3,5);
				poziom2 = random(6,12);
				break;
			case 3:
				group_name2 = "crazies";
				ile2 = random(2,4);
				poziom2 = random(2,15);
				break;
			}
			break;
		case 6:
			group_name = NULL;
			esencial = FindUnitData("q_magowie_golem");
			poziom = 8;
			dont_attack = true;
			rozmowa = dialog_q_magowie;
			ile = 1;
			break;
		case 7:
			group_name = NULL;
			esencial = FindUnitData("q_szaleni_szaleniec");
			poziom = 13;
			dont_attack = true;
			rozmowa = dialog_q_szaleni;
			ile = 1;
			quest_crazies->check_stone = true;
			kamien = true;
			break;
		case 8:
			group_name = "unk";
			poziom = 13;
			od_tylu = true;
			if(quest_crazies->crazies_state == Quest_Crazies::State::PickedStone)
			{
				quest_crazies->crazies_state = Quest_Crazies::State::FirstAttack;
				ile = 1;
				quest_crazies->SetProgress(Quest_Crazies::Progress::Started);
			}
			else
				ile = random(1,3);
		}
	}
	else
	{
		switch(game_enc->grupa)
		{
		case -1:
			if(rand2()%3 != 0)
				esencial = FindUnitData("wild_hunter");
			group_name = "animals";
			break;
		case SG_BANDYCI:
			group_name = "bandits";
			break;
		case SG_GOBLINY:
			group_name = "goblins";
			break;
		case SG_ORKOWIE:
			group_name = "orcs";
			break;
		}

		ile = random(3,5);
		poziom = random(6,12);
		rozmowa = game_enc->dialog;
		dont_attack = game_enc->dont_attack;
		quest = game_enc->quest;
		location_event_handler = game_enc->location_event_handler;
	}

	EnemyGroup* group = NULL;
	int suma = 0;
	if(group_name)
	{
		group = &FindEnemyGroup(group_name);
		for(int i=0; i<group->count; ++i)
			suma += group->enemies[i].count;
	}

	Unit* closest = NULL;
	float dist, best_dist;

	VEC3 spawn_pos(128.f,0,128.f);
	if(od_tylu)
	{
		switch(enc_kierunek)
		{
		case 0:
			spawn_pos = VEC3(140.f,0,128.f);
			break;
		case 1:
			spawn_pos = VEC3(128.f,0.f,140.f);
			break;
		case 2:
			spawn_pos = VEC3(116.f,0.f,128.f);
			break;
		case 3:
			spawn_pos = VEC3(128.f,0.f,116.f);
			break;
		}
	}

	if(esencial)
	{
		closest = SpawnUnitNearLocation(local_ctx, spawn_pos, *esencial, &look_pt, clamp(random(esencial->level), poziom/2, poziom), 4.f);
		closest->dont_attack = dont_attack;
		//assert(closest->level <= poziom);
		best_dist = distance(closest->pos, look_pt);
		--ile;

		if(kamien)
		{
			int slot = closest->FindItem(FindItem("q_szaleni_kamien"));
			if(slot != -1)
				closest->items[slot].team_count = 0;
		}
	}

	for(int i=0; i<ile; ++i)
	{
		int x = rand2()%suma,
			y = 0;
		for(int j=0; j<group->count; ++j)
		{
			y += group->enemies[j].count;
			if(x < y)
			{
				Unit* u = SpawnUnitNearLocation(local_ctx, spawn_pos, *group->enemies[j].unit, &look_pt, clamp(random(group->enemies[j].unit->level), poziom/2, poziom), 4.f);
				//assert(u->level <= poziom);
				// ^ w czasie spotkania mo�e wygenerowa� silniejszych wrog�w ni� poziom :(
				u->dont_attack = dont_attack;
				dist = distance(u->pos, look_pt);
				if(!closest || dist < best_dist)
				{
					closest = u;
					best_dist = dist;
				}
				break;
			}
		}
	}

	// druga grupa
	if(group_name2)
	{
		suma = 0;
		group = &FindEnemyGroup(group_name2);
		for(int i=0; i<group->count; ++i)
			suma += group->enemies[i].count;

		for(int i=0; i<ile2; ++i)
		{
			int x = rand2()%suma,
				y = 0;
			for(int j=0; j<group->count; ++j)
			{
				y += group->enemies[j].count;
				if(x < y)
				{
					Unit* u = SpawnUnitNearLocation(local_ctx, spawn_pos, *group->enemies[j].unit, &look_pt, clamp(random(group->enemies[j].unit->level), poziom2/2, poziom2), 4.f);
					//assert(u->level <= poziom2);
					// ^ w czasie spotkania mo�e wygenerowa� silniejszych wrog�w ni� poziom :(
					u->dont_attack = dont_attack;
					break;
				}
			}
		}
	}

	// dialog
	if(rozmowa)
	{
		DialogContext& ctx = *leader->player->dialog_ctx;
		StartDialog2(leader->player, closest, rozmowa);
		ctx.dialog_quest = quest;
	}
}

void Game::SpawnEncounterObjects()
{
	SpawnForestObjects((enc_kierunek == 0 || enc_kierunek == 2) ? 0 : 1);
}

void Game::SpawnEncounterTeam()
{
	VEC3 pos;
	float dir;

	VEC3 look_pt;
	switch(enc_kierunek)
	{
	case 0:
		if(far_encounter)
			pos = VEC3(140.f,0.f,128.f);
		else
			pos = VEC3(135.f,0.f,128.f);
		dir = PI/2;
		break;
	case 1:
		if(far_encounter)
			pos = VEC3(128.f,0.f,140.f);
		else
			pos = VEC3(128.f,0.f,135.f);
		dir = 0;
		break;
	case 2:
		if(far_encounter)
			pos = VEC3(116.f,0.f,128.f);
		else
			pos = VEC3(121.f,0.f,128.f);
		dir = 3.f/2*PI;
		break;
	case 3:
		if(far_encounter)
			pos = VEC3(128.f,0.f,116.f);
		else
			pos = VEC3(128.f,0.f,121.f);
		dir = PI;
		break;
	}

	AddPlayerTeam(pos, dir, false, true);
}

Encounter* Game::AddEncounter(int& id)
{
	for(int i=0, size = (int)encs.size(); i<size; ++i)
	{
		if(!encs[i])
		{
			id = i;
			encs[i] = new Encounter;
			return encs[i];
		}
	}

	Encounter* enc = new Encounter;
	id = encs.size();
	encs.push_back(enc);
	return enc;
}

void Game::RemoveEncounter(int id)
{
	assert(in_range(id, 0, (int)encs.size()-1) && encs[id]);
	delete encs[id];
	encs[id] = NULL;
}

Encounter* Game::GetEncounter(int id)
{
	assert(in_range(id, 0, (int)encs.size()-1) && encs[id]);
	return encs[id];
}

Encounter* Game::RecreateEncounter(int id)
{
	assert(in_range(id, 0, (int)encs.size()-1));
	Encounter* e = new Encounter;
	encs[id] = e;
	return e;
}

// znajduje poblisk� lokacje wkt�rej s� tacy wrogowie
// je�li jest pusta oczyszczona to si� tam pojawiaj�
// je�li nie ma to zak�ada ob�z
int Game::GetRandomSpawnLocation(const VEC2& pos, SPAWN_GROUP group, float range)
{
	int best_ok = -1, best_empty = -1, index = cities;
	float ok_range, empty_range, dist;

	for(vector<Location*>::iterator it = locations.begin()+cities, end = locations.end(); it != end; ++it, ++index)
	{
		if(!*it)
			continue;
		if(!(*it)->active_quest && ((*it)->type == L_DUNGEON || (*it)->type == L_CRYPT))
		{
			InsideLocation* inside = (InsideLocation*)*it;
			if((*it)->state == LS_CLEARED || inside->spawn == group || inside->spawn == SG_BRAK)
			{
				dist = distance(pos, (*it)->pos);
				if(dist <= range)
				{
					if(inside->spawn == group)
					{
						if(best_ok == -1 || dist < ok_range)
						{
							best_ok = index;
							ok_range = dist;
						}
					}
					else
					{
						if(best_empty == -1 || dist < empty_range)
						{
							best_empty = index;
							empty_range = dist;
						}
					}
				}
			}
		}
	}

	if(best_ok != -1)
	{
		locations[best_ok]->reset = true;
		return best_ok;
	}

	if(best_empty != -1)
	{
		locations[best_empty]->spawn = group;
		locations[best_empty]->reset = true;
		return best_empty;
	}

	return CreateCamp(pos, group, range/2);
}

int Game::CreateCamp(const VEC2& pos, SPAWN_GROUP group, float range, bool allow_exact)
{
	Camp* loc = new Camp;
	loc->state = LS_UNKNOWN;
	loc->active_quest = NULL;
	loc->name = txCamp;
	loc->type = L_CAMP;
	loc->last_visit = -1;
	loc->st = random(3,15);
	loc->reset = false;
	loc->spawn = group;
	loc->pos = pos;
	loc->create_time = worldtime;

	FindPlaceForLocation(loc->pos, range, allow_exact);

	return AddLocation(loc);
}

// po 30 dniach od odwiedzin oznacza lokacje do zresetowania
void Game::DoWorldProgress(int days)
{
	// usuwanie questowych postaci po czasie
	for(vector<TimedUnit>::iterator it = timed_units.begin(), end = timed_units.end(); it != end;)
	{
		TimedUnit& tu = *it;
		tu.days -= days;
		if(tu.days <= 0)
		{
			RemoveUnitFromLocation(tu.unit, tu.location);
			if(it+1 == end)
			{
				timed_units.pop_back();
				break;
			}
			else
			{
				it = timed_units.erase(it);
				end = timed_units.end();
			}
		}
		else
			++it;
	}

	// tworzenie oboz�w
	create_camp += days;
	if(create_camp >= 10)
	{
		create_camp = 0;
		SPAWN_GROUP group;
		switch(rand2()%3)
		{
		case 0:
			group = SG_BANDYCI;
			break;
		case 1:
			group = SG_ORKOWIE;
			break;
		case 2:
			group = SG_GOBLINY;
			break;
		}
		CreateCamp(random_VEC2(16.f,600-16.f), group, 128.f);
	}

	// zanikanie questowych spotka�
	for(vector<Encounter*>::iterator it = encs.begin(), end = encs.end(); it != end; ++it)
	{
		if(*it && (*it)->timed && (*it)->quest->IsTimedout())
		{
			(*it)->quest->enc = -1;
			delete *it;
			if(it+1 == end)
			{
				encs.pop_back();
				break;
			}
			else
				*it = NULL;
		}
	}

	// ustawianie podziemi jako nie questowych po czasie / usuwanie oboz�w questowych
	for(vector<Quest_Dungeon*>::iterator it = quests_timeout.begin(), end = quests_timeout.end(); it != end;)
	{
		if((*it)->IsTimedout() && (*it)->target_loc != current_location)
		{
			Location* loc = locations[(*it)->target_loc];

			if(loc->type == L_CAMP && (*it)->target_loc == picked_location)
				continue;

			loc->active_quest = NULL;

			if(loc->type == L_CAMP)
			{
				if(IsOnline())
				{
					NetChange& c = Add1(net_changes);
					c.type = NetChange::REMOVE_CAMP;
					c.id = (*it)->target_loc;
				}

				(*it)->target_loc = -1;

				OutsideLocation* outside = (OutsideLocation*)loc;
				DeleteElements(outside->chests);
				DeleteElements(outside->items);
				for(vector<Unit*>::iterator it2 = outside->units.begin(), end2 = outside->units.end(); it2 != end2; ++it2)
					delete *it2;
				outside->units.clear();
				delete loc;
				
				for(vector<Location*>::iterator it2 = locations.begin(), end2 = locations.end(); it2 != end2; ++it2)
				{
					if((*it2) == loc)
					{
						if(it2+1 == end2)
							locations.pop_back();
						else
						{
							*it2 = NULL;
							++empty_locations;
						}
						break;
					}
				}

				if(it+1 == end)
				{
					quests_timeout.pop_back();
					break;
				}
				else
				{
					it = quests_timeout.erase(it);
					end = quests_timeout.end();
				}
			}
			else
				++it;
		}
		else
			++it;
	}

	// resetowanie lokacji / usuwanie oboz�w po czasie
	int index = 0;
	for(vector<Location*>::iterator it = locations.begin(), end = locations.end(); it != end; ++it, ++index)
	{
		if(!*it || (*it)->active_quest || (*it)->type == L_ENCOUNTER)
			continue;
		if((*it)->type == L_CAMP)
		{
			Camp* camp = (Camp*)(*it);
			if(worldtime - camp->create_time >= 30 && location != *it && (picked_location == -1 || locations[picked_location] != *it))
			{
				// usu� ob�z
				DeleteElements(camp->chests);
				DeleteElements(camp->items);
				for(vector<Unit*>::iterator it2 = camp->units.begin(), end2 = camp->units.end(); it2 != end2; ++it2)
					delete *it2;
				camp->units.clear();
				delete *it;

				if(it+1 == end)
				{
					locations.pop_back();
					break;
				}
				else
				{
					*it = NULL;
					++empty_locations;
				}

				if(IsOnline())
				{
					NetChange& c = Add1(net_changes);
					c.type = NetChange::REMOVE_CAMP;
					c.id = index;
				}
			}
		}
		else if((*it)->last_visit != -1 && worldtime - (*it)->last_visit >= 30)
		{
			(*it)->reset = true;
			if((*it)->state == LS_CLEARED)
				(*it)->state = LS_ENTERED;
			if((*it)->type == L_DUNGEON || (*it)->type == L_CRYPT)
			{
				InsideLocation* inside = (InsideLocation*)*it;
				if(inside->target != LABIRYNTH)
					inside->spawn = RandomSpawnGroup(g_base_locations[inside->target]);
				if(inside->IsMultilevel())
					((MultiInsideLocation*)inside)->Reset();
			}
		}
	}

	UpdateQuests(days);

	// aktualizuj newsy
	bool deleted = false;
	for(vector<News*>::iterator it = news.begin(), end = news.end(); it != end; ++it)
	{
		if(worldtime - (*it)->add_time > 30)
		{
			delete *it;
			*it = NULL;
			deleted = true;
		}
	}
	if(deleted)
		RemoveNullElements(news);
}

// up�yw czasu
// 1-10 dni (usu� zw�oki)
// 5-30 dni (usu� krew)
// 1+ zamknij/otw�rz drzwi
template<typename T>
struct RemoveRandomPred
{
	int chance, a, b;

	RemoveRandomPred(int chance, int a, int b)
	{

	}

	inline bool operator () (const T&)
	{
		return random(a,b) < chance;
	}
};

void Game::UpdateLocation(LevelContext& ctx, int days, int open_chance, bool reset)
{
	if(days <= 10)
	{
		// usu� niekt�re zw�oki i przedmioty
		for(vector<Unit*>::iterator it = ctx.units->begin(), end = ctx.units->end(); it != end; ++it)
		{
			if(!(*it)->IsAlive() && random(4,10) < days)
			{
				delete *it;
				*it = NULL;
			}
		}
		ctx.units->erase(std::remove_if(ctx.units->begin(), ctx.units->end(), is_null<Unit*>), ctx.units->end());
		vector<GroundItem*>::iterator from = std::remove_if(ctx.items->begin(), ctx.items->end(), RemoveRandomPred<GroundItem*>(days, 0, 10));
		for(vector<GroundItem*>::iterator it = from, end = ctx.items->end(); it != end; ++it)
			delete *it;
		ctx.items->erase(from, ctx.items->end());
	}
	else
	{
		// usu� wszystkie zw�oki i przedmioty
		if(reset)
		{
			for(vector<Unit*>::iterator it = ctx.units->begin(), end = ctx.units->end(); it != end; ++it)
				delete *it;
			ctx.units->clear();
		}
		else
		{
			for(vector<Unit*>::iterator it = ctx.units->begin(), end = ctx.units->end(); it != end; ++it)
			{
				if(!(*it)->IsAlive())
				{
					delete *it;
					*it = NULL;
				}
			}
			ctx.units->erase(std::remove_if(ctx.units->begin(), ctx.units->end(), is_null<Unit*>), ctx.units->end());
		}
		DeleteElements(ctx.items);
	}

	// wylecz jednostki
	if(!reset)
	{
		for(vector<Unit*>::iterator it = ctx.units->begin(), end = ctx.units->end(); it != end; ++it)
		{
			if((*it)->IsAlive())
				(*it)->NaturalHealing(days);
		}
	}

	if(days > 30)
	{
		// usu� krew
		ctx.bloods->clear();
	}
	else if(days >= 5)
	{
		// usu� cz�ciowo krew
		ctx.bloods->erase(std::remove_if(ctx.bloods->begin(), ctx.bloods->end(), RemoveRandomPred<Blood>(days, 4, 30)), ctx.bloods->end());
	}

	if(ctx.traps)
	{
		if(days > 30)
		{
			// usu� wszystkie jednorazowe pu�apki
			for(vector<Trap*>::iterator it = ctx.traps->begin(), end = ctx.traps->end(); it != end;)
			{
				if((*it)->base->type == TRAP_FIREBALL)
				{
					delete *it;
					if(it+1 == end)
					{
						ctx.traps->pop_back();
						break;
					}
					else
					{
						std::iter_swap(it, end-1);
						ctx.traps->pop_back();
						end = ctx.traps->end();
					}
				}
				else
					++it;
			}
		}
		else if(days >= 5)
		{
			// usu� cz�� 1razowych pu�apek
			for(vector<Trap*>::iterator it = ctx.traps->begin(), end = ctx.traps->end(); it != end;)
			{
				if((*it)->base->type == TRAP_FIREBALL && rand2()%30 < days)
				{
					delete *it;
					if(it+1 == end)
					{
						ctx.traps->pop_back();
						break;
					}
					else
					{
						std::iter_swap(it, end-1);
						ctx.traps->pop_back();
						end = ctx.traps->end();
					}
				}
				else
					++it;
			}
		}
	}

	// losowo otw�rz/zamknij drzwi
	if(ctx.doors)
	{
		for(vector<Door*>::iterator it = ctx.doors->begin(), end = ctx.doors->end(); it != end; ++it)
		{
			Door& door = **it;
			if(door.locked == 0)
			{
				if(rand2()%100 < open_chance)
					door.state = Door::Open;
				else
					door.state = Door::Closed;
			}
		}
	}
}

void Game::UpdateLocation(int days, int open_chance, bool reset)
{
	UpdateLocation(local_ctx, days, open_chance, reset);

	if(city_ctx)
	{
		for(vector<InsideBuilding*>::iterator it = city_ctx->inside_buildings.begin(), end = city_ctx->inside_buildings.end(); it != end; ++it)
			UpdateLocation((*it)->ctx, days, open_chance, reset);
	}
}

void Game::GenerateCamp(Location& loc)
{
	OutsideLocation* outside = (OutsideLocation*)&loc;

	// stw�rz map� je�li jeszcze nie ma
	const int _s = OutsideLocation::size;
	if(!outside->tiles)
	{
		outside->tiles = new TerrainTile[_s*_s];
		outside->h = new float[(_s+1)*(_s+1)];
		memset(outside->tiles, 0, sizeof(TerrainTile)*_s*_s);
	}

	Perlin perlin2(4,4,1);

	for(int i=0, y=0; y<_s; ++y)
	{
		for(int x=0; x<_s; ++x, ++i)
		{
			int v = int((perlin2.Get(1.f/256*float(x), 1.f/256*float(y))+1.f)*50);
			TERRAIN_TILE& t = outside->tiles[i].t;
			if(v < 42)
				t = TT_GRASS;
			else if(v < 58)
				t = TT_GRASS2;
			else
				t = TT_GRASS3;
		}
	}

	LoadingStep(txGeneratingTerrain);

	Perlin perlin(4,4,1);

	// losuj teren
	terrain->SetHeightMap(outside->h);
	terrain->RandomizeHeight(0.f,5.f);
	float* h = terrain->GetHeightMap();

	for(uint y=0; y<_s; ++y)
	{
		for(uint x=0; x<_s; ++x)
		{
			if(x < 15 || x > _s-15 || y < 15 || y > _s-15)
				h[x+y*(_s+1)] += random(10.f,15.f);
		}
	}

	terrain->RoundHeight();
	terrain->RoundHeight();

	for(uint y=0; y<_s; ++y)
	{
		for(uint x=0; x<_s; ++x)
			h[x+y*(_s+1)] += (perlin.Get(1.f/256*x, 1.f/256*y)+1.f)*4;
	}

	terrain->RoundHeight();
	terrain->RemoveHeightMap();
}

cstring camp_objs[] = {
	"barrel",
	"barrels",
	"box",
	"boxes",
	"bow_target",
	"torch",
	"torch_off",
	"tanning_rack",
	"hay",
	"firewood",
	"bench",
	"chest",
	"melee_target",
	"anvil",
	"cauldron"
};
const uint n_camp_objs = countof(camp_objs);
Obj* camp_objs_ptrs[n_camp_objs];

void Game::SpawnCampObjects()
{
	SpawnForestObjects();

	vector<VEC2> pts;
	Obj* ognisko = FindObject("campfire"),
		* ognisko_zgaszone = FindObject("campfire_off"),
		* namiot = FindObject("tent"),
		* poslanie = FindObject("bedding");

	if(!camp_objs_ptrs[0])
	{
		for(uint i=0; i<n_camp_objs; ++i)
			camp_objs_ptrs[i] = FindObject(camp_objs[i]);
	}

	for(int i=0; i<20; ++i)
	{
		VEC2 pt = random(VEC2(96,96), VEC2(256-96,256-96));

		// sprawd� czy nie ma w pobli�u ogniska
		bool jest = false;
		for(vector<VEC2>::iterator it = pts.begin(), end = pts.end(); it != end; ++it)
		{
			if(distance(pt, *it) < 16.f)
			{
				jest = true;
				break;
			}
		}
		if(jest)
			continue;

		pts.push_back(pt);

		// ognisko
		if(SpawnObjectNearLocation(local_ctx, rand2()%5 == 0 ? ognisko_zgaszone : ognisko, pt, random(MAX_ANGLE)))
		{
			// namioty / pos�ania
			for(int j=0, ile = random(4,7); j<ile; ++j)
			{
				float kat = random(MAX_ANGLE);
				bool czy_namiot = (rand2()%2 == 0);
				if(czy_namiot)
					SpawnObjectNearLocation(local_ctx, namiot, pt + VEC2(sin(kat),cos(kat))*random(4.f,5.5f), pt);
				else
					SpawnObjectNearLocation(local_ctx, poslanie, pt + VEC2(sin(kat),cos(kat))*random(3.f,4.f), pt);
			}
		}
	}

	for(int i=0; i<100; ++i)
	{
		VEC2 pt = random(VEC2(90,90), VEC2(256-90,256-90));
		bool ok = true;
		for(vector<VEC2>::iterator it = pts.begin(), end = pts.end(); it != end; ++it)
		{
			if(distance(*it, pt) < 4.f)
			{
				ok = false;
				break;
			}
		}
		if(ok)
		{
			Obj* obj = camp_objs_ptrs[rand2()%n_camp_objs];
			Object* o = SpawnObjectNearLocation(local_ctx, obj, pt, random(MAX_ANGLE), 2.f);
			if(o && IS_SET(obj->flagi, OBJ_SKRZYNIA))
			{
				int gold, level = location->st;
				Chest* chest = (Chest*)o;

				GenerateTreasure(level, 5, chest->items, gold);
				InsertItemBare(chest->items, &gold_item, (uint)gold);
				SortItems(chest->items);
			}
		}
	}
}

void Game::SpawnCampUnits()
{
	static TGroup ee;
	static vector<VEC2> poss;
	OutsideLocation* outside = (OutsideLocation*)location;
	int level = outside->st;
	cstring group_name;

	switch(outside->spawn)
	{
	default:
		assert(0);
	case SG_BANDYCI:
		group_name = "bandits";
		break;
	case SG_ORKOWIE:
		group_name = "orcs";
		break;
	case SG_GOBLINY:
		group_name = "goblins";
		break;
	case SG_MAGOWIE:
		group_name = "mages";
		break;
	case SG_ZLO:
		group_name = "evil";
		break;
	}

	// ustal wrog�w
	EnemyGroup& group = FindEnemyGroup(group_name);
	ee.total = 0;
	for(int j=0; j<group.count; ++j)
	{
		if(group.enemies[j].unit->level.x <= level)
		{
			ee.total += group.enemies[j].count;
			ee.enemies.push_back(&group.enemies[j]);
		}
	}

	for(int added=0, tries=50; added<5 && tries>0; --tries)
	{
		VEC2 pos = random(VEC2(90,90), VEC2(256-90,256-90));

		bool ok = true;
		for(vector<VEC2>::iterator it = poss.begin(), end = poss.end(); it != end; ++it)
		{
			if(distance(pos, *it) < 8.f)
			{
				ok = false;
				break;
			}
		}

		if(ok)
		{
			// losuj grupe
			poss.push_back(pos);
			++added;

			VEC3 pos3(pos.x, 0, pos.y);

			// postaw jednostki
			int levels = level * 2;
			while(levels > 0)
			{
				int k = rand2()%ee.total, l = 0;
				UnitData* ud;

				DEBUG_DO(ud = NULL);

				for(vector<EnemyEntry*>::iterator it = ee.enemies.begin(), end = ee.enemies.end(); it != end; ++it)
				{
					l += (*it)->count;
					if(k < l)
					{
						ud = (*it)->unit;
						break;
					}
				}

				assert(ud);

				if(ud->level.x > levels)
					break;

				int enemy_level = random2(ud->level.x, min3(ud->level.y, levels, level));
				if(!SpawnUnitNearLocation(local_ctx, pos3, *ud, NULL, enemy_level, 6.f))
					break;
				levels -= enemy_level;
			}
		}
	}

	poss.clear();
	ee.enemies.clear();
}

Object* Game::SpawnObjectNearLocation(LevelContext& ctx, Obj* obj, const VEC2& pos, float rot, float range, float margin, float scale)
{
	bool ok = false;
	if(obj->type == OBJ_CYLINDER)
	{
		global_col.clear();
		VEC3 pt(pos.x, 0, pos.y);
		GatherCollisionObjects(ctx, global_col, pt, obj->r+margin+range);
		float extra_radius = range/20;
		for(int i=0; i<20; ++i)
		{
			if(!Collide(global_col, pt, obj->r+margin))
			{
				ok = true;
				break;
			}
			pt = VEC3(pos.x, 0, pos.y);
			int j = rand2()%poisson_disc_count;
			pt.x += POISSON_DISC_2D[j].x*extra_radius;
			pt.z += POISSON_DISC_2D[j].y*extra_radius;
			extra_radius += range/20;
		}

		if(!ok)
			return NULL;

		if(ctx.type == LevelContext::Outside)
			terrain->SetH(pt);

		return SpawnObject(ctx, obj, pt, rot, scale);
	}
	else
	{
		global_col.clear();
		VEC3 pt(pos.x, 0, pos.y);
		GatherCollisionObjects(ctx, global_col, pt, sqrt(obj->size.x+obj->size.y)+margin+range);
		float extra_radius = range/20;
		BOX2D box(pos);
		box.v1.x -= obj->size.x;
		box.v1.y -= obj->size.y;
		box.v2.x += obj->size.x;
		box.v2.y += obj->size.y;
		for(int i=0; i<20; ++i)
		{
			if(!Collide(global_col, box, margin, rot))
			{
				ok = true;
				break;
			}
			pt = VEC3(pos.x, 0, pos.y);
			int j = rand2()%poisson_disc_count;
			pt.x += POISSON_DISC_2D[j].x*extra_radius;
			pt.z += POISSON_DISC_2D[j].y*extra_radius;
			extra_radius += range/20;
			box.v1.x = pt.x - obj->size.x;
			box.v1.y = pt.z - obj->size.y;
			box.v2.x = pt.x + obj->size.x;
			box.v2.y = pt.z + obj->size.y;
		}

		if(!ok)
			return NULL;

		if(ctx.type == LevelContext::Outside)
			terrain->SetH(pt);

		return SpawnObject(ctx, obj, pt, rot, scale);
	}
}

Object* Game::SpawnObjectNearLocation(LevelContext& ctx, Obj* obj, const VEC2& pos, const VEC2& rot_target, float range, float margin, float scale)
{
	if(obj->type == OBJ_CYLINDER)
		return SpawnObjectNearLocation(ctx, obj, pos, lookat_angle(pos, rot_target), range, margin, scale);
	else
	{
		global_col.clear();
		VEC3 pt(pos.x, 0, pos.y);
		GatherCollisionObjects(ctx, global_col, pt, sqrt(obj->size.x+obj->size.y)+margin+range);
		float extra_radius = range/20, rot;
		BOX2D box(pos);
		box.v1.x -= obj->size.x;
		box.v1.y -= obj->size.y;
		box.v2.x += obj->size.x;
		box.v2.y += obj->size.y;
		bool ok = false;
		for(int i=0; i<20; ++i)
		{
			rot = lookat_angle(VEC2(pt.x,pt.z), rot_target);
			if(!Collide(global_col, box, margin, rot))
			{
				ok = true;
				break;
			}
			pt = VEC3(pos.x, 0, pos.y);
			int j = rand2()%poisson_disc_count;
			pt.x += POISSON_DISC_2D[j].x*extra_radius;
			pt.z += POISSON_DISC_2D[j].y*extra_radius;
			extra_radius += range/20;
			box.v1.x = pt.x - obj->size.x;
			box.v1.y = pt.z - obj->size.y;
			box.v2.x = pt.x + obj->size.x;
			box.v2.y = pt.z + obj->size.y;
		}

		if(!ok)
			return NULL;

		if(ctx.type == LevelContext::Outside)
			terrain->SetH(pt);

		return SpawnObject(ctx, obj, pt, rot, scale);
	}
}

int Game::GetClosestLocation(LOCATION type, const VEC2& pos, int target)
{
	int best = -1, index = 0;
	float dist, best_dist;

	for(vector<Location*>::iterator it = locations.begin(), end = locations.end(); it != end; ++it, ++index)
	{
		if(!*it || (*it)->active_quest || (*it)->type != type)
			continue;
		if(target != -1 && ((InsideLocation*)(*it))->target != target)
			continue;
		dist = distance((*it)->pos, pos);
		if(best == -1 || dist < best_dist)
		{
			best = index;
			best_dist = dist;
		}
	}

	return best;
}

int Game::GetClosestLocationNotTarget(LOCATION type, const VEC2& pos, int not_target)
{
	int best = -1, index = 0;
	float dist, best_dist;

	for(vector<Location*>::iterator it = locations.begin(), end = locations.end(); it != end; ++it, ++index)
	{
		if(!*it || (*it)->active_quest || (*it)->type != type)
			continue;
		if(((InsideLocation*)(*it))->target == not_target)
			continue;
		dist = distance((*it)->pos, pos);
		if(best == -1 || dist < best_dist)
		{
			best = index;
			best_dist = dist;
		}
	}

	return best;
}

void Game::SpawnTmpUnits(City* city)
{
	InsideBuilding* inn = city->FindInn();
	CityBuilding* pola = city->FindBuilding(B_TRAINING_GROUND);

	// bohaterowie
	if(first_city)
	{
		first_city = false;
		for(int i=0; i<4; ++i)
		{
			UnitData& ud = GetHero(ClassInfo::GetRandom());

			if(rand2()%2 == 0 || !pola)
			{
				// w karczmie
				Unit* u = SpawnUnitInsideInn(ud, random(2,5), inn);
				if(u)
				{
					u->rot = random(MAX_ANGLE);
					u->temporary = true;
				}
			}
			else
			{
				// na polu treningowym
				Unit* u = SpawnUnitNearLocation(local_ctx, VEC3(2.f*pola->unit_pt.x+1, 0, 2.f*pola->unit_pt.y+1), ud, NULL, random(2,5), 8.f);
				if(u)
					u->temporary = true;
			}
		}
	}
	else
	{
		int ile = random(1,4);
		for(int i=0; i<ile; ++i)
		{
			UnitData& ud = GetHero(ClassInfo::GetRandom());

			if(rand2()%2 == 0 || !pola)
			{
				// w karczmie
				Unit* u = SpawnUnitInsideArea(inn->ctx, (rand2()%5 == 0 ? inn->arena2 : inn->arena1), ud, random(2,15));
				if(u)
				{
					u->rot = random(MAX_ANGLE);
					u->temporary = true;
				}
			}
			else
			{
				// na polu treningowym
				Unit* u = SpawnUnitNearLocation(local_ctx, VEC3(2.f*pola->unit_pt.x+1, 0, 2.f*pola->unit_pt.y+1), ud, NULL, random(2,15), 8.f);
				if(u)
					u->temporary = true;
			}
		}
	}

	// questowa posta�
	if(city_ctx->type == L_CITY || rand2()%2 == 0)
	{
		Unit* u = SpawnUnitInsideInn(*FindUnitData("traveler"), -2, inn);
		if(u)
		{
			u->rot = random(MAX_ANGLE);
			u->temporary = true;
		}
	}
}

void Game::RemoveTmpUnits(City* city)
{
	RemoveTmpUnits(local_ctx);

	for(vector<InsideBuilding*>::iterator it = city->inside_buildings.begin(), end = city->inside_buildings.end(); it != end; ++it)
		RemoveTmpUnits((*it)->ctx);
}

void Game::RemoveTmpUnits(LevelContext& ctx)
{
	for(vector<Unit*>::iterator it = ctx.units->begin(), end = ctx.units->end(); it != end;)
	{
		Unit* u = *it;
		if(u->temporary)
		{
			delete u;

			if(it+1 == end)
			{
				ctx.units->pop_back();
				return;
			}
			else
			{
				it = ctx.units->erase(it);
				end = ctx.units->end();
			}
		}
		else
			++it;
	}
}

int Game::AddLocation(Location* loc)
{
	assert(loc);

	if(empty_locations)
	{
		--empty_locations;
		int index = locations.size()-1;
		for(vector<Location*>::reverse_iterator rit = locations.rbegin(), rend = locations.rend(); rit != rend; ++rit, --index)
		{
			if(!*rit)
			{
				*rit = loc;
				if(IsOnline())
				{
					NetChange& c = Add1(net_changes);
					c.type = NetChange::ADD_LOCATION;
					c.id = index;
				}
				return index;
			}
		}
		assert(0);
		return -1;
	}
	else
	{
		if(IsOnline())
		{
			NetChange& c = Add1(net_changes);
			c.type = NetChange::ADD_LOCATION;
			c.id = locations.size();
		}
		locations.push_back(loc);
		return locations.size()-1;
	}
}

int Game::CreateLocation(LOCATION type, const VEC2& pos, float range, int target, SPAWN_GROUP spawn, bool allow_exact, int _levels)
{
	VEC2 pt = pos;
	if(range < 0.f)
	{
		pt = random_VEC2(16.f,600-16.f);
		range = -range;
	}
	if(!FindPlaceForLocation(pt, range, allow_exact))
		return -1;

	int levels = -1;
	if(type == L_DUNGEON || type == L_CRYPT)
	{
		BaseLocation& base = g_base_locations[target];
		if(_levels == -1)
			levels = base.levels.random();
		else if(_levels == 0)
			levels = base.levels.x;
		else if(_levels == 9)
			levels = base.levels.y;
		else
			levels = _levels;
	}

	Location* loc = CreateLocation(type, levels);
	loc->pos = pt;
	loc->type = type;

	if(type == L_DUNGEON || type == L_CRYPT)
	{
		InsideLocation* inside = (InsideLocation*)loc;
		inside->target = target;
		if(target == LABIRYNTH)
		{
			if(spawn == SG_LOSOWO)
				inside->spawn = SG_NIEUMARLI;
			else
				inside->spawn = spawn;
			inside->st = random(8, 15);
		}
		else
		{
			if(spawn == SG_LOSOWO)
				inside->spawn = RandomSpawnGroup(g_base_locations[target]);
			else
				inside->spawn = spawn;
			inside->st = random(3, 15);
		}
	}
	else
	{
		loc->st = random(3, 13);
		if(spawn != SG_LOSOWO)
			loc->spawn = spawn;
	}

	loc->GenerateName();

	return AddLocation(loc);
}

bool Game::FindPlaceForLocation(VEC2& pos, float range, bool allow_exact)
{
	VEC2 pt;
	
	if(allow_exact)
		pt = pos;
	else
		pt = clamp(pos + random_circle_pt(range), VEC2(16,16), VEC2(600.f-16.f,600.f-16.f));
	
	for(int i=0; i<20; ++i)
	{
		bool valid = true;
		for(vector<Location*>::iterator it = locations.begin(), end = locations.end(); it != end; ++it)
		{
			if(*it && distance(pt, (*it)->pos) < 24)
			{
				valid = false;
				break;
			}
		}

		if(valid)
		{
			pos = pt;
			return true;
		}
		else
			pt = clamp(pos + random_circle_pt(range), VEC2(16,16), VEC2(600.f-16.f,600.f-16.f));
	}

	return false;
}

int Game::GetNearestLocation2(const VEC2& pos, int flags, bool not_quest, int flagi_cel)
{
	assert(flags);

	float best_dist = 999.f;
	int best_index = -1, index = 0;

	for(vector<Location*>::iterator it = locations.begin(), end = locations.end(); it != end; ++it, ++index)
	{
		if(*it && IS_SET(flags, 1<<(*it)->type) && (!not_quest || !(*it)->active_quest))
		{
			if(flagi_cel != -1)
			{
				if((*it)->type == L_DUNGEON || (*it)->type == L_CRYPT)
				{
					if(!IS_SET(flagi_cel, 1<<((InsideLocation*)(*it))->target))
						break;
				}
			}
			float dist = distance(pos, (*it)->pos);
			if(dist < best_dist)
			{
				best_dist = dist;
				best_index = index;
			}
		}
	}

	return best_index;
}

int Game::FindLocationId(Location* loc)
{
	assert(loc);

	for(int index=0, size=int(locations.size()); index<size; ++index)
	{
		if(locations[index] == loc)
			return index;
	}

	return -1;
}

void Game::GenerateMoonwell(Location& loc)
{
	OutsideLocation* outside = (OutsideLocation*)&loc;

	// stw�rz map� je�li jeszcze nie ma
	const int _s = OutsideLocation::size;
	outside->tiles = new TerrainTile[_s*_s];
	outside->h = new float[(_s+1)*(_s+1)];
	memset(outside->tiles, 0, sizeof(TerrainTile)*_s*_s);

	Perlin perlin2(4,4,1);

	for(int i=0, y=0; y<_s; ++y)
	{
		for(int x=0; x<_s; ++x, ++i)
		{
			int v = int((perlin2.Get(1.f/256*float(x), 1.f/256*float(y))+1.f)*50);
			TERRAIN_TILE& t = outside->tiles[i].t;
			if(v < 42)
				t = TT_GRASS;
			else if(v < 58)
				t = TT_GRASS2;
			else
				t = TT_GRASS3;
		}
	}

	LoadingStep(txGeneratingTerrain);

	Perlin perlin(4,4,1);

	// losuj teren
	terrain->SetHeightMap(outside->h);
	terrain->RandomizeHeight(0.f,5.f);
	float* h = terrain->GetHeightMap();

	for(uint y=0; y<_s; ++y)
	{
		for(uint x=0; x<_s; ++x)
		{
			if(x < 15 || x > _s-15 || y < 15 || y > _s-15)
				h[x+y*(_s+1)] += random(10.f,15.f);
		}
	}

	terrain->RoundHeight();
	terrain->RoundHeight();

	for(uint y=0; y<_s; ++y)
	{
		for(uint x=0; x<_s; ++x)
			h[x+y*(_s+1)] += (perlin.Get(1.f/256*x, 1.f/256*y)+1.f)*5;
	}

	// ustaw zielon� traw� na �rodku
	for(int y=40; y<_s-40; ++y)
	{
		for(int x=40; x<_s-40; ++x)
		{
			float d;
			if((d = distance(float(x),float(y),64.f,64.f)) < 8.f)
			{
				outside->tiles[x+y*_s].t = TT_GRASS;
				h[x+y*(_s+1)] += (1.f-d/8.f)*5;
			}
		}
	}

	terrain->RoundHeight();
	terrain->RoundHeight();
	terrain->RoundHeight();
	terrain->RemoveHeightMap();
}

void Game::SpawnMoonwellObjects()
{
	VEC3 pos(128.f,0,128.f);
	terrain->SetH(pos);
	pos.y -= 0.2f;
	SpawnObject(local_ctx, FindObject("moonwell"), pos, 0.f, 1.f);
	SpawnObject(local_ctx, FindObject("moonwell_phy"), pos, 0.f, 1.f);

	if(!trees[0].obj)
	{
		for(uint i=0; i<n_trees; ++i)
			trees[i].obj = FindObject(trees[i].name);
		for(uint i=0; i<n_trees2; ++i)
			trees2[i].obj = FindObject(trees2[i].name);
		for(uint i=0; i<n_misc; ++i)
			misc[i].obj = FindObject(misc[i].name);
	}

	TerrainTile* tiles = ((OutsideLocation*)location)->tiles;

	// drzewa
	for(int i=0; i<1024; ++i)
	{
		INT2 pt(random(1,OutsideLocation::size-2), random(1, OutsideLocation::size-2));
		if(distance(float(pt.x), float(pt.y), 64.f, 64.f) > 5.f)
		{
			TERRAIN_TILE co = tiles[pt.x+pt.y*OutsideLocation::size].t;
			if(co == TT_GRASS)
			{
				VEC3 pos(random(2.f)+2.f*pt.x,0,random(2.f)+2.f*pt.y);
				pos.y = terrain->GetH(pos);
				OutsideObject& o = trees[rand2()%n_trees];
				SpawnObject(local_ctx, o.obj, pos, random(MAX_ANGLE), random2(o.scale));
			}
			else if(co == TT_GRASS3)
			{
				VEC3 pos(random(2.f)+2.f*pt.x,0,random(2.f)+2.f*pt.y);
				pos.y = terrain->GetH(pos);
				int co;
				if(rand2()%12 == 0)
					co = 3;
				else
					co = rand2()%3;
				OutsideObject& o = trees2[co];
				SpawnObject(local_ctx, o.obj, pos, random(MAX_ANGLE), random2(o.scale));
			}
		}
	}

	// inne
	for(int i=0; i<512; ++i)
	{
		INT2 pt(random(1,OutsideLocation::size-2), random(1, OutsideLocation::size-2));
		if(distance(float(pt.x), float(pt.y), 64.f, 64.f) > 5.f)
		{
			if(tiles[pt.x+pt.y*OutsideLocation::size].t != TT_SAND)
			{
				VEC3 pos(random(2.f)+2.f*pt.x,0,random(2.f)+2.f*pt.y);
				pos.y = terrain->GetH(pos);
				OutsideObject& o = misc[rand2()%n_misc];
				SpawnObject(local_ctx, o.obj, pos, random(MAX_ANGLE), random2(o.scale));
			}
		}
	}
}

void Game::SpawnMoonwellUnits(const VEC3& team_pos)
{
	// zbierz grupy
	EnemyGroup* groups[4];
	groups[0] = &FindEnemyGroup("cave_wolfs");
	groups[1] = &FindEnemyGroup("cave_spiders");
	groups[2] = &FindEnemyGroup("cave_rats");
	groups[3] = &FindEnemyGroup("animals");
	UnitData* ud_hunter = FindUnitData("wild_hunter");

	int level = GetDungeonLevel();
	static TGroup ee[4];
	static vector<VEC2> poss;
	OutsideLocation* outside = (OutsideLocation*)location;
	poss.push_back(VEC2(team_pos.x, team_pos.z));

	// ustal wrog�w
	for(int i=0; i<4; ++i)
	{
		ee[i].total = 0;
		for(int j=0; j<groups[i]->count; ++j)
		{
			if(groups[i]->enemies[j].unit->level.x <= level)
			{
				ee[i].total += groups[i]->enemies[j].count;
				ee[i].enemies.push_back(&groups[i]->enemies[j]);
			}
		}
	}

	for(int added=0, tries=50; added<8 && tries>0; --tries)
	{
		VEC2 pos = outside->GetRandomPos();
		if(distance(pos, VEC2(128.f,128.f)) < 12.f)
			continue;

		bool ok = true;
		for(vector<VEC2>::iterator it = poss.begin(), end = poss.end(); it != end; ++it)
		{
			if(distance(pos, *it) < 24.f)
			{
				ok = false;
				break;
			}
		}

		if(ok)
		{
			// losuj grupe
			TGroup& group = ee[rand2()%4];
			if(group.enemies.empty())
				continue;

			poss.push_back(pos);
			++added;

			VEC3 pos3(pos.x, 0, pos.y);

			// postaw jednostki
			int levels = level * 2;
			if(rand2()%5 == 0 && ud_hunter->level.x <= level)
			{
				int enemy_level = random2(ud_hunter->level.x, min3(ud_hunter->level.y, levels, level));
				if(SpawnUnitNearLocation(local_ctx, pos3, *ud_hunter, NULL, enemy_level, 6.f))
					levels -= enemy_level;
			}
			while(levels > 0)
			{
				int k = rand2()%group.total, l = 0;
				UnitData* ud;

				DEBUG_DO(ud = NULL);

				for(vector<EnemyEntry*>::iterator it = group.enemies.begin(), end = group.enemies.end(); it != end; ++it)
				{
					l += (*it)->count;
					if(k < l)
					{
						ud = (*it)->unit;
						break;
					}
				}

				assert(ud);

				if(ud->level.x > levels)
					break;

				int enemy_level = random2(ud->level.x, min3(ud->level.y, levels, level));
				if(!SpawnUnitNearLocation(local_ctx, pos3, *ud, NULL, enemy_level, 6.f))
					break;
				levels -= enemy_level;
			}
		}
	}

	poss.clear();
	for(int i=0; i<4; ++i)
		ee[i].enemies.clear();
}

void Game::SpawnObjectExtras(LevelContext& ctx, Obj* obj, const VEC3& pos, float rot, void* user_ptr, btCollisionObject** phy_result, float scale, int flags)
{
	assert(obj);

	// ogie� pochodni
	if(!IS_SET(flags, SOE_DONT_SPAWN_PARTICLES))
	{
		if(IS_SET(obj->flagi, OBJ_SWIATLO))
		{
			ParticleEmitter* pe = new ParticleEmitter;
			pe->alpha = 0.8f;
			pe->emision_interval = 0.1f;
			pe->emisions = -1;
			pe->life = -1;
			pe->max_particles = 50;
			pe->op_alpha = POP_LINEAR_SHRINK;
			pe->op_size = POP_LINEAR_SHRINK;
			pe->particle_life = 0.5f;
			pe->pos = pos;
			pe->pos.y += obj->centery;
			pe->pos_min = VEC3(0,0,0);
			pe->pos_max = VEC3(0,0,0);
			pe->spawn_min = 1;
			pe->spawn_max = 3;
			pe->speed_min = VEC3(-1,3,-1);
			pe->speed_max = VEC3(1,4,1);
			pe->mode = 1;
			pe->Init();
			ctx.pes->push_back(pe);

			pe->tex = tFlare;
			if(IS_SET(obj->flagi, OBJ_OGNISKO))
				pe->size = 0.7f;
			else
			{
				pe->size = 0.5f;
				if(IS_SET(flags, SOE_MAGIC_LIGHT))
					pe->tex = tFlare2;
			}

			// �wiat�o
			if(!IS_SET(flags, SOE_DONT_CREATE_LIGHT) && ctx.lights)
			{
				Light& s = Add1(ctx.lights);
				s.pos = pe->pos;
				s.range = 5;
				if(IS_SET(flags, SOE_MAGIC_LIGHT))
					s.color = VEC3(0.8f,0.8f,1.f);
				else
					s.color = VEC3(1.f,0.9f,0.9f);
			}
		}
		else if(IS_SET(obj->flagi, OBJ_KREW))
		{
			// krew
			ParticleEmitter* pe = new ParticleEmitter;
			pe->alpha = 0.8f;
			pe->emision_interval = 0.1f;
			pe->emisions = -1;
			pe->life = -1;
			pe->max_particles = 50;
			pe->op_alpha = POP_LINEAR_SHRINK;
			pe->op_size = POP_LINEAR_SHRINK;
			pe->particle_life = 0.5f;
			pe->pos = pos;
			pe->pos.y += obj->centery;
			pe->pos_min = VEC3(0,0,0);
			pe->pos_max = VEC3(0,0,0);
			pe->spawn_min = 1;
			pe->spawn_max = 3;
			pe->speed_min = VEC3(-1,4,-1);
			pe->speed_max = VEC3(1,6,1);
			pe->mode = 0;
			pe->tex = tKrew[BLOOD_RED];
			pe->size = 0.5f;
			pe->Init();
			ctx.pes->push_back(pe);
		}
		else if(IS_SET(obj->flagi, OBJ_WODA))
		{
			// krew
			ParticleEmitter* pe = new ParticleEmitter;
			pe->alpha = 0.8f;
			pe->emision_interval = 0.1f;
			pe->emisions = -1;
			pe->life = -1;
			pe->max_particles = 500;
			pe->op_alpha = POP_LINEAR_SHRINK;
			pe->op_size = POP_LINEAR_SHRINK;
			pe->particle_life = 3.f;
			pe->pos = pos;
			pe->pos.y += obj->centery;
			pe->pos_min = VEC3(0,0,0);
			pe->pos_max = VEC3(0,0,0);
			pe->spawn_min = 4;
			pe->spawn_max = 8;
			pe->speed_min = VEC3(-0.6f,4,-0.6f);
			pe->speed_max = VEC3(0.6f,7,0.6f);
			pe->mode = 0;
			pe->tex = tWoda;
			pe->size = 0.05f;
			pe->Init();
			ctx.pes->push_back(pe);
		}
	}

	// fizyka
	if(obj->shape)
	{
		CollisionObject& c = Add1(ctx.colliders);
		c.ptr = user_ptr;

		btCollisionObject* cobj = new btCollisionObject;
		cobj->setCollisionShape(obj->shape);

		if(obj->type == OBJ_CYLINDER)
		{
			cobj->getWorldTransform().setOrigin(btVector3(pos.x, pos.y+obj->h/2, pos.z));
			c.type = CollisionObject::SPHERE;
			c.pt = VEC2(pos.x, pos.z);
			c.radius = obj->r;
		}
		else if(obj->type == OBJ_HITBOX)
		{
			btTransform& tr = cobj->getWorldTransform();
			VEC3 zero(0,0,0), pos2;
			D3DXMatrixRotationY(&m1, rot);
			D3DXMatrixMultiply(&m2, obj->matrix, &m1);
			D3DXVec3TransformCoord(&pos2, &zero, &m2);
			pos2 += pos;
			tr.setOrigin(ToVector3(pos2));
			tr.setRotation(btQuaternion(rot, 0, 0));
			
			c.pt = VEC2(pos2.x, pos2.z);
			c.w = obj->size.x;
			c.h = obj->size.y;
			if(not_zero(rot))
			{
				c.type = CollisionObject::RECTANGLE_ROT;
				c.rot = rot;
				c.radius = max(c.w, c.h) * SQRT_2;
			}
			else
				c.type = CollisionObject::RECTANGLE;
		}
		else
		{
			D3DXMatrixRotationY(&m1, rot);
			D3DXMatrixTranslation(&m2, pos);
			// skalowanie jakim� sposobem przechodzi do btWorldTransform i przy rysowaniu jest z�a skala (dwukrotnie u�yta)
			D3DXMatrixScaling(&m3, VEC3(1.f/obj->size.x, 1.f, 1.f/obj->size.y));
			m3 = m3 * *obj->matrix * m1 * m2;
			cobj->getWorldTransform().setFromOpenGLMatrix(&m3._11);
			VEC3 coord(0,0,0), out_pos;
			D3DXVec3TransformCoord(&out_pos, &coord, &m3);
			QUAT q;
			D3DXQuaternionRotationMatrix(&q, &m3);

			float yaw = asin(-2*(q.x*q.z - q.w*q.y));
			c.pt = VEC2(out_pos.x, out_pos.z);
			c.w = obj->size.x;
			c.h = obj->size.y;
			if(not_zero(yaw))
			{
				c.type = CollisionObject::RECTANGLE_ROT;
				c.rot = yaw;
				c.radius = max(c.w, c.h) * SQRT_2;
			}
			else
				c.type = CollisionObject::RECTANGLE;
		}

		phy_world->addCollisionObject(cobj, CG_WALL);

		if(IS_SET(obj->flagi, OBJ_WSKAZNIK_NA_FIZYKE))
		{
			assert(user_ptr && phy_result);
			*phy_result = cobj;
			cobj->setUserPointer(user_ptr);
		}

		if(IS_SET(obj->flagi, OBJ_BLOKUJE_WIDOK))
			c.ptr = CAM_COLLIDER;

		if(phy_result)
			*phy_result = cobj;

		if(IS_SET(obj->flagi, OBJ_PODWOJNA_FIZYKA))
			SpawnObjectExtras(ctx, obj->next_obj, pos, rot, user_ptr, NULL, scale, flags);
		else if(IS_SET(obj->flagi2, OBJ2_WIELOFIZYKA))
		{
			for(int i=0;;++i)
			{
				if(obj->next_obj[i].shape)
					SpawnObjectExtras(ctx, &obj->next_obj[i], pos, rot, user_ptr, NULL, scale, flags);
				else
					break;
			}
		}
	}
	else if(IS_SET(obj->flagi, OBJ_SKALOWALNY))
	{
		CollisionObject& c = Add1(ctx.colliders);
		//c.ptr = obj_ptr;
		c.type = CollisionObject::SPHERE;
		c.pt = VEC2(pos.x, pos.z);
		c.radius = obj->r*scale;

		btCollisionObject* cobj = new btCollisionObject;
		btCylinderShape* shape = new btCylinderShape(btVector3(obj->r*scale, obj->h*scale, obj->r*scale));
		shapes.push_back(shape);
		cobj->setCollisionShape(shape);
		cobj->getWorldTransform().setOrigin(btVector3(pos.x, pos.y+obj->h/2*scale, pos.z));
		cobj->setCollisionFlags(CG_WALL);
		phy_world->addCollisionObject(cobj);
	}

	if(IS_SET(obj->flagi2, OBJ2_CAM_COLLIDERS))
	{
		int roti = (int)round((rot / (PI/2)));
		for(vector<Animesh::Point>::const_iterator it = obj->ani->attach_points.begin(), end = obj->ani->attach_points.end(); it != end; ++it)
		{
			const Animesh::Point& pt = *it;
			if(strncmp(pt.name.c_str(), "camcol", 6) != 0)
				continue;

			VEC3 pos2;
			D3DXMatrixRotationY(&m1, rot);
			D3DXMatrixMultiply(&m2, &pt.mat, &m1);
			D3DXVec3TransformCoord(&pos2, &VEC3(0,0,0), &m2);
			pos2 += pos;

			btBoxShape* shape = new btBoxShape(btVector3(pt.size.x, pt.size.y, pt.size.z));				
			shapes.push_back(shape);
			btCollisionObject* co = new btCollisionObject;
			co->setCollisionShape(shape);
			co->setCollisionFlags(CG_WALL);
			co->getWorldTransform().setOrigin(ToVector3(pos2));
			phy_world->addCollisionObject(co);
			if(roti != 0)
				co->getWorldTransform().setRotation(btQuaternion(rot,0,0));

			float w = pt.size.x, h = pt.size.z;
			if(roti == 1 || roti == 3)
				std::swap(w, h);

			CameraCollider& cc = Add1(cam_colliders);
			cc.box.v1.x = pos2.x - w;
			cc.box.v2.x = pos2.x + w;
			cc.box.v1.z = pos2.z - h;
			cc.box.v2.z = pos2.z + h;
			cc.box.v1.y = pos2.y - pt.size.y;
			cc.box.v2.y = pos2.y + pt.size.y;
		}		
	}
}

void Game::GenerateSecretLocation(Location& loc)
{
	sekret_stan = SS2_WYGENEROWANO2;

	OutsideLocation* outside = (OutsideLocation*)&loc;

	// stw�rz map� je�li jeszcze nie ma
	const int _s = OutsideLocation::size;
	outside->tiles = new TerrainTile[_s*_s];
	outside->h = new float[(_s+1)*(_s+1)];
	memset(outside->tiles, 0, sizeof(TerrainTile)*_s*_s);

	Perlin perlin2(4,4,1);

	for(int i=0, y=0; y<_s; ++y)
	{
		for(int x=0; x<_s; ++x, ++i)
		{
			int v = int((perlin2.Get(1.f/256*float(x), 1.f/256*float(y))+1.f)*50);
			TERRAIN_TILE& t = outside->tiles[i].t;
			if(v < 42)
				t = TT_GRASS;
			else if(v < 58)
				t = TT_GRASS2;
			else
				t = TT_GRASS3;
		}
	}

	LoadingStep(txGeneratingTerrain);

	Perlin perlin(4,4,1);

	// losuj teren
	terrain->SetHeightMap(outside->h);
	terrain->RandomizeHeight(0.f,5.f);
	float* h = terrain->GetHeightMap();

	terrain->RoundHeight();
	terrain->RoundHeight();

	for(uint y=0; y<_s; ++y)
	{
		for(uint x=0; x<_s; ++x)
			h[x+y*(_s+1)] += (perlin.Get(1.f/256*x, 1.f/256*y)+1.f)*5;
	}

	terrain->RoundHeight();

	float h1 = h[64+32*(_s+1)],
		  h2 = h[64+96*(_s+2)];

	// wyr�wnaj teren ko�o portalu i budynku
	for(uint y=0; y<_s; ++y)
	{
		for(uint x=0; x<_s; ++x)
		{
			if(distance(float(x), float(y), 64.f, 32.f) < 4.f)
				h[x+y*(_s+1)] = h1;
			else if(distance(float(x), float(y), 64.f, 96.f) < 12.f)
				h[x+y*(_s+1)] = h2;
		}
	}

	terrain->RoundHeight();

	for(uint y=0; y<_s; ++y)
	{
		for(uint x=0; x<_s; ++x)
		{
			if(x < 15 || x > _s-15 || y < 15 || y > _s-15)
				h[x+y*(_s+1)] += random(10.f,15.f);
		}
	}

	terrain->RoundHeight();

	for(uint y=0; y<_s; ++y)
	{
		for(uint x=0; x<_s; ++x)
		{
			if(distance(float(x), float(y), 64.f, 32.f) < 4.f)
				h[x+y*(_s+1)] = h1;
			else if(distance(float(x), float(y), 64.f, 96.f) < 12.f)
				h[x+y*(_s+1)] = h2;
		}
	}

	terrain->RemoveHeightMap();
}

void Game::SpawnSecretLocationObjects()
{
	VEC3 pos(128.f,0,96.f*2);
	terrain->SetH(pos);
	Obj* o = FindObject("tomashu_dom");
	pos.y += 0.05f;
	SpawnObject(local_ctx, o, pos, 0, 1.f);
	ProcessBuildingObjects(local_ctx, NULL, NULL, o->ani, NULL, 0.f, 0, VEC3(0,0,0), B_NONE, NULL, false);

	pos.z = 64.f;
	terrain->SetH(pos);
	SpawnObject(local_ctx, FindObject("portal"), pos, 0, 1.f);

	Portal* portal = new Portal;
	portal->at_level = 0;
	portal->next_portal = NULL;
	portal->pos = pos;
	portal->rot = 0.f;
	portal->target = 0;
	portal->target_loc = sekret_gdzie;
	location->portal = portal;

	if(!trees[0].obj)
	{
		for(uint i=0; i<n_trees; ++i)
			trees[i].obj = FindObject(trees[i].name);
		for(uint i=0; i<n_trees2; ++i)
			trees2[i].obj = FindObject(trees2[i].name);
		for(uint i=0; i<n_misc; ++i)
			misc[i].obj = FindObject(misc[i].name);
	}

	TerrainTile* tiles = ((OutsideLocation*)location)->tiles;

	// drzewa
	for(int i=0; i<1024; ++i)
	{
		INT2 pt(random(1,OutsideLocation::size-2), random(1, OutsideLocation::size-2));
		if(distance(float(pt.x), float(pt.y), 64.f, 32.f) > 4.f &&
			distance(float(pt.x), float(pt.y), 64.f, 96.f) > 12.f)
		{
			TERRAIN_TILE co = tiles[pt.x+pt.y*OutsideLocation::size].t;
			if(co == TT_GRASS)
			{
				VEC3 pos(random(2.f)+2.f*pt.x,0,random(2.f)+2.f*pt.y);
				pos.y = terrain->GetH(pos);
				OutsideObject& o = trees[rand2()%n_trees];
				SpawnObject(local_ctx, o.obj, pos, random(MAX_ANGLE), random2(o.scale));
			}
			else if(co == TT_GRASS3)
			{
				VEC3 pos(random(2.f)+2.f*pt.x,0,random(2.f)+2.f*pt.y);
				pos.y = terrain->GetH(pos);
				int co;
				if(rand2()%12 == 0)
					co = 3;
				else
					co = rand2()%3;
				OutsideObject& o = trees2[co];
				SpawnObject(local_ctx, o.obj, pos, random(MAX_ANGLE), random2(o.scale));
			}
		}
	}

	// inne
	for(int i=0; i<512; ++i)
	{
		INT2 pt(random(1,OutsideLocation::size-2), random(1, OutsideLocation::size-2));
		if(distance(float(pt.x), float(pt.y), 64.f, 32.f) > 4.f &&
			distance(float(pt.x), float(pt.y), 64.f, 96.f) > 12.f)
		{
			if(tiles[pt.x+pt.y*OutsideLocation::size].t != TT_SAND)
			{
				VEC3 pos(random(2.f)+2.f*pt.x,0,random(2.f)+2.f*pt.y);
				pos.y = terrain->GetH(pos);
				OutsideObject& o = misc[rand2()%n_misc];
				SpawnObject(local_ctx, o.obj, pos, random(MAX_ANGLE), random2(o.scale));
			}
		}
	}
}

void Game::SpawnSecretLocationUnits()
{
	OutsideLocation* outside = (OutsideLocation*)location;
	UnitData* golem = FindUnitData("golem_adamantine");
	static vector<VEC2> poss;

	poss.push_back(VEC2(128.f,64.f));
	poss.push_back(VEC2(128.f,256.f-64.f));

	for(int added=0, tries=50; added<10 && tries>0; --tries)
	{
		VEC2 pos = outside->GetRandomPos();

		bool ok = true;
		for(vector<VEC2>::iterator it = poss.begin(), end = poss.end(); it != end; ++it)
		{
			if(distance(pos, *it) < 32.f)
			{
				ok = false;
				break;
			}
		}

		if(ok)
		{
			SpawnUnitNearLocation(local_ctx, VEC3(pos.x,0,pos.y), *golem, NULL, -2);
			poss.push_back(pos);
			++added;
		}
	}

	poss.clear();
}

void Game::SpawnTeamSecretLocation()
{
	AddPlayerTeam(VEC3(128.f,0.f,66.f), PI, false, false);
}

void Game::GenerateMushrooms(int days_since)
{
	InsideLocation* inside = (InsideLocation*)location;
	InsideLocationLevel& lvl = inside->GetLevelData();
	INT2 pt;
	VEC2 pos;
	int dir;
	const Item* shroom = FindItem("f_mushroom");

	for(int i=0; i<days_since*20; ++i)
	{
		pt = random(INT2(1, 1), INT2(lvl.w-2, lvl.h-2));
		if(OR2_EQ(lvl.map[pt.x+pt.y*lvl.w].type, PUSTE, KRATKA_SUFIT) && lvl.IsTileNearWall(pt, dir))
		{
			pos.x = 2.f*pt.x;
			pos.y = 2.f*pt.y;
			switch(dir)
			{
			case GDIR_LEFT:
				pos.x += 0.5f;
				pos.y += random(-1.f,1.f);
				break;
			case GDIR_RIGHT:
				pos.x += 1.5f;
				pos.y += random(-1.f,1.f);
				break;
			case GDIR_UP:
				pos.x += random(-1.f,1.f);
				pos.y += 1.5f;
				break;
			case GDIR_DOWN:
				pos.x += random(-1.f,1.f);
				pos.y += 0.5f;
				break;
			}
			SpawnGroundItemInsideRadius(shroom, pos, 0.5f);
		}
	}
}

void Game::GenerateCityPickableItems()
{
	Obj* table = FindObject("table");
	Obj* shelves = FindObject("shelves");

	// piwa w karczmie
	InsideBuilding* inn = city_ctx->FindInn();
	const Item* beer = FindItem("p_beer");
	const Item* vodka = FindItem("p_vodka");
	for(vector<Object>::iterator it = inn->ctx.objects->begin(), end = inn->ctx.objects->end(); it != end; ++it)
	{
		if(it->base == table)
		{
			// 50% szansy na piwo
			if(rand2()%2 == 0)
			{
				PickableItemBegin(inn->ctx, *it);
				PickableItemAdd(beer);
			}
		}
		else if(it->base == shelves)
		{
			PickableItemBegin(inn->ctx, *it);
			for(int i=0, ile=random(3,5); i<ile; ++i)
				PickableItemAdd(beer);
			for(int i=0, ile=random(1,3); i<ile; ++i)
				PickableItemAdd(vodka);
		}
	}

	// jedzenie w sklepie
	CityBuilding* food = city_ctx->FindBuilding(B_FOOD_SELLER);
	if(food)
	{
		Object* o = NULL;
		float best_dist = 9999.f, dist;
		for(vector<Object>::iterator it = local_ctx.objects->begin(), end = local_ctx.objects->end(); it != end; ++it)
		{
			if(it->base == shelves)
			{
				dist = distance(food->walk_pt, it->pos);
				if(dist < best_dist)
				{
					best_dist = dist;
					o = &*it;
				}
			}
		}

		if(o)
		{
			const ItemList* lis = FindItemList("food_and_drink").lis;
			PickableItemBegin(local_ctx, *o);
			for(int i=0; i<20; ++i)
				PickableItemAdd(lis->Get());
		}
	}

	// miksturki u alchemika
	CityBuilding* alch = city_ctx->FindBuilding(B_ALCHEMIST);
	if(alch)
	{
		Object* o = NULL;
		float best_dist = 9999.f, dist;
		for(vector<Object>::iterator it = local_ctx.objects->begin(), end = local_ctx.objects->end(); it != end; ++it)
		{
			if(it->base == shelves)
			{
				dist = distance(alch->walk_pt, it->pos);
				if(dist < best_dist)
				{
					best_dist = dist;
					o = &*it;
				}
			}
		}

		if(o)
		{
			PickableItemBegin(local_ctx, *o);
			const Item* heal_pot = FindItem("p_hp");
			PickableItemAdd(heal_pot);
			if(rand2()%2 == 0)
				PickableItemAdd(heal_pot);
			if(rand2()%2 == 0)
				PickableItemAdd(FindItem("p_nreg"));
		}
	}
}

namespace PickableItem
{
	struct Item
	{
		uint spawn;
		VEC3 pos;
	};
	LevelContext* ctx;
	Object* o;
	vector<BOX> spawns;
	vector<Item> items;
}

void Game::PickableItemBegin(LevelContext& ctx, Object& o)
{
	assert(o.base);

	PickableItem::ctx = &ctx;
	PickableItem::o = &o;
	PickableItem::spawns.clear();
	PickableItem::items.clear();

	for(vector<Animesh::Point>::iterator it = o.base->ani->attach_points.begin(), end = o.base->ani->attach_points.end(); it != end; ++it)
	{
		if(strncmp(it->name.c_str(), "spawn_", 6) == 0)
		{
			assert(it->type == Animesh::Point::BOX);
			BOX box(it->mat._41, it->mat._42, it->mat._43);
			box.v1.x -= it->size.x-0.05f;
			box.v1.z -= it->size.z-0.05f;
			box.v2.x += it->size.x-0.05f;
			box.v2.z += it->size.z-0.05f;
			box.v1.y = box.v2.y = box.v1.y - it->size.y;
			PickableItem::spawns.push_back(box);
		}
	}

	assert(!PickableItem::spawns.empty());
}

void Game::PickableItemAdd(const Item* item)
{
	assert(item);

	for(int i=0; i<20; ++i)
	{
		// pobierz punkt
		uint spawn = rand2()%PickableItem::spawns.size();
		BOX& box = PickableItem::spawns[spawn];
		// ustal pozycj�
		VEC3 pos(random(box.v1.x, box.v2.x), box.v1.y, random(box.v1.z, box.v2.z));
		// sprawd� kolizj�
		bool ok = true;
		for(vector<PickableItem::Item>::iterator it = PickableItem::items.begin(), end = PickableItem::items.end(); it != end; ++it)
		{
			if(it->spawn == spawn)
			{
				if(CircleToCircle(pos.x, pos.z, 0.15f, it->pos.x, it->pos.z, 0.15f))
				{
					ok = false;
					break;
				}
			}
		}
		// dodaj
		if(ok)
		{
			PickableItem::Item& i = Add1(PickableItem::items);
			i.spawn = spawn;
			i.pos = pos;

			GroundItem* gi = new GroundItem;
			gi->count = 1;
			gi->team_count = 1;
			gi->item = item;
			gi->netid = item_netid_counter++;
			gi->rot = random(MAX_ANGLE);
			float rot = PickableItem::o->rot.y,
				  s = sin(rot),
				  c = cos(rot);
			gi->pos = VEC3(pos.x*c + pos.z*s, pos.y, -pos.x*s + pos.z*c) + PickableItem::o->pos;
			PickableItem::ctx->items->push_back(gi);

			break;
		}
	}
}

void Game::GenerateDungeonFood()
{
	// determine how much food to spawn
	int mod = 3;
	InsideLocation* inside = (InsideLocation*)location;
	BaseLocation& base = g_base_locations[inside->target];

	if(IS_SET(base.options, BLO_LESS_FOOD))
		--mod;

	SPAWN_GROUP spawn = location->spawn;
	if(spawn == SG_WYZWANIE)
	{
		if(dungeon_level == 0)
			spawn = SG_ORKOWIE;
		else if(dungeon_level == 1)
			spawn = SG_MAGOWIE_I_GOLEMY;
		else
			spawn = SG_ZLO;
	}
	SpawnGroup& sg = g_spawn_groups[spawn];
	mod += sg.food_mod;

	if(mod <= 0)
		return;

	// get food list and base objects
	const ItemList& lis = *FindItemList(sg.orc_food ? "orc_food" : "normal_food").lis;
	Obj* table = FindObject("table");
	Obj* shelves = FindObject("shelves");

	// spawn food
	for(vector<Object>::iterator it = local_ctx.objects->begin(), end = local_ctx.objects->end(); it != end; ++it)
	{
		if(it->base == table)
		{
			int count = random(mod/2, mod);
			if(count)
			{
				PickableItemBegin(local_ctx, *it);
				for(int i=0; i<count; ++i)
					PickableItemAdd(lis.Get());
			}
		}
		else if(it->base == shelves)
		{
			int count = random(mod, mod*3/2);
			if(count)
			{
				PickableItemBegin(local_ctx, *it);
				for(int i=0; i<count; ++i)
					PickableItemAdd(lis.Get());
			}
		}
	}
}

void Game::GenerateCityMap(Location& loc)
{
	assert(loc.type == L_CITY);

	LOG("Generating city map.");

	City* city = (City*)location;
	if(!city->tiles)
	{
		city->tiles = new TerrainTile[OutsideLocation::size*OutsideLocation::size];
		city->h = new float[(OutsideLocation::size+1)*(OutsideLocation::size+1)];
	}

	gen->Init(city->tiles, city->h, OutsideLocation::size, OutsideLocation::size);
	gen->SetRoadSize(3, 32);
	gen->SetTerrainNoise(random(3,5), random(3.f,8.f), 1.f, random(1.f,2.f));

	gen->RandomizeHeight();
	LoadingStep();

	RoadType rtype;
	int swap = 0;
	bool plaza = (rand2()%2 == 0);
	GAME_DIR dir = (GAME_DIR)(rand2()%4);

	switch(rand2()%4)
	{
	case 0:
		rtype = RoadType_Plus;
		break;
	case 1:
		rtype = RoadType_Line;
		break;
	case 2:
	case 3:
		rtype = RoadType_Three;
		swap = rand2()%6;
		break;
	}

	rtype = RoadType_Plus;

	gen->GenerateMainRoad(rtype, dir, 4, plaza, swap, city->entry_points, city->gates, true);
	LoadingStep();
	gen->FlattenRoadExits();
	LoadingStep();
	gen->GenerateRoads(TT_ROAD, 25);
	for(int i=0; i<2; ++i)
	{
		LoadingStep();
		gen->FlattenRoad();
	}
	LoadingStep();

	vector<ToBuild> tobuild;

	tobuild.push_back(ToBuild(B_CITY_HALL));
	tobuild.push_back(ToBuild(B_ARENA));
	tobuild.push_back(ToBuild(B_MERCHANT));
	tobuild.push_back(ToBuild(B_FOOD_SELLER));
	tobuild.push_back(ToBuild(B_BLACKSMITH));
	tobuild.push_back(ToBuild(B_ALCHEMIST));
	tobuild.push_back(ToBuild(B_INN));
	tobuild.push_back(ToBuild(B_TRAINING_GROUND));
	tobuild.push_back(ToBuild(B_BARRACKS));
	std::random_shuffle(tobuild.begin()+2, tobuild.begin()+8, myrand);

	const BUILDING houses[3] = {
		B_HOUSE,
		B_HOUSE2,
		B_HOUSE3
	};

	for(int i=0; i<city->citizens*3; ++i)
		tobuild.push_back(ToBuild(houses[rand2()%countof(houses)], false));

	gen->GenerateBuildings(tobuild);
	LoadingStep();
	gen->ApplyWallTiles(city->gates);
	LoadingStep();

	gen->SmoothTerrain();
	LoadingStep();
	gen->FlattenRoad();
	LoadingStep();

	if(plaza && rand2()%4 != 0)
	{
		g_have_well = true;
		g_well_pt = INT2(64,64);
	}
	else
		g_have_well = false;
	
	// budynki
	city->buildings.resize(tobuild.size());
	vector<ToBuild>::iterator build_it = tobuild.begin();
	for(vector<CityBuilding>::iterator it = city->buildings.begin(), end = city->buildings.end(); it != end; ++it, ++build_it)
	{
		it->type = build_it->type;
		it->pt = build_it->pt;
		it->rot = build_it->rot;
		it->unit_pt = build_it->unit_pt;
	}

	terrain->SetHeightMap(city->h);

	// set exits y
	for(vector<EntryPoint>::iterator entry_it = city->entry_points.begin(), entry_end = city->entry_points.end(); entry_it != entry_end; ++entry_it)
		entry_it->exit_y = terrain->GetH(entry_it->exit_area.Midpoint()) + 0.1f;

	terrain->RemoveHeightMap();

	gen->CleanBorders();
	LoadingStep();
}

void Game::GenerateVillageMap(Location& loc)
{
	assert(loc.type == L_VILLAGE);

	LOG("Generating village map.");

	Village* village = (Village*)location;
	village->have_exit = false;
	if(!village->tiles)
	{
		village->tiles = new TerrainTile[OutsideLocation::size*OutsideLocation::size];
		village->h = new float[(OutsideLocation::size+1)*(OutsideLocation::size+1)];
	}

	gen->Init(village->tiles, village->h, OutsideLocation::size, OutsideLocation::size);
	gen->SetRoadSize(3, 32);
	gen->SetTerrainNoise(random(3,5), random(3.f,8.f), 1.f, random(2.f,10.f));

	gen->RandomizeHeight();
	LoadingStep();

	RoadType rtype;
	int roads, swap = 0;
	bool plaza;
	GAME_DIR dir = (GAME_DIR)(rand2()%4);
	bool extra_roads;

	switch(rand2()%6)
	{
	case 0:
		rtype = RoadType_Line;
		roads = random(0,2);
		plaza = (rand2()%3 == 0);
		extra_roads = true;
		break;
	case 1:
		rtype = RoadType_Curve;
		roads = (rand2()%4 == 0 ? 1 : 0);
		plaza = false;
		extra_roads = false;
		break;
	case 2:
		rtype = RoadType_Oval;
		roads = (rand2()%4 == 0 ? 1 : 0);
		plaza = false;
		extra_roads = false;
		break;
	case 3:
		rtype = RoadType_Three;
		roads = random(0,3);
		plaza = (rand2()%3 == 0);
		swap = rand2()%6;
		extra_roads = true;
		break;
	case 4:
		rtype = RoadType_Sin;
		roads = (rand2()%4 == 0 ? 1 : 0);
		plaza = (rand2()%3 == 0);
		extra_roads = false;
		break;
	case 5:
		rtype = RoadType_Part;
		roads = (rand2()%3 == 0 ? 1 : 0);
		plaza = (rand2()%3 != 0);
		extra_roads = true;
		break;
	}

	gen->GenerateMainRoad(rtype, dir, roads, plaza, swap, village->entry_points, village->gates, extra_roads);
	if(extra_roads)
		gen->GenerateRoads(TT_SAND, 5);
	LoadingStep();
	gen->FlattenRoadExits();
	for(int i=0; i<2; ++i)
	{
		LoadingStep();
		gen->FlattenRoad();
	}
	LoadingStep();

	vector<ToBuild> tobuild;
	tobuild.push_back(ToBuild(B_VILLAGE_HALL));
	tobuild.push_back(ToBuild(B_MERCHANT));
	tobuild.push_back(ToBuild(B_FOOD_SELLER));
	tobuild.push_back(ToBuild(B_VILLAGE_INN));
	if(village->v_buildings[0] != B_NONE)
		tobuild.push_back(ToBuild(village->v_buildings[0]));
	if(village->v_buildings[1] != B_NONE)
		tobuild.push_back(ToBuild(village->v_buildings[1]));
	std::random_shuffle(tobuild.begin()+1, tobuild.end(), myrand); // losowa kolejno�� budynk�w (opr�cz Village Hall)

	const BUILDING cottage[] = {B_COTTAGE, B_COTTAGE2, B_COTTAGE3};
	tobuild.push_back(ToBuild(B_BARRACKS));

	for(int i=0; i<village->citizens; ++i)
		tobuild.push_back(ToBuild(cottage[rand2()%3], false));

	gen->GenerateBuildings(tobuild);
	LoadingStep();
	gen->GenerateFields();
	LoadingStep();
	gen->SmoothTerrain();
	LoadingStep();
	gen->FlattenRoad();
	LoadingStep();

	g_have_well = false;

	City* city = (City*)location;

	// budynki
	city->buildings.resize(tobuild.size());
	vector<ToBuild>::iterator build_it = tobuild.begin();
	for(vector<CityBuilding>::iterator it = city->buildings.begin(), end = city->buildings.end(); it != end; ++it, ++build_it)
	{
		it->type = build_it->type;
		it->pt = build_it->pt;
		it->rot = build_it->rot;
		it->unit_pt = build_it->unit_pt;
	}

	gen->CleanBorders();
	LoadingStep();
}

void Game::GetCityEntry(VEC3& pos, float& rot)
{
	if(city_ctx->entry_points.size() == 1)
	{
		pos = VEC3_x0y(city_ctx->entry_points[0].spawn_area.Midpoint());
		rot = city_ctx->entry_points[0].spawn_rot;
	}
	else
	{
		// check which spawn rot i closest to entry rot
		float best_dif = 999.f;
		int best_index = -1, index = 0;
		float dir = clip(-world_dir+PI/2);
		for(vector<EntryPoint>::iterator it = city_ctx->entry_points.begin(), end = city_ctx->entry_points.end(); it != end; ++it, ++index)
		{
			float dif = angle_dif(dir, it->spawn_rot);
			if(dif < best_dif)
			{
				best_dif = dif;
				best_index = index;
			}
		}
		pos = VEC3_x0y(city_ctx->entry_points[best_index].spawn_area.Midpoint());
		rot = city_ctx->entry_points[best_index].spawn_rot;
	}

	terrain->SetH(pos);
}

void Game::SetExitWorldDir()
{
	// set world_dir
	const float mini = 32.f;
	const float maxi = 256-32.f;
	GAME_DIR best_dir = (GAME_DIR)-1;
	float best_dist=999.f, dist;
	VEC2 close_pt, pt;
	// check right
	dist = GetClosestPointOnLineSegment(VEC2(maxi, mini), VEC2(maxi, maxi), VEC2(leader->pos.x, leader->pos.z), pt);
	if(dist < best_dist)
	{
		best_dist = dist;
		best_dir = GDIR_RIGHT;
		close_pt = pt;
	}
	// check left
	dist = GetClosestPointOnLineSegment(VEC2(mini, mini), VEC2(mini, maxi), VEC2(leader->pos.x, leader->pos.z), pt);
	if(dist < best_dist)
	{
		best_dist = dist;
		best_dir = GDIR_LEFT;
		close_pt = pt;
	}
	// check bottom
	dist = GetClosestPointOnLineSegment(VEC2(mini, mini), VEC2(maxi, mini), VEC2(leader->pos.x, leader->pos.z), pt);
	if(dist < best_dist)
	{
		best_dist = dist;
		best_dir = GDIR_DOWN;
		close_pt = pt;
	}
	// check top
	dist = GetClosestPointOnLineSegment(VEC2(mini, maxi), VEC2(maxi, maxi), VEC2(leader->pos.x, leader->pos.z), pt);
	if(dist < best_dist)
	{
		best_dist = dist;
		best_dir = GDIR_UP;
		close_pt = pt;
	}

	if(close_pt.x < 33.f)
		world_dir = lerp(3.f/4.f*PI, 5.f/4.f*PI, 1.f-(close_pt.y-33.f)/(256.f-66.f));
	else if(close_pt.x > 256.f-33.f)
	{
		if(close_pt.y > 128.f)
			world_dir = lerp(0.f, 1.f/4*PI, (close_pt.y-128.f)/(256.f-128.f-33.f));
		else
			world_dir = lerp(7.f/4*PI, PI*2, (close_pt.y-33.f)/(256.f-128.f-33.f));
	}
	else if(close_pt.y < 33.f)
		world_dir = lerp(5.f/4*PI, 7.f/4*PI, (close_pt.x-33.f)/(256.f-66.f));
	else
		world_dir = lerp(1.f/4*PI, 3.f/4*PI, 1.f-(close_pt.x-33.f)/(256.f-66.f));
}
