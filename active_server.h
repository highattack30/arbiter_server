#ifndef ACTIVE_SERVER_H
#define ACTIVE_SERVER_H

#include "player_rcu.h"

#include "win32.h"
#include "typeDefs.h"

#include <map>

struct as_thread
{
	player_rcu players;
	HANDLE	thread;
	std::atomic_bool freez;
	std::atomic_bool alive;
	LARGE_INTEGER i;
	LONGLONG	start;
	double frequency_seconds;
	double delta_time;
	double elapsed_time;
};

struct active_server {
	
	WSAEVENT shutdown_event;
	as_thread* threads;
	CRITICAL_SECTION update_lock;

	std::map<uint64, uint32> p_threads;

} static as_server;

bool WINAPI active_server_init();
void WINAPI active_server_release();

void WINAPI active_add_player(std::shared_ptr<player>);
void WINAPI active_remove_player(std::shared_ptr<player>);

static DWORD WINAPI active_server_thread(LPVOID);

#endif
