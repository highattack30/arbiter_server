#ifndef SERVERDEFS_H
#define SERVERDEFS_H

#include <stdio.h>
#include <mutex>
#include <atomic>

#include "win32.h"
#include "typeDefs.h"
#include "logService.h"
#include "serverEnums.h"
#include "DBHandler.h"
#include "jobEnums.h"
#include "job.h"

class world; class connection;
#define WSA_VERSION MAKEWORD(2,2)

struct a_worker_thread
{
	job_todo*						j_c;
	ULONG							out_jobs;
	uint64							noOfBytesTransfered;
	uint64							id;
	HANDLE							a_thread;
};

struct server_data
{
	std::shared_ptr<connection>		clients[SBS_SERVER_MAX_CLIENTS];
	HANDLE							server_iocp;
	SOCKET							listeningSocket;
	struct sockaddr_in				listenerData;
	WSAEVENT						acceptEvent;
	WSAEVENT						shutdownEvent;
	DWORD							acceptThreadID;
	HANDLE							acceptThread;
	uint32							noOfThreads;
	a_worker_thread*				worker_threads;
	uint32							no_of_droped_jobs;
	WSADATA							wsaData;
	std::mutex						clientListMutex;
	std::atomic_uint64_t			clientsCount;
	CRITICAL_SECTION				log_sync;
};

static server_data a_server;

bool WINAPI arbiter_server_process_job_async(void* j, e_job_type);

DWORD WINAPI arbiter_server_worker_func(void* argv);
DWORD WINAPI arbiter_server_accept_func(void* argv);

bool WINAPI arbiter_server_init();
void WINAPI arbiter_server_release();

bool WINAPI arbiter_server_connexion_add(SOCKET sock, sockaddr_in sockData);
bool WINAPI arbiter_server_connexion_remove(uint32 id);

void server_lock_log();
void server_unlock_log();

uint64	WINAPI arbiter_get_connection_count();

static void WINAPI arbiter_server_process_job(void* j,e_job_type, sql::Connection * con);
#endif // !SERVERDEFS_H

