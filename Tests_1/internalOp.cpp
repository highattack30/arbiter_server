#include "internalOp.h"
#include "connexion.h"
#include "Stream.h"
#include "itemEnums.h"
#include "connexionEnums.h"
#include "serverEnums.h"
#include "job.h"
#include "world_server.h"
#include "arbiter_server.h"
#include "active_server.h"

bool WINAPI io_load_topo_fin(j_enter_world_fin * j)
{
	std::shared_ptr<player> p = j->p_;
	std::shared_ptr<connection> c = p->con;

	if (!j->result)
	{
		arbiter_server_connexion_remove(c->_id);
		return false;
	}

	Stream *data = new Stream();



	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_ONGOING_LEVEL_EVENT_LIST);
	if (!connection_send(c, data))
		return false;


	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_ONGOING_HUNTING_EVENT_LIST);
	if (!connection_send(c, data))
		return false;

	//data->Clear();
	//data->Resize(8);
	//data->WriteInt16(8);
	//data->WriteInt16(S_ADD_AWAKEN_ENCHANT_DATA);
	//if (!connection_send(c, data))
	//	return false;
	//
	//data->Clear();
	//data->Resize(8);
	//data->WriteInt16(8);
	//data->WriteInt16(S_ADD_AWAKEN_CHANGE_DATA);
	//if (!connection_send(c, data))
	//	return false;

	//data->Clear();
	//data->Resize(8);
	//data->WriteInt16(8);
	//data->WriteInt16(S_GUILD_QUEST_LIST);
	//if (!connection_send(c, data))
	//	return false;


	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_VISITED_SECTION_LIST);
	if (!connection_send(c, data))
		return false;

	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_REQUEST_INVITE_GUILD_TAG);
	if (!connection_send(c, data))
		return false;

	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_USER_BLOCK_LIST);
	if (!connection_send(c, data))
		return false;

	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_FRIEND_GROUP_LIST);
	if (!connection_send(c, data))
		return false;

	data->Clear();
	data->WriteInt16(0);
	data->WriteInt16(S_FRIEND_LIST);
	data->WriteInt32(0);
	short name_pos = data->NextPos();
	data->WritePos(name_pos);
	data->WriteString("[SKY-LAKE Server]");
	data->WritePos(0);
	if (!connection_send(c, data))
		return false;

	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_UPDATE_FRIEND_INFO);
	data->WriteInt32(0);
	if (!connection_send(c, data))
		return false;

	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_EP_SYSTEM_DAILY_EVENT_EXP_ON_OFF);
	if (!connection_send(c, data))
		return false;


	data->Clear();
	data->Resize(28);
	data->WriteInt16(28);
	data->WriteInt16(S_SPAWN_ME);
	data->WriteWorldId(p);
	data->WriteFloat(p->position.x.load());
	data->WriteFloat(p->position.y.load());
	data->WriteFloat(p->position.z.load());
	data->WriteInt16(p->position.heading.load());
	data->WriteInt16(1); //alive
	if (!connection_send(c, data))
		return false;


	

	data->Clear();
	data->Resize(8);
	data->WriteInt16(8);
	data->WriteInt16(S_ATTENDANCE_EVENT_REWARD_COUNT);
	if (!connection_send(c, data))
		return false;


	data->Clear();
	data->Resize(9);
	data->WriteInt16(9);
	data->WriteInt16(S_ACCOUNT_BENEFIT_LIST);
	if (!connection_send(c, data))
		return false;


#pragma region stats
	player_recalculate_stats(p);
#pragma endregion

	data->Clear();
	data->Resize(20);
	data->WriteInt16(20);
	data->WriteInt16(S_PLAYER_CHANGE_ALL_PROF);
	if (!connection_send(c, data))
		return false;


	data->Clear();
	data->Resize(12);
	data->WriteInt16(12);
	data->WriteInt16(S_PARCEL_READ_RECV_STATUS);
	if (!connection_send(c, data))
		return false;


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
	if (!connection_send(c, data))
		return false;

	data->Clear();
	data->Resize(17);
	data->WriteInt16(17);
	data->WriteInt16(S_USER_STATUS);
	data->WriteWorldId(p);
	//data->WriteInt32(0);
	//data->WriteByte(0);
	if (!connection_send(c, data))
		return false;

	//data->Clear();
	//data->Resize(16);
	//data->WriteInt16(16);
	//data->WriteInt16(S_CREST_INFO);
	//if (!connection_send(c, data))
	//	return false;

	return true;
}
