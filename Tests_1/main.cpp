

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <iostream>

#include "win32.h"
#include "logService.h"
#include "arbiter_server.h"
#include "world_server.h"
#include "OpCodes.h"
#include "config.h"
#include "dataservice.h"
#include "chat.h"
#include "entity_manager.h"
#include "active_server.h"
#include "skylake_stats.h"

void release();

int main(int32 argc, char** argv)
{
	srand((unsigned int)time(0));

	std::string cmdLine;

	if (!init_config(".//data//config.ini"))
		goto error_proc;

	if (config::server.buffer_edit_enable)
	{
		HANDLE _handler = GetStdHandle(STD_OUTPUT_HANDLE);
		COORD dc = COORD();
		dc.X = config::server.buffer_width;
		dc.Y = config::server.buffer_height;
		SetConsoleScreenBufferSize(_handler, dc);

		ShowWindow(GetConsoleWindow(), SW_MAXIMIZE);
		SetConsoleTextAttribute(_handler, config::server.buffer_color);
	}

	TITLE(config::server.server_Name);

	if (!s_stats_load_scripts())
	{
		LOG("\nStats load failed!");
		goto error_proc;
	}

	printf("::LOADED_STATS_SCRIPTS!\n\n");

	entity_manager::init();

	if (!active_server_init())
	{
		LOG("\nActive_server init failed!");
		goto error_proc;
	}

	if (!chat_init())
	{
		LOG("\nChat_server init failed!");
		goto error_proc;
	}

	if (!opcode_init())
	{
		LOG("\nOpcodes init failed!");
		goto error_proc;
	}

	if (!mysql_init())
	{
		LOG("\nMysql init failed!");
		goto error_proc;
	}

	if (!world_server_init())
	{
		LOG("\nWORLD_SERVER_INIT FAILED!");
		goto error_proc;
	}

	if (!data_service::data_init())
	{
		LOG("\nData init failed!");
		goto error_proc;
	}

	if (!arbiter_server_init())
	{
		LOG("\nARBITER_SERVER_INIT FAILED!");
		goto error_proc;
	}


#pragma region cmdLine

	printf("::SKY_LAKE UP!\n::ACCEPTING CONNECTIONS...\n\n");

	while (1)
	{
		std::cin >> cmdLine;

		if (cmdLine == "exit")
		{
			break;
		}
		else if (cmdLine == "rlstats")
		{
			if (!s_stats_load_scripts())
			{
				LOG("\nStats load failed!");
			}
			else
				LOG("\nStats load success!");
		}
		else if (cmdLine == "cls")
			system("cls");
		Sleep(0);
	}
#pragma endregion

	release();
	system("pause");
	return 0;

error_proc:

	release();
	LOG("\nERROR!\n");

	_getch();
	return 0;
}

void release()
{
	chat_release();
	data_service::data_release();
	world_server_release();
	arbiter_server_release();
	active_server_release();
	opcode_release();
	mysql_release();


	entity_manager::release();
}