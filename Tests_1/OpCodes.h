#ifndef OPCODESMM_H
#define OPCODESMM_H

#include "typeDefs.h"
#include "opcodeEnum.h"

#include <memory>

//#define OP_DUMP 1
#define SQL_DEBUG 1

class connection; 

#ifndef OP_FUNCTION
typedef bool(WINAPI *op_function)(std::shared_ptr<connection>, void* argv[]);
#endif


static op_function opcodes_[OPCODE_MAX];
static uint32 opcode_count;

op_function opcode_resolve(e_opcode);
static bool opcode_add(e_opcode, op_function);

bool opcode_init();
void opcode_release();

//init
bool WINAPI op_check_version(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_login_arbiter(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_set_visible_range(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_get_user_list(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_hardware_info(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_request_vip_system_info(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_get_user_guild_logo(std::shared_ptr<connection>, void* argv[]);
//lobby
bool WINAPI op_can_create_player(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_create_player(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_change_user_slot_id(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_check_username(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_str_evaluate_string(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_select_player(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_delete_player(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_cancel_delete_player(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_save_client_account_settings(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_save_client_user_settings(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_exit(std::shared_ptr<connection> c, void* argv[]);

//world
bool WINAPI op_load_topo_fin(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_return_to_lobby(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_cancel_return_to_lobby(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_guard_pk_policy(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_reign_info(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_request_gamestat_ping(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_social(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_visit_new_section(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_simple_tip_repeate_checl(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_tradebroker_heighes_item_level(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_server_time(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_update_contents_playtime(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_request_ingame_product_list(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_player_location(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_event_guide(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_set_item_string(std::shared_ptr<connection>, void* argv[]);

//misc
bool WINAPI op_show_inven(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_equipe_item(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_unequipe_item(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_dungeon_clear_count_list(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_dungeon_cooltime_list(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_request_user_itemlevel_info(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_npc_guild_list(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_view_battlefield_result(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_chat(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_del_item(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_show_item_tooltip_ex(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_move_inven_pos(std::shared_ptr<connection>, void* argv[]);

//contract
bool WINAPI op_request_contract(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_cancel_contract(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_bind_item_begin_progress(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_bind_item_execute(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_execute_temper(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_cancel_temper(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_play_execute_temper(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_add_to_temper_material_ex(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_check_unidentify_items(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_random_passive_lock(std::shared_ptr<connection>, void* argv[]);
bool WINAPI op_unidentify_execute(std::shared_ptr<connection>, void* argv[]);

//guild
bool WINAPI op_request_guild_info(std::shared_ptr<connection> c, void* argv[]);
#endif


