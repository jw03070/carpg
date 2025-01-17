#include "Pch.h"
#include "GameCore.h"
#include "PickServerPanel.h"
#include "Language.h"
#include "Input.h"
#include "Const.h"
#include "Game.h"
#include "Version.h"
#include "BitStreamFunc.h"
#include "ResourceManager.h"
#include "GameGui.h"
#include "LobbyApi.h"
#include <json.hpp>

//=================================================================================================
PickServerPanel::PickServerPanel(const DialogInfo& info) : DialogBox(info), pick_autojoin(false)
{
	size = Int2(524, 340);
	bts.resize(2);

	bts[0].size = Int2(180, 44);
	bts[0].pos = Int2(336, 30);
	bts[0].id = IdOk;
	bts[0].parent = this;

	bts[1].size = Int2(180, 44);
	bts[1].pos = Int2(336, 80);
	bts[1].id = IdCancel;
	bts[1].parent = this;

	cb_internet.id = IdInternet;
	cb_internet.radiobox = true;
	cb_internet.bt_size = Int2(32, 32);
	cb_internet.parent = this;
	cb_internet.pos = Int2(336, 130);
	cb_internet.size = Int2(200, 32);

	cb_lan.id = IdLan;
	cb_lan.radiobox = true;
	cb_lan.bt_size = Int2(32, 32);
	cb_lan.parent = this;
	cb_lan.pos = Int2(336, 170);
	cb_lan.size = Int2(200, 32);

	grid.pos = Int2(8, 8);
	grid.size = Int2(320, 300);
	grid.event = GridEvent(this, &PickServerPanel::GetCell);
	grid.selection_type = Grid::BACKGROUND;
	grid.selection_color = Color(0, 255, 0, 128);
}

//=================================================================================================
void PickServerPanel::LoadLanguage()
{
	auto s = Language::GetSection("PickServerPanel");
	txFailedToGetServers = s.Get("failedToGetServers");
	txInvalidServerVersion = s.Get("invalidServerVersion");

	bts[0].text = s.Get("join");
	bts[1].text = gui->txCancel;

	cb_internet.text = s.Get("internet");
	cb_lan.text = s.Get("lan");

	grid.AddColumn(Grid::IMGSET, 50);
	grid.AddColumn(Grid::TEXT_COLOR, 100, s.Get("players"));
	grid.AddColumn(Grid::TEXT_COLOR, 150, s.Get("name"));
	grid.Init();
}

//=================================================================================================
void PickServerPanel::LoadData()
{
	tIcoSave = res_mgr->Load<Texture>("save-16.png");
	tIcoPassword = res_mgr->Load<Texture>("padlock-16.png");
}

//=================================================================================================
void PickServerPanel::Draw(ControlDrawData*)
{
	DrawPanel();

	// controls
	for(int i = 0; i < 2; ++i)
		bts[i].Draw();
	cb_internet.Draw();
	cb_lan.Draw();
	grid.Draw();
}

//=================================================================================================
void PickServerPanel::Update(float dt)
{
	// update gui
	for(int i = 0; i < 2; ++i)
	{
		bts[i].mouse_focus = focus;
		bts[i].Update(dt);
	}
	cb_internet.mouse_focus = focus;
	cb_internet.Update(dt);
	cb_lan.mouse_focus = focus;
	cb_lan.Update(dt);
	grid.focus = focus;
	grid.Update(dt);

	if(!focus)
		return;

	if(input->Focus() && input->PressedRelease(Key::Escape))
	{
		Event((GuiEvent)(IdCancel));
		return;
	}

	// ping servers
	timer += dt;
	if(timer >= 1.f)
	{
		if(lan_mode)
		{
			net->peer->Ping("255.255.255.255", (word)net->port, false);
			timer = 0;
		}
		else if(!bad_request)
		{
			if(timer > 30.f)
				api->GetServers();
			else
				api->GetChanges();
			timer = 0;
		}
	}

	// listen for packets
	Packet* packet;
	for(packet = net->peer->Receive(); packet; net->peer->DeallocatePacket(packet), packet = net->peer->Receive())
	{
		BitStreamReader reader(packet);
		byte msg_id;
		reader >> msg_id;

		switch(msg_id)
		{
		case ID_UNCONNECTED_PONG:
			if(lan_mode)
			{
				// header
				TimeMS time_ms;
				char sign[2];
				reader >> time_ms;
				reader >> sign;
				if(!reader || sign[0] != 'C' || sign[1] != 'A')
				{
					// someone responded but this is not carpg server or game already started and we are ignored
					break;
				}

				// info about server
				uint version;
				byte active_players, players_max, flags;
				reader >> version;
				reader >> active_players;
				reader >> players_max;
				reader >> flags;
				const string& server_name = reader.ReadString1();
				if(!reader)
				{
					Warn("PickServer: Broken response from %.", packet->systemAddress.ToString());
					break;
				}

				// search for server in list
				bool found = false;
				int index = 0;
				for(vector<ServerData>::iterator it = servers.begin(), end = servers.end(); it != end; ++it, ++index)
				{
					if(it->adr == packet->systemAddress)
					{
						// update
						found = true;
						Info("PickServer: Updated server %s (%s).", it->name.c_str(), it->adr.ToString());
						it->name = server_name;
						it->active_players = active_players;
						it->max_players = players_max;
						it->flags = flags;
						it->timer = 0.f;
						it->version = version;

						CheckAutojoin();
						break;
					}
				}

				if(!found)
				{
					// add to servers list
					Info("PickServer: Added server %s (%s).", server_name.c_str(), packet->systemAddress.ToString());
					ServerData& sd = Add1(servers);
					sd.id = -1;
					sd.name = server_name;
					sd.active_players = active_players;
					sd.max_players = players_max;
					sd.adr = packet->systemAddress;
					sd.flags = flags;
					sd.timer = 0.f;
					sd.version = version;
					grid.AddItem();

					CheckAutojoin();
				}
			}
			break;
		case ID_DISCONNECTION_NOTIFICATION:
			// when client was connecting to server using master server and have invalid password
			// disconnecting from server will be received here, ignore message
			break;
		default:
			Warn("PickServer: Unknown packet %d from %s.", msg_id, packet->systemAddress.ToString());
			break;
		}
	}

	// update servers
	if(lan_mode)
	{
		int index = 0;
		for(vector<ServerData>::iterator it = servers.begin(), end = servers.end(); it != end;)
		{
			it->timer += dt;
			if(it->timer >= 2.f)
			{
				Info("PickServer: Removed server %s (%s).", it->name.c_str(), it->adr.ToString());
				grid.RemoveItem(index);
				it = servers.erase(it);
				end = servers.end();
			}
			else
			{
				++it;
				++index;
			}
		}
	}

	// enable/disable join button
	if(grid.selected == -1)
		bts[0].state = Button::DISABLED;
	else if(bts[0].state == Button::DISABLED)
		bts[0].state = Button::NONE;
}

//=================================================================================================
void PickServerPanel::Event(GuiEvent e)
{
	switch(e)
	{
	case GuiEvent_Show:
	case GuiEvent_WindowResize:
		if(e == GuiEvent_Show)
			visible = true;
		pos = global_pos = (gui->wnd_size - size) / 2;
		for(int i = 0; i < 2; ++i)
			bts[i].global_pos = global_pos + bts[i].pos;
		cb_internet.global_pos = global_pos + cb_internet.pos;
		cb_lan.global_pos = global_pos + cb_lan.pos;
		grid.Move(global_pos);
		break;
	case GuiEvent_Close:
		visible = false;
		grid.LostFocus();
		break;
	case GuiEvent_LostFocus:
		grid.LostFocus();
		break;
	case IdOk:
		if(servers[grid.selected].IsValidVersion())
			event(e);
		else
			gui->SimpleDialog(Format(txInvalidServerVersion, VersionToString(servers[grid.selected].version), VERSION_STR), this);
		break;
	case IdCancel:
		net->ClosePeer();
		net->peer->Shutdown(0);
		CloseDialog();
		break;
	case IdInternet:
		cb_lan.checked = false;
		OnChangeMode(false);
		break;
	case IdLan:
		cb_internet.checked = false;
		OnChangeMode(true);
		break;
	}
}

//=================================================================================================
void PickServerPanel::Show(bool pick_autojoin)
{
	this->pick_autojoin = pick_autojoin;

	try
	{
		net->InitClient();
	}
	catch(cstring err)
	{
		gui->SimpleDialog(err, (Control*)game_gui->main_menu);
		return;
	}

	if(net->join_lan)
	{
		Info("Pinging LAN servers...");
		lan_mode = true;
		cb_internet.checked = false;
		cb_lan.checked = true;
		net->peer->Ping("255.255.255.255", (word)net->port, false);
	}
	else
	{
		Info("Getting servers from master server.");
		lan_mode = false;
		cb_internet.checked = true;
		cb_lan.checked = false;
		api->Reset();
		api->GetServers();
	}

	bad_request = false;
	timer = 0;
	servers.clear();
	grid.Reset();

	gui->ShowDialog(this);
}

//=================================================================================================
void PickServerPanel::GetCell(int item, int column, Cell& cell)
{
	ServerData& server = servers[item];

	if(column == 0)
	{
		vector<Texture*>& imgs = *cell.imgset;
		if(IsSet(server.flags, SERVER_PASSWORD))
			imgs.push_back(tIcoPassword);
		if(IsSet(server.flags, SERVER_SAVED))
			imgs.push_back(tIcoSave);
	}
	else
	{
		cell.text_color->color = (server.IsValidVersion() ? Color::Black : Color::Red);
		if(column == 1)
			cell.text_color->text = Format("%d/%d", server.active_players, server.max_players);
		else
			cell.text_color->text = server.name.c_str();
	}
}

//=================================================================================================
void PickServerPanel::OnChangeMode(bool lan_mode)
{
	this->lan_mode = lan_mode;
	if(lan_mode)
	{
		api->Reset();
		net->peer->Ping("255.255.255.255", (word)net->port, false);
	}
	else
	{
		api->GetServers();
		bad_request = false;
	}
	net->join_lan = lan_mode;
	game->cfg.Add("join_lan", lan_mode);
	game->SaveCfg();
	servers.clear();
	grid.Reset();
	timer = 0;
}

//=================================================================================================
bool PickServerPanel::HandleGetServers(nlohmann::json& j)
{
	if(!visible || lan_mode || gui->HaveDialog("GetTextDialog"))
		return false;

	auto& servers = j["servers"];
	for(auto it = servers.begin(), end = servers.end(); it != end; ++it)
		AddServer(*it);

	CheckAutojoin();
	return true;
}

//=================================================================================================
void PickServerPanel::AddServer(nlohmann::json& server)
{
	ServerData& sd = Add1(servers);
	sd.id = server["id"].get<int>();
	sd.guid = server["guid"].get_ref<string&>();
	sd.name = server["name"].get_ref<string&>();
	sd.active_players = server["players"].get<int>();
	sd.max_players = server["maxPlayers"].get<int>();
	sd.flags = server["flags"].get<int>();
	sd.version = server["version"].get<int>();
	sd.timer = 0.f;
	grid.AddItem();
	Info("PickServer: Added server %s (%d).", sd.name.c_str(), sd.id);
}

//=================================================================================================
bool PickServerPanel::HandleGetChanges(nlohmann::json& j)
{
	if(!visible || lan_mode || gui->HaveDialog("GetTextDialog"))
		return false;

	auto& changes = j["changes"];
	for(auto it = changes.begin(), end = changes.end(); it != end; ++it)
	{
		auto& change = *it;
		int type = change["type"].get<int>();
		switch(type)
		{
		case 0: // add server
			AddServer(change["server"]);
			break;
		case 1: // update server
			{
				auto& server = change["server"];
				int id = server["id"].get<int>();
				int players = server["players"].get<int>();
				bool found = false;
				for(ServerData& sd : servers)
				{
					if(sd.id == id)
					{
						sd.active_players = players;
						found = true;
						Info("PickServer: Updated server %s (%d).", sd.name.c_str(), id);
						break;
					}
				}
				if(!found)
					Error("PickServer: Missing server %d to update.", id);
			}
			break;
		case 2: // remove server
			{
				int id = change["serverID"].get<int>();
				int index = 0;
				bool found = false;
				for(auto it2 = servers.begin(), end2 = servers.end(); it2 != end2; ++it2)
				{
					if(it2->id == id)
					{
						Info("PickServer: Removed server %s (%d).", it2->name.c_str(), it2->id);
						found = true;
						grid.RemoveItem(index);
						servers.erase(it2);
						break;
					}
					++index;
				}
				if(!found)
					Error("PickServer: Missing server %d to remove.", id);
			}
			break;
		}
	}

	CheckAutojoin();
	return true;
}

//=================================================================================================
void PickServerPanel::CheckAutojoin()
{
	if(!pick_autojoin)
		return;
	int index = 0;
	for(ServerData& sd : servers)
	{
		if(sd.active_players != sd.max_players && sd.IsValidVersion())
		{
			// autojoin server
			bts[0].state = Button::NONE;
			pick_autojoin = false;
			grid.selected = index;
			Event(GuiEvent(IdOk));
		}
		++index;
	}
}

//=================================================================================================
void PickServerPanel::HandleBadRequest()
{
	bad_request = true;
	gui->SimpleDialog(txFailedToGetServers, this);
}
