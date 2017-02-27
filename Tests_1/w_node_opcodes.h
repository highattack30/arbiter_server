#ifndef W_NODE_OPCODE_FN_H
#define W_NODE_OPCODE_FN_H

#include "typeDefs.h"
#include "win32.h"
#include "world_server_opcodes.h"

struct w_s_working_thread;
typedef void(WINAPI * w_opcode_fn)(w_s_working_thread*);

static w_opcode_fn op_fns[WOP_MAX];

void WINAPI w_op_fn_init();

w_opcode_fn get_w_op_fn(e_world_node_opcode);

void WINAPI op_init(w_s_working_thread*);
void WINAPI op_close(w_s_working_thread*);
void WINAPI op_player_enter_world(w_s_working_thread*);
void WINAPI op_player_leave_world(w_s_working_thread*);
void WINAPI op_player_move_broadcast(w_s_working_thread*);
void WINAPI op_get_visible_players(w_s_working_thread*);
#endif
