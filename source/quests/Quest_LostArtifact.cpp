#include "Pch.h"
#include "GameCore.h"
#include "Quest_LostArtifact.h"
#include "Game.h"
#include "Journal.h"
#include "GameFile.h"
#include "QuestManager.h"
#include "World.h"
#include "LevelArea.h"
#include "Team.h"
#include "ItemHelper.h"
#include "SaveState.h"

//=================================================================================================
void Quest_LostArtifact::Start()
{
	type = Q_LOST_ARTIFACT;
	category = QuestCategory::Random;
	start_loc = world->GetCurrentLocationIndex();
	item = OtherItem::artifacts[Rand() % OtherItem::artifacts.size()];
}

//=================================================================================================
GameDialog* Quest_LostArtifact::GetDialog(int type2)
{
	switch(type2)
	{
	case QUEST_DIALOG_START:
		return GameDialog::TryGet("q_lost_artifact_start");
	case QUEST_DIALOG_NEXT:
		return GameDialog::TryGet("q_lost_artifact_end");
	case QUEST_DIALOG_FAIL:
		return GameDialog::TryGet("q_lost_artifact_timeout");
	default:
		assert(0);
		return nullptr;
	}
}

//=================================================================================================
void Quest_LostArtifact::SetProgress(int prog2)
{
	prog = prog2;
	switch(prog2)
	{
	case Progress::Started:
		{
			OnStart(game->txQuest[106]);
			quest_mgr->quests_timeout.push_back(this);

			item->CreateCopy(quest_item);
			quest_item.id = Format("$%s", item->id.c_str());
			quest_item.quest_id = id;

			Location& sl = GetStartLocation();

			// event
			spawn_item = Quest_Dungeon::Item_OnGround;
			item_to_give[0] = &quest_item;
			if(Rand() % 2 == 0)
				target_loc = world->GetClosestLocation(L_DUNGEON, sl.pos, { HERO_CRYPT, MONSTER_CRYPT });
			else
				target_loc = world->GetClosestLocation(L_DUNGEON, sl.pos, LABYRINTH, F_EXCLUDED);
			Location& tl = GetTargetLocation();
			at_level = tl.GetRandomLevel();

			tl.active_quest = this;
			tl.SetKnown();
			st = tl.st;

			cstring level;
			switch(at_level)
			{
			case 0:
				level = game->txQuest[107];
				break;
			case 1:
				level = game->txQuest[108];
				break;
			case 2:
				level = game->txQuest[109];
				break;
			case 3:
				level = game->txQuest[110];
				break;
			case 4:
				level = game->txQuest[111];
				break;
			case 5:
				level = game->txQuest[112];
				break;
			default:
				level = game->txQuest[113];
				break;
			}

			DialogContext::current->talker->temporary = false;

			msgs.push_back(Format(game->txQuest[82], sl.name.c_str(), world->GetDate()));
			msgs.push_back(Format(game->txQuest[114], item->name.c_str(), level, tl.name.c_str(), GetLocationDirName(sl.pos, tl.pos)));
		}
		break;
	case Progress::Finished:
		{
			state = Quest::Completed;
			if(target_loc != -1)
			{
				Location& loc = GetTargetLocation();
				if(loc.active_quest == this)
					loc.active_quest = nullptr;
			}
			RemoveElementTry<Quest_Dungeon*>(quest_mgr->quests_timeout, this);
			OnUpdate(game->txQuest[115]);
			int reward = GetReward();
			team->AddReward(reward, reward * 3);
			DialogContext::current->talker->temporary = true;
			DialogContext::current->talker->AddItem(&quest_item, 1, true);
			DialogContext::current->pc->unit->RemoveQuestItem(id);
		}
		break;
	case Progress::Timeout:
		{
			state = Quest::Failed;
			if(target_loc != -1)
			{
				Location& loc = GetTargetLocation();
				if(loc.active_quest == this)
					loc.active_quest = nullptr;
			}
			RemoveElementTry<Quest_Dungeon*>(quest_mgr->quests_timeout, this);
			OnUpdate(game->txQuest[116]);
			DialogContext::current->talker->temporary = true;
		}
		break;
	}
}

//=================================================================================================
cstring Quest_LostArtifact::FormatString(const string& str)
{
	if(str == "przedmiot")
		return item->name.c_str();
	else if(str == "target_loc")
		return GetTargetLocationName();
	else if(str == "target_dir")
		return GetLocationDirName(GetStartLocation().pos, GetTargetLocation().pos);
	else if(str == "random_loc")
		return world->GetRandomSettlement(start_loc)->name.c_str();
	else if(str == "poziomie")
	{
		switch(at_level)
		{
		case 0:
			return game->txQuest[117];
		case 1:
			return game->txQuest[118];
		case 2:
			return game->txQuest[119];
		case 3:
			return game->txQuest[120];
		case 4:
			return game->txQuest[121];
		case 5:
			return game->txQuest[122];
		default:
			return game->txQuest[123];
		}
	}
	else if(str == "reward")
		return Format("%d", GetReward());
	else
	{
		assert(0);
		return nullptr;
	}
}

//=================================================================================================
bool Quest_LostArtifact::IsTimedout() const
{
	return world->GetWorldtime() - start_time > 60;
}

//=================================================================================================
bool Quest_LostArtifact::OnTimeout(TimeoutType ttype)
{
	if(done)
		ForLocation(target_loc, at_level)->RemoveQuestGroundItem(id);

	OnUpdate(game->txQuest[277]);
	return true;
}

//=================================================================================================
bool Quest_LostArtifact::IfHaveQuestItem2(cstring id) const
{
	return prog == Progress::Started && strcmp(id, "$$lost_item") == 0;
}

//=================================================================================================
const Item* Quest_LostArtifact::GetQuestItem()
{
	return &quest_item;
}

//=================================================================================================
void Quest_LostArtifact::Save(GameWriter& f)
{
	Quest_Dungeon::Save(f);

	f << item;
	f << st;
}

//=================================================================================================
bool Quest_LostArtifact::Load(GameReader& f)
{
	Quest_Dungeon::Load(f);

	f >> item;
	if(LOAD_VERSION >= V_0_8)
		f >> st;
	else if(target_loc != -1)
		st = GetTargetLocation().st;
	else
		st = 10;

	if(prog >= Progress::Started)
	{
		item->CreateCopy(quest_item);
		quest_item.id = Format("$%s", item->id.c_str());
		quest_item.quest_id = id;
		spawn_item = Quest_Dungeon::Item_OnGround;
		item_to_give[0] = &quest_item;
	}

	return true;
}

//=================================================================================================
int Quest_LostArtifact::GetReward() const
{
	return ItemHelper::CalculateReward(st, Int2(5, 15), Int2(2000, 6000));
}
