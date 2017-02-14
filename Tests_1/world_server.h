#ifndef WORLD_H
#define WORLD_H

#include "typeDefs.h"
#include "config.h"
#include "win32.h"
#include "playerEnums.h"
#include "jobEnums.h"
#include "worldEnums.h"
#include "continent.h"
#include "pms.h"

#include "job.h"

#include <atomic>

class job_head; class player; struct s_worker_thread;


struct world_data
{
	std::vector<continent>	_w;
};

struct s_worker_thread
{
	job_todo*				j_c;
	uint64						out_jobs;
	uint64						noOfBytesTransfered;
	uint64						id;
	HANDLE						w_thread;
};

struct world_server
{
	std::shared_ptr<world_data>				w_data;
	HANDLE									shutdown_event;
	HANDLE									w_iocp;
	std::atomic_uint64_t					p_count;
	uint64									no_of_threads;
	uint64									no_of_droped_jobs;
	s_worker_thread**						world_worker_threads;
	SYSTEM_INFO								sys_info;
	char									name[64];
} static w_server;




DWORD WINAPI world_worker(void* argv);
void WINAPI world_process_job(void*, e_job_type, s_worker_thread *);
bool WINAPI world_server_process_job_async(void*, e_job_type);

bool WINAPI world_server_init();
void WINAPI world_server_release();

uint64 WINAPI world_get_player_count();

static int32 WINAPI world_get_contienent_by_identity(uint32 identity);

static void WINAPI world_player_enter_world(j_enter_world*  j);
static void	WINAPI world_player_exit_world(j_exit_world* j);
static void WINAPI world_player_move(j_move* j);
#endif
