#include "config.h"

#include "stringUtils.h"
#include <fstream>



bool init_config(STRING fileName)
{
	std::fstream file = std::fstream(fileName);
	if (!file.is_open())
		return false;
	std::string error_message;
	std::string line;
	uint32 lineCount = 0;
	while (!file.eof())
	{
		std::getline(file, line);
		lineCount++;

		if (
			(line[0] == '/' && line[1] == '/') ||
			(line[0] == '#')
			)
			continue;
		else if (stringStartsWith(line, "server.name"))
		{
			if (!sscanf(line.c_str(), "server.name=%s", config::server.server_Name))
			{
				error_message = "server_name max 32 characters!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.id"))
		{
			if (!sscanf(line.c_str(), "server.id=%d", &config::server.server_Id))
			{
				error_message = "bad server_id value!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.buffer_edit_enable"))
		{
			if (!sscanf(line.c_str(), "server.buffer_edit_enable=%d", &config::server.buffer_edit_enable))
			{
				error_message = "bad buffer_edit_enable value! use[1/0]";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.buffer_width"))
		{
			if (!sscanf(line.c_str(), "server.buffer_width=%d", &config::server.buffer_width))
			{
				error_message = "bad buffer_width value!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.buffer_height"))
		{
			if (!sscanf(line.c_str(), "server.buffer_height=%d", &config::server.buffer_height))
			{
				error_message = "bad buffer_height value!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.buffer_color"))
		{
			if (!sscanf(line.c_str(), "server.buffer_color=%d", &config::server.buffer_color))
			{
				error_message = "bad buffer_color value! 0x[xy] x->background_color y->foreground_color x,y->[0,F]";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.arbiter.number_of_threads"))
		{
			if (!sscanf(line.c_str(), "server.arbiter.number_of_threads= %d", &config::server.arbiter_no_of_threads))
			{
				error_message = "bad number_of_threads_per_cpu value!";
				goto error_proc;
			}

			if (config::server.arbiter_no_of_threads <= 0 ||
				config::server.arbiter_no_of_threads > 50)
			{
				error_message = "value within [1 - 50]!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.arbiter.job_pull_count"))
		{
			if (!sscanf(line.c_str(), "server.arbiter.job_pull_count= %d", &config::server.arbiter_job_pull_cout))
			{
				error_message = "bad arbiter_job_pull_cout value!";
				goto error_proc;
			}

			if (config::server.arbiter_job_pull_cout <= 0 ||
				config::server.arbiter_job_pull_cout > 32)
			{
				error_message = "value within [1 - 32]!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.world.number_of_threads_per_cpu"))
		{
			if (!sscanf(line.c_str(), "server.world.number_of_threads_per_cpu= %d", &config::server.world_no_of_threads_per_cpu))
			{
				error_message = "bad world_no_of_threads_per_cpu value!";
				goto error_proc;
			}

			if (config::server.world_no_of_threads_per_cpu <= 0 ||
				config::server.world_no_of_threads_per_cpu > 50)
			{
				error_message = "value within [1 - 50]!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.world.job_pull_count"))
		{
			if (!sscanf(line.c_str(), "server.world.job_pull_count= %d", &config::server.world_job_pull_cout))
			{
				error_message = "bad world_job_pull_cout value!";
				goto error_proc;
			}

			if (config::server.world_job_pull_cout <= 0 ||
				config::server.world_job_pull_cout > 32)
			{
				error_message = "value within [1 - 32]!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.active.max_number_of_threads"))
		{
			if (!sscanf(line.c_str(), "server.active.max_number_of_threads= %d", &config::server.active_max_no_of_threads))
			{
				error_message = "bad max_number_of_threads value!";
				goto error_proc;
			}

			if (config::server.active_max_no_of_threads <= 0 ||
				config::server.active_max_no_of_threads > 30)
			{
				error_message = "value within [1 - 30]!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.active.max_number_of_players"))
		{
			if (!sscanf(line.c_str(), "server.active.max_number_of_players= %d", &config::server.active_max_no_of_players))
			{
				error_message = "bad max_number_of_players value!";
				goto error_proc;
			}

			if (config::server.active_max_no_of_players <= 0 ||
				config::server.active_max_no_of_players > 600)
			{
				error_message = "value within [1 - 600]!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.active.idle_sleep_time_ms"))
		{
			if (!sscanf(line.c_str(), "server.active.idle_sleep_time_ms= %d", &config::server.active_idle_sleep_time_ms))
			{
				error_message = "bad idle_sleep_time_ms value!";
				goto error_proc;
			}
			if (config::server.active_idle_sleep_time_ms > 10000)
			{
				error_message = "value within [0 - 10000]!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.active.sleep_time_ms"))
		{
			if (!sscanf(line.c_str(), "server.active.sleep_time_ms= %d", &config::server.active_sleep_time_ms))
			{
				error_message = "bad sleep_time_ms value!";
				goto error_proc;
			}
			if (config::server.active_sleep_time_ms > 5000)
			{
				error_message = "value within [0 - 5000]!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.chat.worker_threads_count"))
		{
			if (!sscanf(line.c_str(), "server.chat.worker_threads_count= %d", &config::chat.worker_threads_count))
			{
				error_message = "bad worker_threads_count value!";
				goto error_proc;
			}

			if (config::chat.worker_threads_count <= 0 ||
				config::chat.worker_threads_count > 10)
			{
				error_message = "value within [1 - 10]!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.chat.job_pull_count"))
		{
			if (!sscanf(line.c_str(), "server.chat.job_pull_count= %d", &config::chat.job_pull_count))
			{
				error_message = "bad job_pull_count value!";
				goto error_proc;
			}

			if (config::chat.job_pull_count <= 0 ||
				config::chat.job_pull_count > 10)
			{
				error_message = "value within [1 - 10]!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.chat.enable_gm_commands"))
		{
			if (!sscanf(line.c_str(), "server.chat.enable_gm_commands= %d", &config::chat.enable_gm_commands))
			{
				error_message = "bad enable_gm_commands value! use '1' for [true] and '0' for [false]";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.chat.run"))
		{
			if (!sscanf(line.c_str(), "server.chat.run= %d", &config::chat.run))
			{
				error_message = "bad run value! use '1' for [true] and '0' for [false]";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.localhost"))
		{
			int32 value;
			if (!sscanf(line.c_str(), "server.localhost= %d", &value))
				goto error_proc;
			if (value)config::net.localhost = true;
			else config::net.localhost = false;
			
		}
		else if (stringStartsWith(line, "server.ip"))
		{
			if (!sscanf(line.c_str(), "server.ip= %s", config::net.ip))
			{
				error_message = "wrong ip format!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.port"))
		{
			if (!sscanf(line.c_str(), "server.port= %hu", &config::net.port))
				goto error_proc;
		}
		else if (stringStartsWith(line, "server.mysql.ip"))
		{
			if (!sscanf(line.c_str(), "server.mysql.ip= %s", config::mysql.mysqlIp))
			{
				error_message = "wrong ip format!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.mysql.port"))
		{
			if (!sscanf(line.c_str(), "server.mysql.port= %d", &config::mysql.mysqlPort))
			{
				error_message = "bad port value!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.mysql.db_name"))
		{
			if (!sscanf(line.c_str(), "server.mysql.db_name= %s", config::mysql.mysqlDbName))
			{
				error_message = "db_name max 16 characters!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.mysql.username"))
		{
			if (!sscanf(line.c_str(), "server.mysql.username= %s", &config::mysql.mysqlUser))
			{
				error_message = "username max 32 characters!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.mysql.password"))
		{
			if (!sscanf(line.c_str(), "server.mysql.password= %s", &config::mysql.mysqlPassword))
			{
				error_message = "password max 32 characters!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.dir.npc"))
		{
			if (!sscanf(line.c_str(), "server.dir.npc= %s", config::dir.dataNpc))
			{
				error_message = "npcs dir. max 259 characters!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.dir.skills.player"))
		{
			if (!sscanf(line.c_str(), "server.dir.skills.player= %s", config::dir.dataPlayerSkills))
			{
				error_message = "player skills dir. max 259 characters!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.dir.skills.npc"))
		{
			if (!sscanf(line.c_str(), "server.dir.skills.npc= %s", config::dir.dataNpcSkills))
			{
				error_message = "npc skills dir. max 259 characters!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.dir.items"))
		{
			if (!sscanf(line.c_str(), "server.dir.items= %s", &config::dir.dataItems))
			{
				error_message = "items dir. max 259 characters!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.dir.abnormalities"))
		{
			if (!sscanf(line.c_str(), "server.dir.abnormalities= %s", &config::dir.dataAbnormalities))
			{
				error_message = "abnormalities dir. max 259 characters!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.dir.passivities"))
		{
			if (!sscanf(line.c_str(), "server.dir.passivities= %s", &config::dir.dataPassivities))
			{
				error_message = "passivities dir. max 259 characters!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.dir.equipment"))
		{
			if (!sscanf(line.c_str(), "server.dir.equipment= %s", &config::dir.dataEquipment))
			{
				error_message = "equipment dir. max 259 characters!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.dir.world"))
		{
			if (!sscanf(line.c_str(), "server.dir.world= %s", config::dir.dataWorld))
			{
				error_message = "world dir. max 259 characters!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "server.dir.scripts"))
		{
			if (!sscanf(line.c_str(), "server.dir.scripts= %s", config::dir.dataScripts))
			{
				error_message = "scripts dir. max 259 characters!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "player.additional_start_items"))
		{
			if (!sscanf(line.c_str(), "player.additional_start_items= %s", config::player.additional_start_items))
			{
				error_message = "additional_start_items max 149 characters!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "player.inventory.start_slot_count"))
		{
			if (!sscanf(line.c_str(), "player.inventory.start_slot_count= %lu", &config::player.start_inventory_slot_count))
			{
				error_message = "bad start_inventory_slot_count max[72] value!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "player.inventory.max_glod"))
		{
			if (!sscanf(line.c_str(), "player.inventory.max_glod= %llu", &config::player.max_glod))
			{
				error_message = "bad max_glod max[0xffffffffffffffffu] value!";
				goto error_proc;
			}

			if (config::player.max_glod == 0) config::player.max_glod = UINT64_MAX;
		}
		else if (stringStartsWith(line, "player.inventory.start_gold"))
		{
			if (!sscanf(line.c_str(), "player.inventory.start_gold = %llu", &config::player.start_gold))
			{
				error_message = "bad start_gold max[0xffffffffffffffffu] value!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "zone.start_positon"))
		{
			if (!sscanf(line.c_str(), "zone.start_positon= %f %f %f", &config::zone.zone_start_position[0], &config::zone.zone_start_position[1], &config::zone.zone_start_position[2]))
			{
				error_message = "bad position value! [x] [y] [z]";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "zone.start_heading"))
		{
			if (!sscanf(line.c_str(), "zone.start_heading= %hd", &config::zone.zone_start_heading))
			{
				error_message = "bad heading value!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "zone.start_continent"))
		{
			if (!sscanf(line.c_str(), "zone.start_continent= %d", &config::zone.zone_start_continent))
			{
				error_message = "bad start_continent value!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "zone.start_channel"))
		{
			if (!sscanf(line.c_str(), "zone.start_channel= %d", &config::zone.zone_start_channel))
			{
				error_message = "bad start_channel value!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "zone.start_world_map_world_id"))
		{
			if (!sscanf(line.c_str(), "zone.start_world_map_world_id= %d", &config::zone.zone_start_world_map_wrold_id))
			{
				error_message = "bad start_world_map_world_id value!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "zone.start_world_map_guard_id"))
		{
			if (!sscanf(line.c_str(), "zone.start_world_map_guard_id= %d", &config::zone.zone_start_world_map_guard_id))
			{
				error_message = "bad start_world_map_guard_id value!";
				goto error_proc;
			}
		}
		else if (stringStartsWith(line, "zone.start_world_map_section_id"))
		{
			if (!sscanf(line.c_str(), "zone.start_world_map_section_id= %d", &config::zone.zone_start_world_map_section_id))
			{
				error_message = "bad start_world_map_section_id value!";
				goto error_proc;
			}
		}

	}

	file.close();

	return true;

error_proc:
	printf("\nFailed to load config_file. ERROR LINE-NO[%d]\nLINE[%s]\nMESSAGE[%s]\n", lineCount, line.c_str(), error_message.c_str());
	file.close();
	return false;
}

config::dir_c			config::dir;
config::server_c		config::server;
config::mysql_c			config::mysql;
config::net_c			config::net;
config::player_c		config::player;
config::area_c			config::zone;
config::chat_c			config::chat;