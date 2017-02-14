#ifndef CONFIG_H
#define CONFIG_H

#define _CRT_SECURE_NO_WARNINGS

#include "win32.h"
#include "typeDefs.h"
#include <string>



class config
{
public:
	struct server_c
	{
		uint32
			arbiter_no_of_threads,
			arbiter_job_pull_cout,
			world_no_of_threads_per_cpu,
			world_job_pull_cout,
			active_max_no_of_threads,
			active_max_no_of_players,
			active_idle_sleep_time_ms,
			active_sleep_time_ms;
		uint32 server_Id,
			buffer_edit_enable,
			buffer_width,
			buffer_height,
			buffer_color;
		char server_Name[32];
	} static server;

	struct net_c
	{
		bool
			localhost;

		char
			ip[16];

		uint16
			port;
	} static net;

	struct dir_c
	{
		char
			dataNpc[FILENAME_MAX],
			dataNpcSkills[FILENAME_MAX],
			dataPlayerSkills[FILENAME_MAX],
			dataItems[FILENAME_MAX],
			dataAbnormalities[FILENAME_MAX],
			dataPassivities[FILENAME_MAX],
			dataEquipment[FILENAME_MAX],
			dataWorld[FILENAME_MAX],
			dataScripts[FILENAME_MAX];
	} static dir;

	struct mysql_c
	{
		char mysqlIp[16];
		uint32 mysqlPort;
		char mysqlUser[32];
		char mysqlPassword[32];
		char mysqlDbName[16];
	} static mysql;

	struct player_c
	{
		char additional_start_items[150];
		uint32 start_inventory_slot_count;
		uint64 start_gold;
		uint64 max_glod;

	} static player;

	struct area_c
	{
		float zone_start_position[3];
		int16 zone_start_heading;
		uint32 zone_start_continent;
		uint32 zone_start_world_map_wrold_id;
		uint32 zone_start_world_map_guard_id;
		uint32 zone_start_world_map_section_id;
		uint32 zone_start_channel;
	}static zone;

	struct chat_c
	{
		uint32 worker_threads_count;
		uint32 job_pull_count;
		uint32 enable_gm_commands;
		uint32 player_use_gm_commands;
		uint32 run;
	} static chat;
};



bool init_config(STRING fileName);

#endif
