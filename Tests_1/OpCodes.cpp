#include "OpCodes.h"
#include "config.h"

#include "stringUtils.h"
#include "connexion.h"
#include "account.h"
#include "servertime.h"
#include "Stream.h"
#include "arbiter_server.h"
#include "world_server.h"
#include "itemEnums.h"
#include "servertime.h"
#include "job.h"
#include "pms.h"
#include "inventory.h"
#include "entity_manager.h"
#include "contract_manager.h"

#include <fstream>
#include <sstream>
#include "chatEnums.h"
#include <unordered_map>

#include "chat_data.h"
#include "chat.h"
#include "itemtemplate.h"
#include "passivitytemplate.h"


#include <inttypes.h>
#include "active_server.h"
#include "skylake_stats.h"
#include "bind_contract.h"
#include "enchant_contract.h"
#include "enigmatic_contract.h"
#include <cppconn/connection.h>

bool opcode_init()
{
	//--------------------------------------------------------------INIT
	opcode_add(C_CHECK_VERSION, op_check_version);
	opcode_add(C_LOGIN_ARBITER, op_login_arbiter);
	opcode_add(C_SET_VISIBLE_RANGE, op_set_visible_range);
	opcode_add(C_GET_USER_LIST, op_get_user_list);
	opcode_add(C_HARDWARE_INFO, op_hardware_info);
	opcode_add(C_REQUEST_VIP_SYSTEM_INFO, op_request_vip_system_info);
	opcode_add(C_GET_USER_GUILD_LOGO, op_get_user_guild_logo);
	opcode_add(C_SAVE_CLIENT_ACCOUNT_SETTING, op_save_client_account_settings);
	opcode_add(C_SAVE_CLIENT_USER_SETTING, op_save_client_user_settings);

	//--------------------------------------------------------------LOBBY
	opcode_add(C_CAN_CREATE_USER, op_can_create_player);
	opcode_add(C_CREATE_USER, op_create_player);
	opcode_add(C_CHANGE_USER_LOBBY_SLOT_ID, op_change_user_slot_id);
	opcode_add(C_CHECK_USERNAME, op_check_username);
	opcode_add(C_STR_EVALUATE_LIST, op_str_evaluate_string);
	opcode_add(C_SELECT_USER, op_select_player);
	opcode_add(C_DELETE_USER, op_delete_player);
	opcode_add(C_CANCEL_DELETE_USER, op_cancel_delete_player);
	opcode_add(C_RETURN_TO_LOBBY, op_return_to_lobby);
	opcode_add(C_EXIT, op_exit);

	//--------------------------------------------------------------WORLD
	opcode_add(C_LOAD_TOPO_FIN, op_load_topo_fin);
	opcode_add(C_SIMPLE_TIP_REPEAT_CHECK, op_simple_tip_repeate_checl);
	opcode_add(C_TRADE_BROKER_HIGHEST_ITEM_LEVEL, op_tradebroker_heighes_item_level);
	opcode_add(C_SERVER_TIME, op_server_time);
	opcode_add(C_UPDATE_CONTENTS_PLAYTIME, op_update_contents_playtime);
	opcode_add(C_REQUEST_INGAMESTORE_PRODUCT_LIST, op_request_ingame_product_list);
	opcode_add(C_PLAYER_LOCATION, op_player_location);
	opcode_add(C_GUARD_PK_POLICY, op_guard_pk_policy);
	opcode_add(C_VISIT_NEW_SECTION, op_visit_new_section);
	opcode_add(C_REIGN_INFO, op_reign_info);
	opcode_add(C_EVENT_GUIDE, op_event_guide);
	opcode_add(C_SET_ITEM_STRING, op_set_item_string);
	opcode_add(C_REQUEST_GAMESTAT_PING, op_request_gamestat_ping);

	//--------------------------------------------------------------MISC
	opcode_add(C_SHOW_INVEN, op_show_inven);
	opcode_add(C_EQUIP_ITEM, op_equipe_item);
	opcode_add(C_UNEQUIP_ITEM, op_unequipe_item);
	opcode_add(C_DUNGEON_CLEAR_COUNT_LIST, op_dungeon_clear_count_list);
	opcode_add(C_DUNGEON_COOL_TIME_LIST, op_dungeon_cooltime_list);
	opcode_add(C_REQUEST_USER_ITEMLEVEL_INFO, op_request_user_itemlevel_info);
	opcode_add(C_NPCGUILD_LIST, op_npc_guild_list);
	opcode_add(C_VIEW_BATTLE_FIELD_RESULT, op_view_battlefield_result);
	opcode_add(C_CHAT, op_chat);
	opcode_add(C_DEL_ITEM, op_del_item);
	opcode_add(C_SHOW_ITEM_TOOLTIP_EX, op_show_item_tooltip_ex);
	opcode_add(C_MOVE_INVEN_POS, op_move_inven_pos);


	//--------------------------------------------------------------CONTRACT
	opcode_add(C_REQUEST_CONTRACT, op_request_contract);
	opcode_add(C_CANCEL_CONTRACT, op_cancel_contract);
	opcode_add(C_BIND_ITEM_BEGIN_PROGRESS, op_bind_item_begin_progress);
	opcode_add(C_BIND_ITEM_EXECUTE, op_bind_item_execute);
	opcode_add(C_EXECUTE_TEMPER, op_execute_temper);
	opcode_add(C_CANCEL_TEMPER, op_cancel_temper);
	opcode_add(C_PLAY_EXECUTE_TEMPER, op_play_execute_temper);
	opcode_add(C_ADD_TO_TEMPER_MATERIAL_EX, op_add_to_temper_material_ex);
	opcode_add(C_CHECK_UNIDENTIFY_ITEMS, op_check_unidentify_items);
	opcode_add(C_RANDOM_PASSIVE_LOCK, op_random_passive_lock);
	opcode_add(C_UNIDENTIFY_EXECUTE, op_unidentify_execute);

	//---------------------------------------------------------------Guild
	opcode_add(C_REQUEST_GUILD_INFO, op_request_guild_info);


	return true;
}

void opcode_release()
{
	opcode_count = 0;
}

bool opcode_add(e_opcode v, op_function f)
{
	opcodes_[v] = f;
	return true;
}

op_function opcode_resolve(e_opcode v)
{
	return opcodes_[v];
}


//--------------------------------------------------------------INIT
bool WINAPI op_check_version(std::shared_ptr<connection> c, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif
	return  true;
}

bool WINAPI op_login_arbiter(std::shared_ptr<connection> c, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	Stream * stream = &c->_recvBuffer.data;

	uint16 nameOffset = stream->ReadInt16();
	uint16 ticketOffset = stream->ReadInt16();
	uint16 ticketSize = stream->ReadInt16();

	int32 unk1 = stream->ReadInt32();
	int8 unk2 = stream->ReadUInt8();
	int32 unk3 = stream->ReadInt32();
	int32 patchVersion = stream->ReadInt32();

	byte ticket[33];
	stream->_pos = ticketOffset - 4;
	stream->Read(ticket, 32);
	ticket[32] = '\0';

	byte username[SC_PLAYER_NAME_MAX_LENGTH];
	stream->_pos = nameOffset - 4;
	stream->ReadASCIIStringTo(username, SC_PLAYER_NAME_MAX_LENGTH);

	bool result = account_perform_login(c, (sql::Connection*)argv[0], ticket, username);
	if (!result)
	{
		printf("AUTH BAD\n");
		arbiter_server_connexion_remove(c->_id);
		return false;
	}

	Stream data = Stream();
	data.Resize(5);
	data.WriteInt16(5);
	data.WriteInt16(S_CHECK_VERSION);
	data.WriteUInt8(1);
	result = connection_send(c, &data);
	if (!result)
		return false;

	data.Clear();
	data.Resize(5);
	data.WriteInt16(5);
	data.WriteInt16(S_LOADING_SCREEN_CONTROL_INFO);
	data.WriteUInt8(0);
	result = connection_send(c, &data);
	if (!result)
		return false;

	data.Clear();
	data.Resize(12);
	data.WriteInt16(12);
	data.WriteInt16(S_REMAIN_PLAY_TIME);
	data.WriteInt32(6);
	data.WriteInt32(0);
	result = connection_send(c, &data);
	if (!result)
		return false;

	data.Clear();
	data.Resize(23);
	data.WriteInt16(23);
	data.WriteInt16(S_LOGIN_ARBITER);
	data.WriteInt16(1);
	data.WriteInt16(0); //unk?
	data.WriteInt32(0);
	data.WriteInt16(0);
	data.WriteInt32(6);
	data.WriteInt32(0);
	data.WriteUInt8(0);
	result = connection_send(c, &data);
	if (!result)
		return false;

	data.Clear();
	data.Resize(14);
	data.WriteInt16(0);
	data.WriteInt16(S_LOGIN_ACCOUNT_INFO);
	data.WriteInt16(14); // server name offset
	data.WriteInt64(3656625); //??? SERVER_ID??
	data.WriteString("PlanetSL_4012");
	data.WritePos(0);

	result = connection_send(c, &data);
	if (!result)
		return false;

	return true;
}

bool WINAPI op_set_visible_range(std::shared_ptr<connection>, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif
	//??
	return true;
}

bool WINAPI op_get_user_list(std::shared_ptr<connection> c, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif
	Stream &stream = c->_recvBuffer.data;
	stream.Clear();
	stream.Resize(35);
	stream.WriteInt16(0);
	stream.WriteInt16(S_GET_USER_LIST);
	uint16 count_pos = stream._pos;
	stream._pos += 2;
	uint16 next = stream._pos;
	stream._pos += 2;

	//stream.WriteByte(0);
	//stream.WriteInt32(0);
	stream._pos += 5;
	stream.WriteInt32((int)SC_PLAYER_MAX_CHARACTER_COUNT);

	stream.WriteInt32(1);	//unk 
	//stream.WriteInt16(0);
	stream._pos += 2;
	stream.WriteInt32(40);//unk


	//stream.WriteInt32(0);
	stream._pos += 4;
	stream.WriteInt32(24);	//unk

	sql::Connection * con = (sql::Connection*)argv[0];

	uint32 worldMapWorldId = 0;
	uint32 worldMapGuardId = 0;
	uint32 worldMapSectionId = 0;
	uint64 banTime, lastOnline;

	uint16 count = 0;
	for (uint32 i = 0; i < SC_PLAYER_MAX_CHARACTER_COUNT; i++)
	{
		std::shared_ptr<player> p = c->_players[i];
		if (!p) continue;

		try
		{
			sql::PreparedStatement* p_s = con->prepareStatement("SELECT * FROM players WHERE name=?");
			p_s->setString(1, p->name);

			sql::ResultSet *r_s = p_s->executeQuery();
			if (r_s && r_s->next())
			{
				lastOnline = r_s->getInt64("lastOnlineUTC");
				banTime = r_s->getInt64("banTimeUTC");

				worldMapGuardId = r_s->getInt("worldMapGuardId");
				worldMapWorldId = r_s->getInt("worldMapWorldId");
				worldMapSectionId = r_s->getInt("worldMapSectionId");
			}
			if (r_s)delete r_s;
			delete p_s;
		}
		catch (sql::SQLException& e)
		{
#ifdef SQL_DEBUG
			printf("::SQL-ERROR: FN[%s] MSG[%s]\n", __FUNCTION__, e.what());
#endif
			continue;
		}

		count++;
		stream.WritePos(next);
		stream.WriteInt16(stream._pos);
		next = stream.NextPos();

		stream.WriteInt32(0);

		uint16 name_pos = stream.NextPos();
		uint16 details1_pos = stream.NextPos();
		stream.WriteInt16(32);
		uint16 details2_pos = stream.NextPos();
		stream.WriteInt16(64);
		uint16 guild_name_pos = stream.NextPos();

		stream.WriteInt32(p->dbid);

		stream.WriteInt32(p->pGender);
		stream.WriteInt32(p->pRace);
		stream.WriteInt32(p->pClass);
		stream.WriteInt32(p->level);

		stream.WriteInt32(6);//unk map id1 ?
		stream.WriteInt32(1231); //unk 1c050000 1308

		stream.WriteInt32(worldMapWorldId);
		stream.WriteInt32(worldMapGuardId);
		stream.WriteInt32(worldMapSectionId);

		stream.WriteInt64(lastOnline); //unk
		stream.WriteUInt8(banTime > 0 ? 1 : 0); //deleteion in progress
		stream.WriteInt64(banTime); //time until character deletion


		stream.WriteInt32(0);


		stream.WriteInt32(p->i_.get_profile_item(PROFILE_WEAPON));
		stream.WriteInt32(p->i_.get_profile_item(PROFILE_EARRING_L));
		stream.WriteInt32(p->i_.get_profile_item(PROFILE_EARRING_R));
		stream.WriteInt32(p->i_.get_profile_item(PROFILE_ARMOR));
		stream.WriteInt32(p->i_.get_profile_item(PROFILE_GLOVES));
		stream.WriteInt32(p->i_.get_profile_item(PROFILE_BOOTS));

		stream.WriteInt32(p->i_.get_profile_item(PROFILE_NECKLACE)); //unk

		stream.WriteInt32(p->i_.get_profile_item(PROFILE_RING_L));
		stream.WriteInt32(p->i_.get_profile_item(PROFILE_RING_R));
		stream.WriteInt32(p->i_.get_profile_item(PROFILE_INNERWARE));
		stream.WriteInt32(p->i_.get_profile_item(PROFILE_MASK)); //head
		stream.WriteInt32(p->i_.get_profile_item(PROFILE_HEAD_ADRONMENT)); //face

		stream.Write(p->details3, SC_PLAYER_DETAILS_3_BUFFER_SIZE);

		stream.WriteUInt8(c->_account.isGm ? 0x01 : 0x00);//isgm

		stream.WriteInt64(0);
		stream.WriteInt32(0); //4
		stream.WriteUInt8(0); //2


		stream.WriteInt32(0xA817EB64); //unk0xA85C54e6
		stream.WriteInt64(0);
		stream.WriteInt64(0);
		stream.WriteInt64(0);
		stream.WriteInt64(0);
		stream.WriteInt64(0);
		stream.WriteInt64(0);
		stream.WriteInt64(0);
		stream.WriteInt64(0);
		stream.WriteInt64(0);
		stream.WriteInt64(0);
		stream.WriteInt64(0);
		stream.WriteInt64(0);

		stream.WriteInt32(p->i_.get_profile_item(PROFILE_SKIN_HEAD));
		stream.WriteInt32(p->i_.get_profile_item(PROFILE_SKIN_FACE));
		stream.WriteInt32(p->i_.get_profile_item(PROFILE_SKIN_BACK));
		stream.WriteInt32(p->i_.get_profile_item(PROFILE_SKIN_WEAPON));
		stream.WriteInt32(p->i_.get_profile_item(PROFILE_SKIN_BODY));
		stream.WriteInt32(0);

		stream.WriteInt32(0);

		stream.WriteInt32(55311);
		stream.WriteInt32(55311);
		stream.WriteUInt8(1);
		stream.WriteInt32(0);
		stream.WriteUInt8(0);

		stream.WriteInt32(25601);
		stream.WriteUInt8(0);

		stream.WriteInt32(30); //achievements

		stream.WriteInt32(0);
		stream.WriteInt32(0);
		stream.WriteInt32(0);

		stream.WritePos(name_pos);
		stream.WriteString(p->name);

		stream.WritePos(details1_pos);
		stream.Write(p->details1, SC_PLAYER_DETAILS_1_BUFFER_SIZE);

		stream.WritePos(details2_pos);
		stream.Write(p->details2, SC_PLAYER_DETAILS_2_BUFFER_SIZE);

		stream.WritePos(guild_name_pos);
		stream.WriteInt16(0);

	}

	stream._pos = count_pos;
	stream.WriteInt16(count);
	stream.SetEnd();
	stream.WritePos(0);

	bool result = connection_send(c);
	if (!result) return false;

	try
	{
		sql::PreparedStatement *s = ((sql::Connection*)argv[0])->prepareStatement("SELECT accountSettings FROM accounts WHERE username=?");
		s->setString(1, c->_account.username);
		sql::ResultSet * rs = s->executeQuery();
		if (rs && rs->next())
		{
			std::istream *blob = rs->getBlob(1);
			if (blob)
			{
				blob->seekg(std::istream::end);
				uint32 size = (uint32)blob->tellg();
				blob->seekg(std::istream::beg);

				stream.Clear();
				stream.Resize(6 + (uint16)size);
				stream.WriteInt16(6 + (uint16)size);
				stream.WriteInt16(S_LOAD_CLIENT_ACCOUNT_SETTING);
				stream.WriteInt16(8);
				blob->read((char*)&stream._raw[stream._pos], size);
				stream.SetEnd();

				connection_send(c);

				delete blob;
			}
			else
			{
				stream.Clear();
				stream.Resize(6);
				stream.WriteInt16(6);
				stream.WriteInt16(S_LOAD_CLIENT_ACCOUNT_SETTING);
				stream.WriteInt16(0);

				connection_send(c);
			}
			delete rs;
		}

		delete s;
	}
	catch (const sql::SQLException& e)
	{
		printf("SQL-ERROR-[%s] FUNC[%s]\n", e.what(), __FUNCTION__);
		stream.Clear();
		stream.Resize(8);
		stream.WriteInt16(8);
		stream.WriteInt16(S_LOAD_CLIENT_ACCOUNT_SETTING);
		stream.WriteInt16(8);
		stream.WriteInt16(0);
		result = connection_send(c);
		if (!result)
			return false;
	}



	stream.Clear();
	stream.Resize(8);
	stream.WriteInt16(8);
	stream.WriteInt16(S_ACCOUNT_PACKAGE_LIST);
	stream.WriteInt32(0);
	result = connection_send(c);
	if (!result)
		return false;

	stream.Clear();
	stream.Resize(17);
	stream.WriteInt16(17);
	stream.WriteInt16(S_CONFIRM_INVITE_CODE_BUTTON);
	stream.WriteInt32(15);
	stream.WriteInt32(0);
	stream.WriteInt32(0);
	stream.WriteUInt8(0);
	//result = connection_send(c);
	//if (!result)
	//	return false;


	stream.Clear();
	stream.Resize(9);
	stream.WriteInt16(9);
	stream.WriteInt16(S_UPDATE_CONTENTS_ON_OFF);
	stream.WriteInt32(2);
	stream.WriteUInt8(0);
	result = connection_send(c);
	if (!result)
		return false;

	stream.Clear();
	stream.Resize(9);
	stream.WriteInt16(9);
	stream.WriteInt16(S_UPDATE_CONTENTS_ON_OFF);
	stream.WriteInt32(3);
	stream.WriteUInt8(0);
	result = connection_send(c);
	if (!result)
		return false;

	stream.Clear();
	stream.Resize(9);
	stream.WriteInt16(9);
	stream.WriteInt16(S_UPDATE_CONTENTS_ON_OFF);
	stream.WriteInt32(4);
	stream.WriteUInt8(0);
	result = connection_send(c);
	if (!result)
		return false;

	stream.Clear();
	stream.Resize(9);
	stream.WriteInt16(9);
	stream.WriteInt16(S_UPDATE_CONTENTS_ON_OFF);
	stream.WriteInt32(8);
	stream.WriteUInt8(0);
	result = connection_send(c);
	if (!result)
		return false;

	stream.Clear();
	stream.Resize(9);
	stream.WriteInt16(9);
	stream.WriteInt16(S_UPDATE_CONTENTS_ON_OFF);
	stream.WriteInt32(9);
	stream.WriteUInt8(0);
	result = connection_send(c);
	if (!result)
		return false;

	return true;
}

bool WINAPI op_hardware_info(std::shared_ptr<connection>, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	return true;
}

bool WINAPI op_request_vip_system_info(std::shared_ptr<connection>c, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	Stream data = Stream();
	data.Resize(47);
	data.WriteInt16(47);
	data.WriteInt16(S_SEND_VIP_SYSTEM_INFO);
	uint16 next = data._pos;
	data._pos += 2;
	data.WriteUInt8(1); //hasTeraRewards
	data.WriteInt32(1); //teraRewardsValue
	//data.WriteInt32(0); //1780
	//data.WriteInt32(0);//6060
	//data.WriteInt32(0); //330
	//data.WriteInt32(0); //0
	data._pos += 16;
	data.WriteUInt8(1); //bool? what?

	data.WriteInt32(timer_get_day_seconds()); // 3752


	//data.WriteInt32(0);
	//data.WriteInt32(0);
	//data.WriteInt32(0);
	data._pos += 12;
	data.WriteUInt8(1);
	data.WritePos(next);
	data.WriteInt16(0);//?
	bool result = connection_send(c, &data);
	if (!result)
		return false;
	return true;
}

bool WINAPI op_get_user_guild_logo(std::shared_ptr<connection>c, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	Stream data = Stream();
	data.Resize(8);
	data.WriteInt16(8);
	data.WriteInt16(S_GET_USER_GUILD_LOGO);
	data.WriteInt32(0);

	return connection_send(c, &data);
}



//--------------------------------------------------------------LOBBY
bool WINAPI op_can_create_player(std::shared_ptr<connection> c, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	if (!c)
		return false;
	int8 index = connection_can_create_player(c);

	Stream data = Stream(5);
	data.WriteInt16(5);
	data.WriteInt16(S_CAN_CREATE_USER);
	data.WriteUInt8(index >= 0 ? 0x01 : 0x00);
	return connection_send(c, &data);
}

bool WINAPI op_create_player(std::shared_ptr<connection> c, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	uint8 index = 0;
	if ((index = connection_can_create_player(c)) >= 0)
	{
		std::shared_ptr<player> p = c->_players[index] = entity_manager::create_player(c, 0);
		p->spawn.init(p);
		Stream * stream = &c->_recvBuffer.data;

		uint16 nameOffset = stream->ReadInt16();
		uint16 details1Offset = stream->ReadInt16();
		stream->ReadInt16();
		uint16 details2Offset = stream->ReadInt16();
		stream->ReadInt16();

		uint32 pGender = (uint32)stream->ReadInt32();
		uint32 pRace = (uint32)stream->ReadInt32();
		uint32 pClass = (uint32)stream->ReadInt32();

		p->model = player_calculate_model((e_player_class)pClass, (e_player_gender)pGender, (e_player_race)pRace);


		stream->Read(p->details3, SC_PLAYER_DETAILS_3_BUFFER_SIZE);

		stream->_pos = nameOffset - 4;
		stream->ReadASCIIStringTo((byte*)p->name, SC_PLAYER_NAME_MAX_LENGTH);

		stream->_pos = details1Offset - 4;


		stream->Read(p->details1, SC_PLAYER_DETAILS_1_BUFFER_SIZE);
		stream->Read(p->details2, SC_PLAYER_DETAILS_2_BUFFER_SIZE);

		p->exp = 1;
		p->restedExp = 1;
		p->pClass = (e_player_class)pClass;
		p->pGender = (e_player_gender)pGender;
		p->pRace = (e_player_race)pRace;
		p->level = pClass == REAPER ? 40 : 1;

		p->position.continent_id = config::zone.zone_start_continent;
		p->position.channel = config::zone.zone_start_channel - 1;
		p->position.x.store(config::zone.zone_start_position[0]);
		p->position.y.store(config::zone.zone_start_position[1]);
		p->position.z.store(config::zone.zone_start_position[2]);
		p->position.heading = config::zone.zone_start_heading;
		p->position.worldMapGuardId = config::zone.zone_start_world_map_guard_id;
		p->position.worldMapSectionId = config::zone.zone_start_world_map_section_id;
		p->position.worldMapWorldId = config::zone.zone_start_world_map_wrold_id;

		inventory_init(p, (uint16)config::player.start_inventory_slot_count);
		inventory_new(p);

		p->i_.gold = config::player.start_gold;

		s_stats_init_player(p);

		p->c_manager.init(p);

		stream->Clear();
		stream->Resize(5);
		stream->WriteInt16(5);
		stream->WriteInt16(S_CREATE_USER);
		stream->WriteUInt8(1);

		if (!connection_send(c, stream))
			return false;

		sql::Connection* con = (sql::Connection*)argv[0];
		if (con)
		{
			BOOL result = 0;
			std::istringstream details1 = std::istringstream(std::string((const char*)p->details1, SC_PLAYER_DETAILS_1_BUFFER_SIZE));
			std::istringstream details2 = std::istringstream(std::string((const char*)p->details2, SC_PLAYER_DETAILS_2_BUFFER_SIZE));
			std::istringstream data = std::istringstream(std::string((const char*)p->details3, SC_PLAYER_DETAILS_3_BUFFER_SIZE));
			sql::PreparedStatement * s = nullptr;
			try
			{
				s = con->prepareStatement("INSERT INTO players(username,name,x,y,z,h,race,gender,class,exp,restedExp,areaId,level,details1,details2,details3,lastOnlineUTC,creationTimeUTC,banTimeUTC,visitedSections,worldMapGuardId,worldMapWorldId,worldMapSectionId,channel) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");

				s->setString(1, c->_account.username);
				s->setString(2, p->name);

				s->setDouble(3, config::zone.zone_start_position[0]);
				s->setDouble(4, config::zone.zone_start_position[1]);
				s->setDouble(5, config::zone.zone_start_position[2]);
				s->setInt(6, config::zone.zone_start_heading);

				s->setInt(7, pRace);
				s->setInt(8, pGender);
				s->setInt(9, pClass);

				s->setInt64(10, p->exp);
				s->setInt64(11, p->restedExp);

				s->setInt(12, config::zone.zone_start_continent);
				s->setInt(13, p->level);

				s->setBlob(14, &details1);
				s->setBlob(15, &details2);
				s->setBlob(16, &data);

				s->setUInt64(17, 0);
				s->setUInt64(18, timer_get_current_UTC());
				s->setUInt64(19, 0);
				s->setNull(20, 0);

				s->setInt(21, config::zone.zone_start_world_map_guard_id);
				s->setInt(22, config::zone.zone_start_world_map_wrold_id);
				s->setInt(23, config::zone.zone_start_world_map_section_id);
				s->setInt(24, config::zone.zone_start_channel - 1);

				result = s->executeUpdate();
			}
			catch (const sql::SQLException& e)
			{
				printf("SQL-ERROR-[%s] FUNC[%s]\n", e.what(), __FUNCTION__);
			}

			delete s;

			Stream * inventoryRaw = p->i_.get_raw();
			std::istringstream invBlob = std::istringstream(std::string((const char*)inventoryRaw->_raw, inventoryRaw->_size));
			s = con->prepareStatement("INSERT INTO player_inventory(username,name,items,slotCount,gold) VALUES(?,?,?,?,?)");
			s->setString(1, c->_account.username);
			s->setString(2, p->name);
			s->setBlob(3, &invBlob);

			s->setInt(4, p->i_.slot_count);
			s->setInt64(5, p->i_.gold);
			try
			{
				result = s->executeUpdate();
			}
			catch (const sql::SQLException& e)
			{
				printf("SQL-ERROR-[%s] FUNC[%s]\n", e.what(), __FUNCTION__);
			}

			delete inventoryRaw;
			delete s;

			s = con->prepareStatement("INSERT INTO player_settings(username,name,settings) VALUES(?,?,?)");
			s->setString(1, c->_account.username);
			s->setString(2, p->name);
			s->setNull(3, 0);
			try
			{
				result = s->executeUpdate();
			}
			catch (const sql::SQLException& e)
			{
				printf("SQL-ERROR-[%s] FUNC[%s]\n", e.what(), __FUNCTION__);
			}

			delete s;

			s = con->prepareStatement("INSERT INTO player_skills(username,name,learnedSkills,skillCount) VALUES(?,?,?,?)");
			s->setString(1, c->_account.username);
			s->setString(2, p->name);
			s->setNull(3, 0);
			s->setInt(4, 0);
			try
			{
				result = s->executeUpdate();
			}
			catch (const sql::SQLException& e)
			{
				printf("SQL-ERROR-[%s] FUNC[%s]\n", e.what(), __FUNCTION__);
			}

			delete s;


			s = con->prepareStatement("INSERT INTO player_bank(username,name,items,slotCount,gold) VALUES(?,?,?,?,?)");
			s->setString(1, c->_account.username);
			s->setString(2, p->name);
			s->setNull(3, 0);
			s->setInt(4, 0);
			s->setInt(5, 0);
			try
			{
				result = s->executeUpdate();
			}
			catch (const sql::SQLException& e)
			{
				printf("SQL-ERROR-[%s] FUNC[%s]\n", e.what(), __FUNCTION__);
			}

			delete s;

			try
			{
				s = con->prepareStatement("SELECT id FROM players WHERE username=? AND name=?");
				s->setString(1, c->_account.username);
				s->setString(2, p->name);

				sql::ResultSet *rs_lid = s->executeQuery();
				if (rs_lid && rs_lid->next())
				{
					p->dbid = rs_lid->getInt(1);
				}
				if (rs_lid)delete rs_lid;
				if (s) delete s;
			}
			catch (const sql::SQLException& e)
			{
				printf("SQL-ERROR-[%s] FUNC[%s]\n", e.what(), __FUNCTION__);
			}

		}

		return true;
	}

	c->_recvBuffer.data.Clear();
	c->_recvBuffer.data.Resize(5);
	c->_recvBuffer.data.WriteInt16(5);
	c->_recvBuffer.data.WriteInt16(S_CREATE_USER);
	c->_recvBuffer.data.WriteUInt8(0);
	return connection_send(c, &c->_recvBuffer.data);
}

bool WINAPI op_change_user_slot_id(std::shared_ptr<connection>, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	return false;
}

bool WINAPI op_check_username(std::shared_ptr<connection>c, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	int16 name_offset = c->_recvBuffer.data.ReadInt16();
	char name[SC_PLAYER_NAME_MAX_LENGTH];
	c->_recvBuffer.data.ReadASCIIStringTo((byte*)name, SC_PLAYER_NAME_MAX_LENGTH);

	int ret = 0;
	sql::Connection* con = (sql::Connection*)argv[0];
	try
	{
		sql::PreparedStatement* p = con->prepareStatement("SELECT * FROM players WHERE name=?");
		p->setString(1, (char*)name);
		sql::ResultSet * rs = p->executeQuery();

		if (rs && rs->next())
			ret = 1;

		if (rs) delete rs;
		if (p)  delete p;
	}
	catch (sql::SQLException& e)
	{
		std::cout << "SQL-ERROR::[" << e.what() << "]\n";
	}


	c->_recvBuffer.data.Clear();
	c->_recvBuffer.data.Resize(5);
	c->_recvBuffer.data.WriteInt16(5);
	c->_recvBuffer.data.WriteInt16(S_CHECK_USERNAME);
	c->_recvBuffer.data.WriteUInt8(ret ? 0 : 1);
	return connection_send(c);
}

bool WINAPI op_str_evaluate_string(std::shared_ptr<connection>c, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif
	c->_recvBuffer.data._pos += 14;
	int32 type = c->_recvBuffer.data.ReadInt32();
	byte name[38];
	bool nameGood = c->_recvBuffer.data.ReadASCIIStringTo(name, 38);

	Stream& stream = c->_recvBuffer.data;
	stream.Clear();
	stream.WriteInt16(0);
	stream.WriteInt16(S_STR_EVALUATE_LIST);
	stream.WriteInt16(1);//unk
	stream.WriteInt16(8);//unk
	stream.WriteInt16(8);//unk
	stream.WriteInt16(1);//unk
	uint16 name_p = stream.NextPos();
	stream.WriteInt32(type);
	stream.WriteInt32(0);
	stream.WritePos(name_p);
	stream.WriteString((char*)name);
	stream.WritePos(0);

	return connection_send(c);
}

bool WINAPI op_select_player(std::shared_ptr<connection> c, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif
	uint32 db_id = c->_recvBuffer.data.ReadInt32();
	bool result = connection_select_player(c, db_id);
	if (!result) return false;

	auto p = c->_players[c->_selected_player];
	Stream * data = &c->_recvBuffer.data;


	data->Clear();
	data->Resize(13);
	data->WriteInt16(13);
	data->WriteInt16(S_SELECT_USER);
	data->WriteUInt8(1);
	if (!connection_send(c))
		return false;


	data->Clear();
	data->Resize(9);
	data->WriteInt16(9);
	data->WriteInt16(S_UPDATE_CONTENTS_ON_OFF);
	data->WriteInt32(2);
	if (!connection_send(c))
		return false;

	data->Clear();
	data->Resize(9);
	data->WriteInt16(9);
	data->WriteInt16(S_UPDATE_CONTENTS_ON_OFF);
	data->WriteInt32(3);
	if (!connection_send(c))
		return false;

	data->Clear();
	data->Resize(9);
	data->WriteInt16(9);
	data->WriteInt16(S_UPDATE_CONTENTS_ON_OFF);
	data->WriteInt32(4);
	if (!connection_send(c))
		return false;

	data->Clear();
	data->Resize(9);
	data->WriteInt16(9);
	data->WriteInt16(S_UPDATE_CONTENTS_ON_OFF);
	data->WriteInt32(8);
	if (!connection_send(c))
		return false;

	data->Clear();
	data->Resize(9);
	data->WriteInt16(9);
	data->WriteInt16(S_UPDATE_CONTENTS_ON_OFF);
	data->WriteInt32(9);
	if (!connection_send(c))
		return false;


	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_BROCAST_GUILD_FLAG);
	if (!connection_send(c))
		return false;


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

	data->WriteSpawnId(p, true);

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
	if (!connection_send(c))
		return false;

#pragma endregion

	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_SHOW_NPC_TO_MAP);
	if (!connection_send(c))
		return false;

	account_load_client_settings(c, (sql::Connection*)argv[0]);
	player_load_user_settings(p, (sql::Connection*)argv[0]);

	p->i_.send();

	int last = 8;
	data->Clear();
	data->Resize(15);
	data->WriteInt16(0);
	data->WriteInt16(S_SKILL_LIST);
	data->WriteInt16(p->skills.active.size() + p->skills.passive.size());
	data->WriteInt16(8);
	for (int i = 0; i < p->skills.active.size(); i++)
	{
		data->WritePos(last);
		data->WriteInt16(data->_pos);
		last = data->_pos;
		data->WriteInt16(0);
		data->WriteInt32(p->skills.active[i]);
		data->WriteUInt8(1);//active
	}
	for (int i = 0; i < p->skills.passive.size(); i++)
	{
		data->WritePos(last);
		data->WriteInt16(data->_pos);
		last = data->_pos;
		data->WriteInt16(0);
		data->WriteInt32(p->skills.passive[i]);
		data->WriteUInt8(0);//passive

	}
	data->WritePos(0);
	if (!connection_send(c))
		return false;

	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_AVAILABLE_SOCIAL_LIST);
	if (!connection_send(c))
		return false;


	data->Clear();
	data->Resize(4);
	data->WriteInt16(4);
	data->WriteInt16(S_CLEAR_QUEST_INFO);
	if (!connection_send(c))
		return false;

	data->Clear();
	data->Resize(9);
	data->WriteInt16(9);
	data->WriteInt16(S_DAILY_QUEST_COMPLETE_COUNT);
	data->WriteInt16(0);
	data->WriteInt16(10);
	if (!connection_send(c))
		return false;

	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_COMPLETED_MISSION_INFO);
	if (!connection_send(c))
		return false;
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

	if (!connection_send(c))
		return false;


	data->Clear();
	data->Resize(10);
	data->WriteInt16(10);
	data->WriteInt16(S_ARTISAN_RECIPE_LIST);
	data->WriteInt32(0);
	data->WriteUInt8(0);
	data->WriteUInt8(1);
	if (!connection_send(c))
		return false;

	//data->Clear();
	//data->Resize(16);
	//data->WriteInt16(16);
	//data->WriteInt16(S_NPCGUILD_LIST);
	//data->WriteInt32(0); // count  and offset 
	//data->WriteWorldId(p);
	//if (!connection_send(c))
	//	return false;

	data->Clear();
	data->Resize(22);
	data->WriteInt16(22);
	data->WriteInt16(S_PET_INCUBATOR_INFO_CHANGE);
	data->WriteInt32(0);// count  and offset 
	data->WriteInt32(1);
	data->WriteInt32(0);
	data->WriteInt32(60);
	data->WriteInt16(0);
	if (!connection_send(c))
		return false;

	data->Clear();
	data->Resize(12);
	data->WriteInt16(12);
	data->WriteInt16(S_VIRTUAL_LATENCY);
	if (!connection_send(c))
		return false;

	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_MOVE_DISTANCE_DELTA);
	data->WriteFloat(200.0f);
	if (!connection_send(c))
		return false;

	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_MY_DESCRIPTION);
	data->WriteInt32(6);
	if (!connection_send(c))
		return false;

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
	if (!connection_send(c, data))
		return false;

	//TOKEN POINTS


	data->Clear();
	data->Resize(28);
	data->WriteInt16(28);
	data->WriteInt16(S_MASSTIGE_STATUS);
	if (!connection_send(c))
		return false;

	data->Clear();
	data->Resize(58);
	data->WriteInt16(58);
	data->WriteInt16(S_AVAILABLE_EVENT_MATCHING_LIST);
	if (!connection_send(c))
		return false;

	data->Clear();
	data->Resize(26);
	data->WriteInt16(26);
	data->WriteInt16(S_CURRENT_ELECTION_STATE);
	if (!connection_send(c))
		return false;

	if (!player_send_external_change(p))
		return false;

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
	if (!connection_send(c, data))
		return false;
#pragma endregion

	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_FESTIVAL_LIST);
	if (!connection_send(c))
		return false;

	data->Clear();
	data->Resize(21);
	data->WriteInt16(21);
	data->WriteInt16(S_LOAD_TOPO);
	data->WriteInt32(p->position.continent_id);
	data->WriteFloat(p->position.x.load());
	data->WriteFloat(p->position.y.load());
	data->WriteFloat(p->position.z.load());
	//data->WriteByte(0);
	if (!connection_send(c))
		return false;

	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_LOAD_HINT);
	if (!connection_send(c))
		return false;


	data->Clear();
	data->Resize(9);
	data->WriteInt16(9);
	data->WriteInt16(S_ACCOUNT_BENEFIT_LIST);
	if (!connection_send(c))
		return false;

	data->Clear();
	data->Resize(16);
	data->WriteInt16(16);
	data->WriteInt16(S_SEND_USER_PLAY_TIME);
	data->WriteInt32(0); //online time [s]
	data->WriteInt64(1484999153); //creation time
	if (!connection_send(c))
		return false;


	data->Clear();
	data->Resize(12);
	data->WriteInt16(12);
	data->WriteInt16(S_PCBANGINVENTORY_DATALIST);
	if (!connection_send(c))
		return false;

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
	if (!connection_send(c))
		return false;


	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_COMPLETED_MISSION_INFO);
	if (!connection_send(c))
		return false;


	data->Clear();
	data->Resize(12);
	data->WriteInt16(12);
	data->WriteInt16(S_FATIGABILITY_POINT);
	data->WriteInt32(1);
	data->WriteInt32(20);
	if (!connection_send(c))
		return false;

	data->Clear();
	data->Resize(44);
	data->WriteInt16(44);
	data->WriteInt16(S_LOAD_EP_INFO);
	if (!connection_send(c))
		return false;

	return true;
}

bool WINAPI op_delete_player(std::shared_ptr<connection>c, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	uint32 db_id = c->_recvBuffer.data.ReadInt32();
	bool result = true;

	std::shared_ptr<player> p_p = nullptr;
	for (uint8 i = 0; i < SC_PLAYER_NAME_MAX_LENGTH; i++)
		if (c->_players[i] && c->_players[i]->dbid == db_id)
		{
			p_p = c->_players[i];
			c->_players[i] = nullptr;
			break;
		}

	if (!p_p) return false;

	sql::Connection* con = (sql::Connection*)argv[0];
	if (con)
	{
		sql::PreparedStatement * p = con->prepareStatement("DELETE  FROM players WHERE id=?");
		p->setInt(1, db_id);
		try
		{
			p->executeUpdate();
		}
		catch (const sql::SQLException& e)
		{
			std::cout << ">>Error:DELETE PLAYER[" << e.what() << "]\n";
		}
		delete p;

		p = con->prepareStatement("DELETE FROM player_inventory WHERE name=?");
		p->setString(1, p_p->name);
		try
		{
			p->executeUpdate();
		}
		catch (const sql::SQLException& e)
		{
			std::cout << ">>Error:DELETE PLAYER[" << e.what() << "]\n";
		}
		delete p;

		p = con->prepareStatement("DELETE FROM player_bank WHERE name=?");
		p->setString(1, p_p->name);
		try
		{
			p->executeUpdate();
		}
		catch (const sql::SQLException& e)
		{
			std::cout << ">>Error:DELETE PLAYER[" << e.what() << "]\n";
		}
		delete p;

		p = con->prepareStatement("DELETE FROM player_skills WHERE name=?");
		p->setString(1, p_p->name);
		try
		{
			p->executeUpdate();
		}
		catch (const sql::SQLException& e)
		{
			std::cout << ">>Error:DELETE PLAYER[" << e.what() << "]\n";
		}
		delete p;

		p = con->prepareStatement("DELETE FROM player_settings WHERE name=?");
		p->setString(1, p_p->name);
		try
		{
			p->executeUpdate();
		}
		catch (const sql::SQLException& e)
		{
			std::cout << ">>Error:DELETE PLAYER[" << e.what() << "]\n";
		}
		delete p;


	}

	entity_manager::destroy_player(p_p->eid);

	Stream data = Stream();
	data.Resize(5);
	data.WriteInt16(5);
	data.WriteInt16(S_DELETE_USER);
	data.WriteUInt8(result == true ? 1 : 0);

	return connection_send(c, &data);
}

bool WINAPI op_cancel_delete_player(std::shared_ptr<connection>, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	return false;
}

bool WINAPI op_save_client_account_settings(std::shared_ptr<connection>c, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif
	c->_recvBuffer.data._pos += 4;
	sql::Connection* con = (sql::Connection*)argv[0];
	sql::PreparedStatement *s = con->prepareStatement("UPDATE accounts SET accountSettings=? WHERE username=?");
	std::stringstream blob = std::stringstream(std::string((char*)c->_recvBuffer.data._raw, c->_recvBuffer.data._size));
	s->setBlob(1, (std::istream*)&blob);
	s->setString(2, c->_account.username);
	try
	{
		s->executeUpdate();
	}
	catch (sql::SQLException& e)
	{
		std::cout << "SQL-ERROR::[" << e.what() << "]\n";
		return false;
	}

	delete s;
	c->_recvBuffer.data.Clear();
	return true;
}

bool WINAPI op_save_client_user_settings(std::shared_ptr<connection>c, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif
	c->_recvBuffer.data._pos += 4;
	sql::Connection* con = (sql::Connection*)argv[0];
	sql::PreparedStatement *s = con->prepareStatement("UPDATE player_settings SET settings=? WHERE name=? AND username=?");
	std::stringstream blob = std::stringstream(std::string((char*)c->_recvBuffer.data._raw, c->_recvBuffer.data._size));
	s->setBlob(1, (std::istream*)&blob);
	s->setString(2, c->_players[c->_selected_player]->name);
	s->setString(3, c->_account.username);

	try
	{
		s->executeUpdate();
	}
	catch (sql::SQLException& e)
	{
		std::cout << "SQL-ERROR::[" << e.what() << "]\n";
	}
	delete s;
	return true;
}



//--------------------------------------------------------------WORLD
bool WINAPI op_load_topo_fin(std::shared_ptr<connection>c, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif
	sql::Connection* con = (sql::Connection*)argv[0];
	uint32	data[2];
	try
	{
		sql::PreparedStatement * p_s = con->prepareStatement("SELECT * FROM players WHERE name=?");
		p_s->setString(1, c->_players[c->_selected_player]->name);

		sql::ResultSet * r_s = p_s->executeQuery();
		if (r_s && r_s->next())
		{
			data[0] = r_s->getInt("areaId"); //13
			data[1] = r_s->getInt("channel");
		}
		if (r_s) delete r_s;
		delete p_s;
	}
	catch (sql::SQLException& e)
	{
		printf("SQL-ERROR-[%s] FUNC[%s]\n", e.what(), __FUNCTION__);
		return false;
	}

	active_add_player(c->_players[c->_selected_player]);

	c->_inLobby = false;
	return world_server_process_job_async(new j_enter_world(c->_players[c->_selected_player], data), J_W_PLAYER_ENTER_WORLD);
}

bool WINAPI op_return_to_lobby(std::shared_ptr<connection> c, void* argv[])
{
	//todo timed
	world_server_process_job_async(new j_exit_world(c->_players[c->_selected_player]), J_W_PLAYER_EXIT_WORLD);
	active_remove_player(c->_players[c->_selected_player]);
	c->_players[c->_selected_player]->save((sql::Connection*)argv[0]);

	Sleep(500);
	Stream data = Stream();
	data.Resize(4);
	data.WriteInt16(4);
	data.WriteInt16(S_RETURN_TO_LOBBY);
	c->_players[c->_selected_player]->con->_inLobby = true;
	return connection_send(c, &data);

}

bool WINAPI op_cancel_return_to_lobby(std::shared_ptr<connection>, void* argv[])
{
	return false;
}

bool WINAPI op_exit(std::shared_ptr<connection> c, void* argv[])
{
	world_server_process_job_async(new j_exit_world(c->_players[c->_selected_player]), J_W_PLAYER_EXIT_WORLD);
	active_remove_player(c->_players[c->_selected_player]);
	c->_players[c->_selected_player]->save((sql::Connection*)argv[0]);

	Stream data = Stream();
	data.Resize(12);
	data.WriteInt16(12);
	data.WriteInt16(S_EXIT);
	data.WriteInt16(0);
	data.WriteInt32(1);
	return connection_send(c, &data);
}

bool WINAPI op_guard_pk_policy(std::shared_ptr<connection>c, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	c->_recvBuffer.data.Clear();
	c->_recvBuffer.data.Resize(7);
	c->_recvBuffer.data.WriteInt16(7);
	c->_recvBuffer.data.WriteInt16(S_GUARD_PK_POLICY);
	c->_recvBuffer.data.WriteInt16(1);
	//uint8 0

	return connection_send(c, &c->_recvBuffer.data);
}

bool WINAPI op_simple_tip_repeate_checl(std::shared_ptr<connection>c, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	uint32 tip = c->_recvBuffer.data.ReadInt32();
	c->_recvBuffer.data.Clear();
	c->_recvBuffer.data.Resize(11);
	c->_recvBuffer.data.WriteInt16(11);
	c->_recvBuffer.data.WriteInt16(S_SIMPLE_TIP_REPEAT_CHECK);
	c->_recvBuffer.data.WriteInt32(tip);
	c->_recvBuffer.data.WriteInt16(1);
	//c->_recvBuffer.data.WriteByte(0);

	return connection_send(c, &c->_recvBuffer.data);
}

bool WINAPI op_tradebroker_heighes_item_level(std::shared_ptr<connection>c, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	c->_recvBuffer.data.Clear();
	c->_recvBuffer.data.Resize(10);
	c->_recvBuffer.data.WriteInt16(10);
	c->_recvBuffer.data.WriteInt16(S_TRADE_BROKER_HIGHEST_ITEM_LEVEL);
	//uint32 0
	//uint16 0
	return connection_send(c, &c->_recvBuffer.data);
}

bool WINAPI op_server_time(std::shared_ptr<connection>c, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	c->_recvBuffer.data.Clear();
	c->_recvBuffer.data.Resize(12);
	c->_recvBuffer.data.WriteInt16(12);
	c->_recvBuffer.data.WriteInt16(S_SERVER_TIME);
	c->_recvBuffer.data.WriteInt64(timer_get_current_UTC());

	return connection_send(c, &c->_recvBuffer.data);
}

bool WINAPI op_update_contents_playtime(std::shared_ptr<connection>, void* argv[] /*todo*/)
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	return true; //todo
}

bool WINAPI op_request_ingame_product_list(std::shared_ptr<connection>, void* argv[] /*todo*/)
{

#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	/*
	data->WriteInt16(7);
	data->WriteInt16(S_INGAMESHOP_CATEGORY_BEGIN);
	data->WriteInt16(1);
	data->WriteByte(0);
	caller->Send(data);
	data->Clear();

	data->WriteInt16(7);
	data->WriteInt16(S_INGAMESHOP_CATEGORY_END);
	data->WriteInt16(1);
	data->WriteByte(0);
	caller->Send(data);
	data->Clear();

	data->WriteInt16(7);
	data->WriteInt16(S_INGAMESHOP_PRODUCT_BEGIN);
	data->WriteInt16(1);
	data->WriteByte(0);
	caller->Send(data);
	data->Clear();

	data->WriteInt16(7);
	data->WriteInt16(S_INGAMESHOP_PRODUCT_END);
	data->WriteInt16(1);
	data->WriteByte(0);
	caller->Send(data);
	data->Clear();
	*/

	return true;
}

bool WINAPI op_player_location(std::shared_ptr<connection>c, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif
	std::shared_ptr<player> p = c->_players[c->_selected_player];

	float p_init[3];
	p_init[0] = a_load(p->position.x);
	p_init[1] = a_load(p->position.y);
	p_init[2] = a_load(p->position.z);

	p->position.x.store(c->_recvBuffer.data.ReadFloat());
	p->position.y.store(c->_recvBuffer.data.ReadFloat());
	p->position.z.store(c->_recvBuffer.data.ReadFloat());
	p->position.heading.store(c->_recvBuffer.data.ReadInt16());
	c->_recvBuffer.data._pos += 2;
	p->position.t_x.store(c->_recvBuffer.data.ReadFloat());
	p->position.t_y.store(c->_recvBuffer.data.ReadFloat());
	p->position.t_z.store(c->_recvBuffer.data.ReadFloat());
	e_player_move_type t = (e_player_move_type)c->_recvBuffer.data.ReadInt32();
	uint16 speed = c->_recvBuffer.data.ReadUInt16();
	uint16 time = c->_recvBuffer.data.ReadInt16();

	//printf("MOVE [%d] [%lu]\n", speed, time);

	Stream data = Stream();
	data.Resize(47);
	data.WriteInt16(47);
	data.WriteInt16(S_USER_LOCATION);
	data.WriteWorldId(p);
	data.WriteFloat(a_load(p->position.x));
	data.WriteFloat(a_load(p->position.y));
	data.WriteFloat(a_load(p->position.z));
	data.WriteInt32(p->position.heading.load());
	data.WriteInt16(t == P_JUMP_START ? speed : p->stats.get_movement_speed(p->status)); //todo 
	data.WriteFloat(a_load(p->position.t_x));
	data.WriteFloat(a_load(p->position.t_y));
	data.WriteFloat(a_load(p->position.t_z));
	data.WriteInt32(t);
	data.WriteUInt8(1);

	if (!world_server_process_job_async(new j_move(p, p_init, t), J_W_PLAYER_MOVE))
		return false;

	p->spawn.bordacast(&data);
	return true;
}

bool WINAPI op_event_guide(std::shared_ptr<connection>, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	return true;
}

bool WINAPI op_set_item_string(std::shared_ptr<connection>, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	return true;
}

bool WINAPI op_reign_info(std::shared_ptr<connection>c, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	int unk1 = c->_recvBuffer.data.ReadInt32();

	c->_recvBuffer.data.Clear();
	c->_recvBuffer.data.Resize(18);
	c->_recvBuffer.data.WriteInt16(18);
	c->_recvBuffer.data.WriteInt16(S_REIGN_INFO);
	c->_recvBuffer.data.WriteInt16(12);//unk
	c->_recvBuffer.data.WriteInt16(14);//unk

	c->_recvBuffer.data.WriteInt32(unk1);
	//c->_recvBuffer.data.WriteInt32(0);
	//c->_recvBuffer.data.WriteInt16(0);

	return connection_send(c, &c->_recvBuffer.data);
}

bool WINAPI op_request_gamestat_ping(std::shared_ptr<connection>c, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	c->_recvBuffer.data.Clear();
	c->_recvBuffer.data.Resize(4);
	c->_recvBuffer.data.WriteInt16(4);
	c->_recvBuffer.data.WriteInt16(S_RESPONSE_GAMESTAT_PONG);
	return connection_send(c);
}

bool WINAPI op_social(std::shared_ptr<connection>, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	return false;
}

bool WINAPI op_visit_new_section(std::shared_ptr<connection>c, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	uint32 worldMapWorldId = (uint32)c->_recvBuffer.data.ReadInt16();
	c->_recvBuffer.data.ReadInt16();

	uint32 worldMapGuardId = (uint32)c->_recvBuffer.data.ReadInt16();
	c->_recvBuffer.data.ReadInt16();

	uint32 worldMapSectionId = c->_recvBuffer.data.ReadInt32();


	//return world_server_process_job_async(
	//	J_W_PLAYER_VISIT_NEW_SECTION,
	//	4,
	//	new void*[4]{ new std::shared_ptr<player>(c->_players[c->_selected_player]), new uint32{worldMapWorldId}, new uint32{worldMapGuardId}, new uint32{worldMapSectionId} }
	//);
	return true;
}


//--------------------------------------------------------------MISC
bool WINAPI op_show_inven(std::shared_ptr<connection>c, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	c->_players[c->_selected_player]->i_.send(1);
	return true;
}

bool WINAPI op_equipe_item(std::shared_ptr<connection> c, void * argv[])
{
	std::shared_ptr<player> p = c->_players[c->_selected_player];
	c->_recvBuffer.data._pos += 8;
	slot_id s_id = c->_recvBuffer.data.ReadInt32();

	p->i_.equipe_item(s_id);
	return true;
}

bool WINAPI op_unequipe_item(std::shared_ptr<connection> c, void * argv[])
{
	std::shared_ptr<player> p = c->_players[c->_selected_player];

	uint32 pid = c->_recvBuffer.data.ReadInt32();
	uint32 psid = c->_recvBuffer.data.ReadInt32();

	uint32 profile_slot_id = c->_recvBuffer.data.ReadInt32();
	uint32 inventory_slot_id = c->_recvBuffer.data.ReadInt32();
	item_eid i_eid = c->_recvBuffer.data.ReadInt32();

	p->i_.unequipe_item(profile_slot_id);

	return true;
}

bool WINAPI op_dungeon_clear_count_list(std::shared_ptr<connection> c, void * argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	c->_recvBuffer.data.Clear();
	c->_recvBuffer.data.Resize(8);
	c->_recvBuffer.data.WriteInt16(8);
	c->_recvBuffer.data.WriteInt16(S_DUNGEON_CLEAR_COUNT_LIST);
	//0
	return connection_send(c);
}

bool WINAPI op_dungeon_cooltime_list(std::shared_ptr<connection> c, void * argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	c->_recvBuffer.data.Clear();
	c->_recvBuffer.data.Resize(8);
	c->_recvBuffer.data.WriteInt16(8);
	c->_recvBuffer.data.WriteInt16(S_DUNGEON_COOL_TIME_LIST);
	//0
	return connection_send(c);
}

bool WINAPI op_request_user_itemlevel_info(std::shared_ptr<connection> c, void * argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	c->_recvBuffer.data.Clear();
	c->_recvBuffer.data.Resize(12);
	c->_recvBuffer.data.WriteInt16(12);
	c->_recvBuffer.data.WriteInt16(S_USER_ITEMLEVEL_INFO);

	return connection_send(c);
}

bool WINAPI op_npc_guild_list(std::shared_ptr<connection> c, void* argv[])
{
#ifdef OP_DUMP
	printf("RECV OP[%s]\n", __FUNCTION__);
#endif

	Stream * data = &c->_recvBuffer.data;
	int16 name_offset = data->ReadInt16();
	data->_pos = name_offset - 4;
	char player_name[SC_PLAYER_NAME_MAX_LENGTH];
	data->ReadASCIIStringTo((byte*)player_name, SC_PLAYER_NAME_MAX_LENGTH);


	data->Clear();
	data->Resize(16);
	data->WriteInt16(16);
	data->WriteInt16(S_NPCGUILD_LIST);

	data->WriteInt16(0); //count
	data->WriteInt16(0);

	data->WriteWorldId(c->_players[c->_selected_player]);
	//
	//data->WriteInt16(16); //count
	//data->WriteInt16(0);
	//
	//data->WriteInt32(9); //region
	//data->WriteInt32(610); //faction
	//data->WriteInt32(6); //rank  # enum { suspicious = 0, apprehensive = 3, wavering, neutral, favorable, friendly, trusted, revered }
	//data->WriteInt32(0); //reputation
	//data->WriteInt32(0); //credits


	return connection_send(c);
}

bool WINAPI op_view_battlefield_result(std::shared_ptr<connection>c, void * argv[]/*TODO*/)
{

	return true;
}

bool WINAPI op_chat(std::shared_ptr<connection> c, void * argv[])
{
	std::shared_ptr<player> p = c->_players[c->_selected_player];

	int16 text_offset = c->_recvBuffer.data.ReadInt16();
	e_chat_type type = (e_chat_type)c->_recvBuffer.data.ReadInt32();

	c->_recvBuffer.data._pos = text_offset - 4;
	std::string text = c->_recvBuffer.data.ReadUTF16StringBigEdianToASCII();

	if (config::chat.run)
		chat_process_message_async(new chat_message(type, p, text, c->_account.isGm ? 0x01 : 0x00));
	else
	{
		//TODO : send 'server_chat is down'
	}
	return true;
}

bool WINAPI op_del_item(std::shared_ptr<connection> c, void * argv[])
{
	c->_recvBuffer.data._pos = 8;
	uint32 slot = c->_recvBuffer.data.ReadUInt32();
	uint32 stack = c->_recvBuffer.data.ReadUInt32();
	c->_players[c->_selected_player]->i_.lock();
	c->_players[c->_selected_player]->i_.inventory_slots[slot]._item->stackCount -= stack;
	if (c->_players[c->_selected_player]->i_.inventory_slots[slot]._item->stackCount <= 0) {
		slot_wipe(c->_players[c->_selected_player]->i_.inventory_slots[slot]);
	}
	c->_players[c->_selected_player]->i_.unlock();
	c->_players[c->_selected_player]->i_.send();
	return true;
}

bool WINAPI op_show_item_tooltip_ex(std::shared_ptr<connection> c, void * argv[])
{
	uint16 name_offset = c->_recvBuffer.data.ReadUInt16();
	uint32 type = c->_recvBuffer.data.ReadUInt32();
	item_eid eid = c->_recvBuffer.data.ReadUInt64();

	//uint32 unk1 = data->ReadUInt32();
	//uint32 unk2 = data->ReadUInt32();
	//uint32 unk3 = data->ReadUInt32();
	//uint32 unk4 = data->ReadUInt32();
	//uint32 unk5 = data->ReadUInt32();
	//
	//data->_pos = name_offset - 4;
	//std::string player_name = std::move(data->ReadUTF16StringBigEdianToASCII());

	send_item_tooltip(c->_players[c->_selected_player], eid, type);
	return true;
}

bool WINAPI op_move_inven_pos(std::shared_ptr<connection> c, void * argv[])
{
	std::shared_ptr<player> p = c->_players[c->_selected_player];

	entityId p_eid = c->_recvBuffer.data.ReadUInt64();

	uint32 s_0 = c->_recvBuffer.data.ReadUInt32();
	uint32 s_1 = c->_recvBuffer.data.ReadUInt32();

	if (p->i_.move_item(s_0, s_1))
		p->i_.send();

	return true;
}


//--------------------------------------------------------------CONTRACT
bool WINAPI op_request_contract(std::shared_ptr<connection> c, void * argv[])
{
	c->_recvBuffer.data._pos = 6;
	e_contract_type c_t = (e_contract_type)c->_recvBuffer.data.ReadUInt32();
	c->_recvBuffer.data.SetFront();

	c->_players[c->_selected_player]->c_manager.request_contract(c_t, c->_recvBuffer.data);

	return true;
}

bool WINAPI op_cancel_contract(std::shared_ptr<connection> c, void * argv[])
{
	e_contract_type c_t = (e_contract_type)c->_recvBuffer.data.ReadUInt32();
	contract * b_c = c->_players[c->_selected_player]->c_manager.get_contract(c_t);
	if (b_c) {
		b_c->cancel();
	}
	return true;
}

bool WINAPI op_bind_item_begin_progress(std::shared_ptr<connection> c, void * argv[])
{
	uint32 unk = c->_recvBuffer.data.ReadUInt32();

	bind_contract * t = (bind_contract*)c->_players[c->_selected_player]->c_manager.get_contract(BIND_CONTRACT);
	if (t) {
		t->begin(unk);
	}

	return true;
}

bool WINAPI op_bind_item_execute(std::shared_ptr<connection> c, void * argv[])
{
	uint32 unk = c->_recvBuffer.data.ReadUInt32();

	bind_contract * t = (bind_contract*)c->_players[c->_selected_player]->c_manager.get_contract(BIND_CONTRACT);
	if (t) {
		t->execute(unk);
	}

	return true;
}

bool WINAPI op_execute_temper(std::shared_ptr<connection> c, void * argv[])
{
	enchant_contract * contract = (enchant_contract*)c->_players[c->_selected_player]->c_manager.get_contract(ENCHANT_CONTRACT);
	if (contract) {
		contract->try_enchant();
	}
	else
		if (c->_account.isGm) {
			chat_send_simple_system_message("ERROR 10005 [ITEM ENCHANT FAILED]", c->_players[c->_selected_player]);
		}

	return true;
}

bool WINAPI op_cancel_temper(std::shared_ptr<connection> c, void * argv[])
{
	enchant_contract * contract = (enchant_contract*)c->_players[c->_selected_player]->c_manager.get_contract(ENCHANT_CONTRACT);
	if (contract) {
		contract->cancel_temper();
	}
	else
		if (c->_account.isGm) {
			chat_send_simple_system_message("ERROR 10006 [op_cancel_temper]", c->_players[c->_selected_player]);
		}

	return true;
}

bool WINAPI op_play_execute_temper(std::shared_ptr<connection> c, void * argv[])
{
	send_social(12, c->_players[c->_selected_player]);
	return true;
}

bool WINAPI op_add_to_temper_material_ex(std::shared_ptr<connection> c, void * argv[])
{
	uint32 slot = c->_recvBuffer.data.ReadUInt32();
	uint32 i_eid = c->_recvBuffer.data.ReadUInt32();
	uint32 unk = c->_recvBuffer.data.ReadUInt32();
	uint32 i_id = c->_recvBuffer.data.ReadUInt32();
	uint32 count = c->_recvBuffer.data.ReadUInt32();

	enchant_contract * contract = (enchant_contract*)c->_players[c->_selected_player]->c_manager.get_contract(ENCHANT_CONTRACT);
	if (contract) {
		if (slot == 0) {
			if (!contract->add_item_to_enchant(unk, i_eid, 0, 0)) {
				if (c->_account.isGm) {
					chat_send_simple_system_message("ERROR 10002 [ADD ITEM TO ENCHANT FAILED]", c->_players[c->_selected_player]);
				}
			}
		}
		else if (slot == 1 || slot == 2) {
			if (!contract->add_material_to_enchant_process(i_eid, count, slot)) {
				if (c->_account.isGm) {
					chat_send_simple_system_message("ERROR 10003 [ADD MATERIAL ITEM " + std::to_string(slot) + " TO ENCHANT FAILED]", c->_players[c->_selected_player]);
				}
			}
		}
		else {
			if (c->_account.isGm) {
				chat_send_simple_system_message("ERROR 10004 [UNK SLOT ID:" + std::to_string(slot) + "]", c->_players[c->_selected_player]);
			}
		}
	}
	return true;
}

bool WINAPI op_check_unidentify_items(std::shared_ptr<connection> c, void * argv[])
{
	Stream &data = c->_recvBuffer.data;
	uint32 scrollEId = data.ReadUInt32();
	uint32 unk2 = data.ReadUInt32();
	uint32 scrollItemId = data.ReadUInt32(); //Master Enigmatic Scroll|Noble Enigmatic Scroll...
	uint32 identifyEId = data.ReadUInt32();
	uint32 unk5 = data.ReadUInt32();
	uint32 identifyScrollItemId = data.ReadUInt32(); //Intricate Identification Scroll ...
	uint32 itemEId = data.ReadUInt32();
	uint32 unk8 = data.ReadUInt32();
	uint32 itemId = data.ReadUInt32();

	enigmatic_contract* con = (enigmatic_contract*)c->_players[c->_selected_player]->c_manager.get_contract(ENIGMATIC_CONTRACT);
	if (con) {
		if (!con->add_item(itemEId, scrollItemId, identifyScrollItemId))
			con->cancel();
	}

	return true;
}

bool WINAPI op_random_passive_lock(std::shared_ptr<connection> c, void * argv[])
{
	std::vector<uint32> passivities;
	uint16 count = c->_recvBuffer.data.ReadUInt16();
	uint16 next = c->_recvBuffer.data.ReadUInt16();

	c->_recvBuffer.data._pos = next - 4;
	for (uint16 i = 0; i < count; i++){
		c->_recvBuffer.data._pos += 4;
		passivities.push_back(c->_recvBuffer.data.ReadUInt32());
	}

	enigmatic_contract* con = (enigmatic_contract*)c->_players[c->_selected_player]->c_manager.get_contract(ENIGMATIC_CONTRACT);
	if (con) {
		con->lock_passivity(passivities);
	}

	return true;
}

bool WINAPI op_unidentify_execute(std::shared_ptr<connection> c, void * argv[])
{
	enigmatic_contract* con = (enigmatic_contract*)c->_players[c->_selected_player]->c_manager.get_contract(ENIGMATIC_CONTRACT);
	if (con) {
		con->execute();
	}
	return true;
}


//--------------------------------------------------------------Guild
bool WINAPI op_request_guild_info(std::shared_ptr<connection> c, void* argv[])
{
	Stream data = Stream();
	data.Resize(4);
	data.WriteInt16(4);
	data.WriteInt16(S_EMPTY_GUILD_WINDOW);

	return connection_send(c, &data);
}