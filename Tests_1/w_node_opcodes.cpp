#include "w_node_opcodes.h"
#include "world_server_handler.h"
#include "player.h"
#include "arbiter_server.h"
#include "connexion.h"
#include "internalOp.h"
#include "chat.h"

void WINAPI w_op_fn_init() {
	op_fns[C_W_INIT] = op_init;
	op_fns[C_W_CLOSE] = op_close;
	op_fns[C_W_PLAYER_ENTER_WORLD] = op_player_enter_world;
	op_fns[C_W_PLAYER_LEAVE_WORLD] = op_player_leave_world;
	op_fns[C_W_PLAYER_MOVE_BROADCAST] = op_player_move_broadcast;
	op_fns[C_W_GET_VISIBLE_PLAYERS] = op_get_visible_players;
	return;
}

w_opcode_fn get_w_op_fn(e_world_node_opcode t) {

	if (t < WOP_MAX)
		return op_fns[t];

	return nullptr;
}



void WINAPI op_init(w_s_working_thread * t) {
	Stream &data = t->current_buffer->data;

	uint32 area_id = data.ReadUInt32();
	uint32 continent_id = data.ReadUInt32();
	uint32 channel_id = data.ReadUInt32();

	std::string name = data.ReadUTF16StringBigEdianToASCII();

	uint8 result = w_server_process_node_init_data(area_id, channel_id, continent_id);
	if (!result) {
		uint32 node_id = t->current_buffer->parent;
		w_server_node * node = w_server_get_node(node_id);
		node->area_id = area_id;
		node->channel_id = channel_id;
		node->continent_id = continent_id;
		node->name = name;
		printf("RECV INIT_DATA FROM WORLD NODE[%d] AREA[%d] CHANNEL[%d] CONTIENTN[%d] NANE[%s]\n", node_id, area_id, channel_id, continent_id, name.c_str());
	}

	Stream d;
	d.Resize(9);
	d.WriteUInt16(9);
	d.WriteUInt16(S_W_INIT);
	d.WriteUInt8(result);
	d.WriteUInt32(!result ? t->current_buffer->parent : 0);
	w_server_send_async(t->current_buffer->parent, &d);

	if (result) {
		nodes[t->current_buffer->parent].active = false;
		t->current_buffer->data.Clear();
		shutdown(nodes[t->current_buffer->parent].socket, 2);
		closesocket(nodes[t->current_buffer->parent].socket);

		printf("WORLD_NODE_REJECTED [%s]\n", result == 1 ? "AREA_REPLICATION" : "CHANNEL_REPLICATION");
	}
	return;
}

void WINAPI op_close(w_s_working_thread * t) {
	w_server_get_node(t->current_buffer->parent)->active = false;
	w_server_get_node(t->current_buffer->parent)->data.data.Clear();
	shutdown(w_server_get_node(t->current_buffer->parent)->socket, SD_BOTH);
	closesocket(w_server_get_node(t->current_buffer->parent)->socket);

	printf("WORLD_NODE_CLOSE_REQUESTED ID[%d]! \n", t->current_buffer->parent);
	return;
}

void WINAPI op_player_enter_world(w_s_working_thread *t) {
	Stream data_m;
	Stream data_y;

	Stream &data = t->current_buffer->data;
	uint32 node_id = data.ReadUInt32();
	uint8  result = data.ReadUInt8();
	uint32 id = data.ReadUInt32();
	std::shared_ptr<connection> c = arbiter_get_connection(id);
	std::shared_ptr<player> p = c->players[c->selected_player];

	std::vector<uint32> visible_players;
	if (result)
	{
		uint32 count = data.ReadUInt32();
		if (count) {

			player_write_spawn_packet(p, data_m);

			uint32 id_2;
			for (uint32 i = 0; i < count; i++) {
				id_2 = data.ReadUInt32();
				visible_players.push_back(id_2);

				//player_write_spawn_packet(arbiter_get_player(id_2), data_y, 0x00);

				arbiter_send(id_2, &data_m);
				//arbiter_send(id, &data_y);
				data_y.Clear();
			}
		}
		c->node_id = node_id;
	}


	io_player_enter_world(c, result, visible_players);
	return;
}

void WINAPI op_player_leave_world(w_s_working_thread *t) {
	uint32 c_id = t->current_buffer->data.ReadUInt32();
	uint16 op = t->current_buffer->data.ReadUInt16();
	io_exectue((e_io_opcodes)op, arbiter_get_connection(c_id), t->current_buffer->data);
	return;
}

void WINAPI op_player_move_broadcast(w_s_working_thread *t) {
	Stream &data = t->current_buffer->data;
	uint32 c_id = data.ReadUInt32();
	std::shared_ptr<player> pp = arbiter_get_player(c_id);
	if (!pp) return;

	int16 s_t[10];
	s_t[0] = data.ReadUInt16(); //p_s  offset
	s_t[1] = data.ReadUInt16(); //p_s count
	s_t[2] = data.ReadUInt16(); //p_d  offset
	s_t[3] = data.ReadUInt16();	//p_d count
	s_t[4] = data.ReadUInt16(); //p_m  offset
	s_t[5] = data.ReadUInt16(); //p_m count



	Stream d_s;


	if (s_t[1]) { //spawn
		uint32 id_d;
		Stream s_y; //you
		d_s.Clear(); //me
		player_write_spawn_packet(arbiter_get_player(c_id), d_s);
		data._pos = s_t[0] - 4;
		for (uint32 i = 0; i < s_t[1]; i++) {
			s_y.Clear();
			id_d = data.ReadUInt32();
			player_write_spawn_packet(arbiter_get_player(id_d), s_y);

			
			arbiter_send(c_id, &s_y);
			arbiter_send(id_d, &d_s);
		}
		chat_send_simple_system_message("Sapwned player!", arbiter_get_player(c_id));
	}

	
	if (s_t[3]) { //despawn
		uint32 id_d;
		Stream d_m;
		d_m.Resize(13);
		d_m.WriteUInt16(13);
		d_m.WriteUInt16(S_DESPAWN_USER);
		d_m.WriteUInt32(c_id);
		d_m.WriteUInt32(SERVER_ID);

		d_s.Clear();
		d_s.Resize(13);
		d_s.WriteUInt16(13);
		d_s.WriteUInt16(S_DESPAWN_USER);
		
		data._pos = s_t[2] - 4;
		for (int16 i = 0; i < s_t[3]; i++) {
			id_d = data.ReadUInt32();
			
			d_s._pos = 4;
			d_s.WriteUInt32(id_d);
			d_s.WriteUInt32(SERVER_ID);
			arbiter_send(c_id, &d_s);
			arbiter_send(id_d, &d_m);
			chat_send_simple_system_message("Desapwned player from other!", arbiter_get_player(id_d));
		}

		chat_send_simple_system_message("Desapwned player!", arbiter_get_player(c_id));

	}


	if (s_t[5]) { //move
		data._pos = s_t[4] - 4;
		s_t[6] = data.ReadInt16();	  //heading
		s_t[7] = data.ReadInt16();	  //type
		s_t[8] = data.ReadInt16();	  //speed
		s_t[9] = data.ReadInt16();	  //time

		float t_t[6];
		t_t[0] = data.ReadFloat();
		t_t[1] = data.ReadFloat();
		t_t[2] = data.ReadFloat();
		t_t[3] = data.ReadFloat();
		t_t[4] = data.ReadFloat();
		t_t[5] = data.ReadFloat();


		d_s.Resize(49);
		d_s.WriteUInt16(49);
		d_s.WriteUInt16(S_USER_LOCATION);
		d_s.WriteInt32(c_id);
		d_s.WriteInt32(SERVER_ID);

		d_s.WriteFloat(t_t[0]); // (&data._raw[data._pos], 12);
		d_s.WriteFloat(t_t[1]);
		d_s.WriteFloat(t_t[2]);
		

		d_s.WriteInt32(s_t[6]);
		d_s.WriteInt16(s_t[7] == 5 ?s_t[8] : pp->stats.get_movement_speed(a_load(pp->status)));

		//d_s.Write(&data._raw[data._pos], 12);

		d_s.WriteFloat(t_t[3]);
		d_s.WriteFloat(t_t[4]);
		d_s.WriteFloat(t_t[5]);


		d_s.WriteUInt32(s_t[7]);
		d_s.WriteUInt8(0);

		for (uint32 i = 0; i < s_t[5]; i++) {
			uint32 id_t = data.ReadUInt32();
			arbiter_send(id_t, &d_s);
		}
	}
	

	return;
}

void WINAPI op_get_visible_players(w_s_working_thread *t) {
	e_io_opcodes opcode = (e_io_opcodes)t->current_buffer->data.ReadUInt16();
	uint32 c_id = t->current_buffer->data.ReadUInt32();
	std::shared_ptr<connection> c = arbiter_get_connection(c_id);
	if (!c) {
		printf("CONNECTION_NULL! op_get_visible_players\n");
		return;
	}

	io_exectue(opcode, std::move(c), t->current_buffer->data);
	return;
}
