#include "internalOp.h"
#include "connexion.h"
#include "Stream.h"
#include "itemEnums.h"
#include "connexionEnums.h"
#include "serverEnums.h"
#include "job.h"
#include "opcodeEnum.h"
#include "arbiter_server.h"
#include "chat.h"
#include "chatEnums.h"
#include "chat_data.h"

void WINAPI io_init() {
	io_fn[IO_REMOVE_PLAYER] = io_remove_player;
	io_fn[IO_LOAD_TOPO_FIN] = io_load_topo_fin;
	io_fn[IO_CHAT] = io_chat;
	return;
}

void WINAPI io_exectue(e_io_opcodes opcode, std::shared_ptr<connection> c, Stream &data) {
	if (opcode < IO_MAX && io_fn[opcode]) {
		io_fn[opcode](c, data);
	}
	return;
}

void WINAPI io_load_topo_fin(std::shared_ptr<connection> c, Stream &data_0) {
	std::shared_ptr<player> p = c->players[c->selected_player];
	std::vector<uint32> p_l;
	uint32 count = data_0.ReadUInt32();
	for (uint32 i = 0; i < count; i++) {
		p_l.push_back(data_0.ReadUInt32());
	}

	if (count) {
		Stream s_y;

		for (size_t i = 0; i < p_l.size(); i++) {
			s_y.Clear();
			player_write_spawn_packet(arbiter_get_player(p_l[0]), s_y);
			connection_send(c, &s_y);
		}
	}

	//if (!result) {
	//	arbiter_server_connexion_remove(c->_id);
	//
	//}


	std::unique_ptr<Stream> data = std::make_unique<Stream>();

	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_ONGOING_LEVEL_EVENT_LIST);
	connection_send(c, data.get());



	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_ONGOING_HUNTING_EVENT_LIST);
	connection_send(c, data.get());


	//data->Clear();
	//data->Resize(8);
	//data->WriteInt16(8);
	//data->WriteInt16(S_ADD_AWAKEN_ENCHANT_DATA);
	//connection_send(c, data.get());
	//	
	//
	//data->Clear();
	//data->Resize(8);
	//data->WriteInt16(8);
	//data->WriteInt16(S_ADD_AWAKEN_CHANGE_DATA);
	//connection_send(c, data.get());
	//	

	//data->Clear();
	//data->Resize(8);
	//data->WriteInt16(8);
	//data->WriteInt16(S_GUILD_QUEST_LIST);
	//connection_send(c, data.get());
	//	


	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_VISITED_SECTION_LIST);
	connection_send(c, data.get());


	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_REQUEST_INVITE_GUILD_TAG);
	connection_send(c, data.get());


	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_USER_BLOCK_LIST);
	connection_send(c, data.get());


	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_FRIEND_GROUP_LIST);
	connection_send(c, data.get());


	data->Clear();
	data->WriteInt16(0);
	data->WriteInt16(S_FRIEND_LIST);
	data->WriteInt32(0);
	short name_pos = data->NextPos();
	data->WritePos(name_pos);
	data->WriteString("[SKY-LAKE Server]");
	data->WritePos(0);
	connection_send(c, data.get());


	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_UPDATE_FRIEND_INFO);
	data->WriteInt32(0);
	connection_send(c, data.get());


	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_EP_SYSTEM_DAILY_EVENT_EXP_ON_OFF);
	connection_send(c, data.get());



	data->Clear();
	data->Resize(28);
	data->WriteInt16(28);
	data->WriteInt16(S_SPAWN_ME);
	data->WriteWorldId(p);
	data->WriteFloat(a_load(p->x));
	data->WriteFloat(a_load(p->y));
	data->WriteFloat(a_load(p->z));
	data->WriteInt16(a_load(p->w));
	data->WriteInt16(1); //alive
	connection_send(c, data.get());





	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_ATTENDANCE_EVENT_REWARD_COUNT);
	connection_send(c, data.get());



	data->Clear();
	data->Resize(9);
	data->WriteInt16(9);
	data->WriteInt16(S_ACCOUNT_BENEFIT_LIST);
	connection_send(c, data.get());



#pragma region stats
	player_recalculate_stats(p);
#pragma endregion

	data->Clear();
	data->Resize(20);
	data->WriteInt16(20);
	data->WriteInt16(S_PLAYER_CHANGE_ALL_PROF);
	connection_send(c, data.get());



	data->Clear();
	data->Resize(12);
	data->WriteInt16(12);
	data->WriteInt16(S_PARCEL_READ_RECV_STATUS);
	connection_send(c, data.get());



	data->Clear();
	data->Resize(28);
	data->WriteInt16(28);
	data->WriteInt16(S_F2P_PremiumUser_Permission);
	data->WriteInt32(0);
	data->WriteInt32(5);
	data->WriteFloat(1.0f);
	data->WriteFloat(1.0f);
	data->WriteInt32(20);
	data->WriteInt32(1);
	connection_send(c, data.get());


	data->Clear();
	data->Resize(17);
	data->WriteInt16(17);
	data->WriteInt16(S_USER_STATUS);
	data->WriteWorldId(p);
	//data->WriteInt32(0);
	//data->WriteByte(0);
	connection_send(c, data.get());


	//data->Clear();
	//data->Resize(16);
	//data->WriteInt16(16);
	//data->WriteInt16(S_CREST_INFO);
	//connection_send(c, data.get());
	//	

	return;
}

void WINAPI io_player_enter_world(std::shared_ptr<connection> c, uint8 result, std::vector<uint32>& p_l) {
	std::unique_ptr<Stream> data = std::make_unique<Stream>();
	std::shared_ptr<player> p = c->players[c->selected_player];
	c->inLobby = false;

	data->Clear();
	data->Resize(13);
	data->WriteInt16(13);
	data->WriteInt16(S_SELECT_USER);
	data->WriteUInt8(result);
	connection_send(c, data.get());

	if (!result)
		return;

	data->Clear();
	data->Resize(9);
	data->WriteInt16(9);
	data->WriteInt16(S_UPDATE_CONTENTS_ON_OFF);
	data->WriteInt32(2);
	connection_send(c, data.get());


	data->Clear();
	data->Resize(9);
	data->WriteInt16(9);
	data->WriteInt16(S_UPDATE_CONTENTS_ON_OFF);
	data->WriteInt32(3);
	connection_send(c, data.get());


	data->Clear();
	data->Resize(9);
	data->WriteInt16(9);
	data->WriteInt16(S_UPDATE_CONTENTS_ON_OFF);
	data->WriteInt32(4);
	connection_send(c, data.get());


	data->Clear();
	data->Resize(9);
	data->WriteInt16(9);
	data->WriteInt16(S_UPDATE_CONTENTS_ON_OFF);
	data->WriteInt32(8);
	connection_send(c, data.get());


	data->Clear();
	data->Resize(9);
	data->WriteInt16(9);
	data->WriteInt16(S_UPDATE_CONTENTS_ON_OFF);
	data->WriteInt32(9);
	connection_send(c, data.get());



	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_BROCAST_GUILD_FLAG);
	connection_send(c, data.get());



#pragma region login
	uint16 login_size = 290;
	data->Clear();
	data->Resize(login_size);

	data->WriteInt16(0);
	data->WriteInt16(S_LOGIN);

	uint16 name_pos = data->_pos;
	data->_pos += 2;

	uint16 details1_pos = data->_pos;
	data->_pos += 2;
	data->WriteInt16(SC_PLAYER_DETAILS_1_BUFFER_SIZE);

	uint16 details2_pos = data->_pos;
	data->_pos += 2;
	data->WriteInt16(SC_PLAYER_DETAILS_2_BUFFER_SIZE);

	data->WriteInt32(p->model);
	//18

	data->WriteWorldId(p);
	data->WriteInt32(SERVER_ID);
	data->WriteInt32(p->dbid);


	data->WriteInt32(0);   //unk
	data->WriteUInt8(1); //alive
	data->WriteInt32(0);   //??
	data->WriteInt32(52);  //??
	data->WriteInt32(110); //??
	data->Write(p->details3, SC_PLAYER_DETAILS_3_BUFFER_SIZE); //int64 data [appearance]
															   //59
	data->WriteInt16(1); //unk
	data->WriteInt16((uint16)p->level);
	//63
	data->WriteInt32(56);
	data->WriteInt16(66);
	data->WriteInt16(3); //unk
	data->WriteInt32(1);
	data->WriteInt32(0);
	data->WriteInt16(0);	//unk


	data->WriteInt64(p->restedExp); //rested exp?
	data->WriteInt64(p->exp); //player exp
	data->WriteInt64(840); //next level exp
						   //105
						   //data->WriteInt64(0);	//unk
						   //data->WriteInt64(0);	//unk
						   //data->WriteInt32(0); //restedCurrent
						   //data->WriteInt32(0); //restedMax
						   //data->WriteInt32(0);	//unk
						   //data->WriteInt32(0);	//unk

	data->WriteInt32(0);
	data->WriteInt32(0);
	data->WriteInt32(0);
	data->WriteInt32(0);

	data->WriteInt32(79497064); //??
	data->WriteInt32(79497064);
	data->WriteFloat(1);
	data->WriteInt32(0); //?float???

	data->WriteInt32(p->i_.get_profile_item(PROFILE_WEAPON));
	data->WriteInt32(p->i_.get_profile_item(PROFILE_ARMOR));
	data->WriteInt32(p->i_.get_profile_item(PROFILE_GLOVES));
	data->WriteInt32(p->i_.get_profile_item(PROFILE_BOOTS));

	data->WriteInt32(p->i_.get_profile_item(PROFILE_INNERWARE));
	data->WriteInt32(p->i_.get_profile_item(PROFILE_HEAD_ADRONMENT)); //face
	data->WriteInt32(p->i_.get_profile_item(PROFILE_MASK)); //head
															//165 good
	data->WriteInt64(30418140); //play time

	data->WriteInt64(1); //unk
						 //181
						 //data->WriteInt32(0); //reaper? 03 00 00 00 în cazul în care 3, cuvintele "Îngerul Morții"
						 //data->WriteInt32(0);
						 //data->WriteInt32(0);  // 00 00 00 00
						 //data->WriteInt32(0);  // 00 00 00 00
						 //data->WriteInt32(0);  // 00 00 00 00
						 //data->WriteInt32(0);  // 00 00 00 00
						 //data->WriteInt32(0);  // 00 00 00 00
						 //data->WriteInt32(0);  // 00 00 00 00
						 //data->WriteInt32(0);  // 00 00 00 00
						 //data->WriteInt32(0);  // 00 00 00 00
						 //data->WriteInt32(0);  // 00 00 00 00
						 //data->WriteInt32(0);  // 00 00 00 00
						 //data->WriteInt32(0);  // 00 00 00 00

	data->WriteUInt8(0); //chat restricted
	data->WriteInt32(1799); //??? 1799 //title

	data->WriteInt32(0);
	data->WriteInt32(0);
	data->WriteInt32(0);
	data->WriteInt32(0);
	data->WriteInt32(0);
	data->WriteInt32(0);
	data->WriteInt32(0);
	data->WriteInt32(0);
	data->WriteInt32(0);
	data->WriteInt32(0); //chat restricted time?
	data->WriteInt32(0);
	data->WriteInt32(0);
	data->WriteInt32(0); //??
	data->WriteInt32(0); //pixie

	data->WriteUInt8(0);
	data->WriteUInt8(1);

	data->WriteInt32(0); //skins   ??
	data->WriteInt32(0); //skins   ??
	data->WriteInt32(0); //skins   ??
	data->WriteInt32(0); //weapon skins
	data->WriteInt32(0); //body skins
	data->WriteInt32(0); //skins

	data->WriteUInt8(1);

	data->WriteInt32(10023);
	data->WriteInt32(566);
	data->WriteInt32(100);
	data->WriteFloat(1.0f);
	data->WriteInt32(0);
	data->WriteUInt8(0);

	data->WritePos(name_pos);
	data->WriteString(p->name);

	data->WritePos(details1_pos);
	data->Write(p->details1, SC_PLAYER_DETAILS_1_BUFFER_SIZE);

	data->WritePos(details2_pos);
	data->Write(p->details2, SC_PLAYER_DETAILS_2_BUFFER_SIZE);

	data->WritePos(0);
	connection_send(c, data.get());


#pragma endregion

	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_SHOW_NPC_TO_MAP);
	connection_send(c, data.get());


	//account_load_client_settings(c);
	//player_load_user_settings(p, (sql::Connection*)argv[0]);

	p->i_.send();

	//int last = 8;
	//data->Clear();
	//data->Resize(15);
	//data->WriteInt16(0);
	//data->WriteInt16(S_SKILL_LIST);
	//data->WriteInt16(p->skills.active.size() + p->skills.passive.size());
	//data->WriteInt16(8);
	//for (int i = 0; i < p->skills.active.size(); i++)
	//{
	//	data->WritePos(last);
	//	data->WriteInt16(data->_pos);
	//	last = data->_pos;
	//	data->WriteInt16(0);
	//	data->WriteInt32(p->skills.active[i]);
	//	data->WriteUInt8(1);//active
	//}
	//for (int i = 0; i < p->skills.passive.size(); i++)
	//{
	//	data->WritePos(last);
	//	data->WriteInt16(data->_pos);
	//	last = data->_pos;
	//	data->WriteInt16(0);
	//	data->WriteInt32(p->skills.passive[i]);
	//	data->WriteUInt8(0);//passive
	//}
	//data->WritePos(0);
	//connection_send(c,data.get());
	//	

	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_AVAILABLE_SOCIAL_LIST);
	connection_send(c, data.get());



	data->Clear();
	data->Resize(4);
	data->WriteInt16(4);
	data->WriteInt16(S_CLEAR_QUEST_INFO);
	connection_send(c, data.get());


	data->Clear();
	data->Resize(9);
	data->WriteInt16(9);
	data->WriteInt16(S_DAILY_QUEST_COMPLETE_COUNT);
	data->WriteInt16(0);
	data->WriteInt16(10);
	connection_send(c, data.get());


	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_COMPLETED_MISSION_INFO);
	connection_send(c, data.get());

	//				     6     6         1      -1    0      -1      0
	struct a_skill { int a; int b; float c; int d; int e; int f; int g; };

	data->Clear();
	data->Resize(25);
	data->WriteInt16(25);
	data->WriteInt16(S_ARTISAN_SKILL_LIST);
	data->WriteInt16(0);
	data->WriteInt16(0);

	data->WriteUInt8(0);

	data->WriteInt32(0);
	data->WriteInt32(0);
	data->WriteInt16(0);
	data->WriteInt16(17480);
	data->WriteInt32(14);

	connection_send(c, data.get());

	player_send_external_change(p, p_l);

	data->Clear();
	data->Resize(10);
	data->WriteInt16(10);
	data->WriteInt16(S_ARTISAN_RECIPE_LIST);
	data->WriteInt32(0);
	data->WriteUInt8(0);
	data->WriteUInt8(1);
	connection_send(c, data.get());


	//data->Clear();
	//data->Resize(16);
	//data->WriteInt16(16);
	//data->WriteInt16(S_NPCGUILD_LIST);
	//data->WriteInt32(0); // count  and offset 
	//data->WriteWorldId(p);
	//connection_send(c,data.get());
	//	

	data->Clear();
	data->Resize(22);
	data->WriteInt16(22);
	data->WriteInt16(S_PET_INCUBATOR_INFO_CHANGE);
	data->WriteInt32(0);// count  and offset 
	data->WriteInt32(1);
	data->WriteInt32(0);
	data->WriteInt32(60);
	data->WriteInt16(0);
	connection_send(c, data.get());


	data->Clear();
	data->Resize(12);
	data->WriteInt16(12);
	data->WriteInt16(S_VIRTUAL_LATENCY);
	connection_send(c, data.get());


	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_MOVE_DISTANCE_DELTA);
	data->WriteFloat(200.0f);
	connection_send(c, data.get());


	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_MY_DESCRIPTION);
	data->WriteInt32(6);
	connection_send(c, data.get());


	data->Clear();
	data->Resize(28);
	data->WriteInt16(28);
	data->WriteInt16(S_F2P_PremiumUser_Permission);
	data->WriteInt16(1);
	data->WriteInt16(20);
	data->WriteInt32(5);
	data->WriteFloat(1.0f);
	data->WriteFloat(1.0f);

	data->WriteInt32(20);
	data->WriteInt32(1);
	connection_send(c, data.get());


	//TOKEN POINTS


	data->Clear();
	data->Resize(28);
	data->WriteInt16(28);
	data->WriteInt16(S_MASSTIGE_STATUS);
	connection_send(c, data.get());


	data->Clear();
	data->Resize(58);
	data->WriteInt16(58);
	data->WriteInt16(S_AVAILABLE_EVENT_MATCHING_LIST);
	connection_send(c, data.get());


	data->Clear();
	data->Resize(26);
	data->WriteInt16(26);
	data->WriteInt16(S_CURRENT_ELECTION_STATE);
	connection_send(c, data.get());


#pragma region S_USER_ITEM_EQUIP_CHANGER
	short nextPos = 0; int changer[] = { 1,3,4,5,6,7,8,9,10,11,19,20 };

	data->Clear();
	data->Resize(160);
	data->WriteInt16(160);
	data->WriteInt16(S_USER_ITEM_EQUIP_CHANGER);

	data->WriteInt16(12); //count
	nextPos = data->_pos;
	data->_pos += 2;

	data->WriteWorldId(p);

	for (size_t i = 0; i < 12; i++)
	{
		data->WritePos(nextPos);
		data->WriteInt16(data->_pos); //base offset
		nextPos = data->_pos;
		data->_pos += 2;
		data->WriteInt64(changer[i]);
	}

	data->WritePos(0);
	connection_send(c, data.get());

#pragma endregion

	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_FESTIVAL_LIST);
	connection_send(c, data.get());


	data->Clear();
	data->Resize(21);
	data->WriteInt16(21);
	data->WriteInt16(S_LOAD_TOPO);
	data->WriteInt32(p->continent_id);
	data->WriteFloat(a_load(p->x));
	data->WriteFloat(a_load(p->y));
	data->WriteFloat(a_load(p->z));
	//data->WriteByte(0);
	connection_send(c, data.get());


	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_LOAD_HINT);
	connection_send(c, data.get());



	data->Clear();
	data->Resize(9);
	data->WriteInt16(9);
	data->WriteInt16(S_ACCOUNT_BENEFIT_LIST);
	connection_send(c, data.get());


	data->Clear();
	data->Resize(16);
	data->WriteInt16(16);
	data->WriteInt16(S_SEND_USER_PLAY_TIME);
	data->WriteInt32(0); //online time [s]
	data->WriteInt64(1484999153); //creation time
	connection_send(c, data.get());



	data->Clear();
	data->Resize(12);
	data->WriteInt16(12);
	data->WriteInt16(S_PCBANGINVENTORY_DATALIST);
	connection_send(c, data.get());


	data->Clear();
	data->Resize(36);
	data->WriteInt16(36);
	data->WriteInt16(S_UPDATE_NPCGUILD);
	data->WriteWorldId(p);
	data->WriteInt32(1);
	data->WriteInt32(9);
	data->WriteInt32(610);
	data->WriteInt32(6);
	data->WriteInt64(0);
	connection_send(c, data.get());



	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_COMPLETED_MISSION_INFO);
	connection_send(c, data.get());



	data->Clear();
	data->Resize(12);
	data->WriteInt16(12);
	data->WriteInt16(S_FATIGABILITY_POINT);
	data->WriteInt32(1);
	data->WriteInt32(20);
	connection_send(c, data.get());


	data->Clear();
	data->Resize(44);
	data->WriteInt16(44);
	data->WriteInt16(S_LOAD_EP_INFO);
	connection_send(c, data.get());


	return;
}

void WINAPI io_remove_player(std::shared_ptr<connection> c, Stream &data) {

	a_store(c->players[c->selected_player]->x, data.ReadFloat());
	a_store(c->players[c->selected_player]->y, data.ReadFloat());
	a_store(c->players[c->selected_player]->z, data.ReadFloat());
	a_store(c->players[c->selected_player]->w, data.ReadInt16());


	uint32 count = data.ReadUInt32();
	if (count) {
		Stream d;
		d.Resize(13);
		d.WriteUInt16(13);
		d.WriteUInt16(S_DESPAWN_USER);
		d.WriteUInt32(c->id);
		d.WriteUInt32(SERVER_ID);
		uint32 c_id;
		for (uint32 i = 0; i < count; i++) {
			arbiter_send(data.ReadUInt32(), &d);
		}
	}

	//perform account update

	c->inLobby = true;
	arbiter_server_connexion_remove(c->id);
	return;
}

void WINAPI io_chat(std::shared_ptr<connection> c, Stream & data) {
	uint32 type = data.ReadUInt32();
	std::string msg = data.ReadUTF16StringBigEdianToASCII();
	byte is_gm = c->account.isGm ? 0x01 : 0x00;
	uint64 time_stamp = data.ReadUInt64();
	std::unique_ptr<chat_message> c_msg = std::make_unique<chat_message>((e_chat_type)type, c->players[c->selected_player], msg, is_gm, time_stamp);

	uint32 count = data.ReadUInt32();
	for (uint32 i = 0; i < count; i++) {
		c_msg->p_l.push_back(data.ReadUInt32());
	}

	if (chat_process_message_async(c_msg.get()))
		c_msg.release();

	return;
}
