#include "active_server.h"

#include "config.h"
#include "player.h"
#include "servertime.h"

bool WINAPI active_server_init()
{
	if (NULL == (as_server.shutdown_event = WSACreateEvent()))
		return false;

	InitializeCriticalSection(&as_server.update_lock);

	as_server.threads = new as_thread[config::server.active_max_no_of_threads];
	for (uint32 i = 0; i < config::server.active_max_no_of_threads; i++)
	{
		a_store(as_server.threads[i].freez, true);
		as_server.threads[i].thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)active_server_thread, (LPVOID)&as_server.threads[i], 0, 0);
	}


	return true;
}

void WINAPI active_server_release()
{
	WSASetEvent(as_server.shutdown_event);

	for (uint32 i = 0; i < config::server.active_max_no_of_threads; i++)
	{
		if (as_server.threads[i].thread)
		{
			a_store(as_server.threads[i].freez, false);
			WaitForSingleObject(as_server.threads[i].thread, INFINITE);
			as_server.threads[i].players.clear();
		}
	}
	delete[] as_server.threads;

	CloseHandle(as_server.shutdown_event);
	DeleteCriticalSection(&as_server.update_lock);
}

void WINAPI active_add_player(std::shared_ptr<player> p)
{
	for (uint32 i = 0; i < config::server.active_max_no_of_threads; i++)
	{
		if (!a_load(as_server.threads[i].freez) &&
			as_server.threads[i].players.get_count() < config::server.active_max_no_of_players)
		{
			as_server.threads[i].players.push_or_remove(p);
			printf("PUSH_FRONT_PLAYER_ACTIVE_SERVER [%d]\n", as_server.threads[i].players.get_count());
			return;
		}
	}
	

	EnterCriticalSection(&as_server.update_lock);
	for (uint32 i = 0; i < config::server.active_max_no_of_threads; i++)
	{
		if (!a_load(as_server.threads[i].freez) &&
			as_server.threads[i].players.get_count() < config::server.active_max_no_of_players)
		{
			as_server.threads[i].players.push_or_remove(p);
			printf("PUSH_FRONT_PLAYER_ACTIVE_SERVER [%d]\n", as_server.threads[i].players.get_count());
			LeaveCriticalSection(&as_server.update_lock);
			return;
		}
	}


	for (uint32 i = 0; i < config::server.active_max_no_of_threads; i++)
	{
		if (a_load(as_server.threads[i].freez))
		{
			as_server.threads[i].players.push_or_remove(p);
			printf("PUSH_FRONT_PLAYER_ACTIVE_SERVER [%d]\n", as_server.threads[i].players.get_count());
			a_store(as_server.threads[i].freez, false);
			break;
		}
	}
	LeaveCriticalSection(&as_server.update_lock);


	
}

void WINAPI active_remove_player(std::shared_ptr<player> p)
{
	uint32 t_id = as_server.p_threads[p->eid];
	as_server.p_threads.erase(p->eid);

	as_server.threads[t_id].players.push_or_remove(p, true);
#ifdef _DEBUG
	printf("REMOVED_PLAYER_FROM_ACTIVE_SERVER\n");
#endif
	EnterCriticalSection(&as_server.update_lock);
	for (uint32 i = 0; i < config::server.active_max_no_of_threads; i++)
	{
		if (a_load(as_server.threads[i].freez) &&
			!as_server.threads[i].players.get_count())
		{
			a_store(as_server.threads[i].freez, true);
		}
	}
	LeaveCriticalSection(&as_server.update_lock);
}

DWORD WINAPI active_server_thread(LPVOID argv)
{
	as_thread* this_ = (as_thread*)argv;
	if (!this_) return 1;
	if (!QueryPerformanceFrequency(&this_->i))
		return 1;
	this_->frequency_seconds = (double)(this_->i.QuadPart);
	
	a_store(this_->alive, true);
enter_proc:
	while(a_load(this_->freez)) Sleep(config::server.active_idle_sleep_time_ms);
	this_->delta_time = this_->elapsed_time = 0.0f;
	QueryPerformanceCounter(&this_->i);
	this_->start = this_->i.QuadPart;
	while (1)
	{
		if (WAIT_OBJECT_0 == WaitForSingleObject(as_server.shutdown_event, 0)) { break; }
		if (a_load(this_->freez)) goto enter_proc;

#pragma region time
		QueryPerformanceCounter(&this_->i);
		this_->delta_time = (double)(this_->i.QuadPart - this_->start) / this_->frequency_seconds;
		this_->start = this_->i.QuadPart;
		this_->elapsed_time += this_->delta_time;
#pragma endregion

		this_->players.update_players(this_->delta_time, this_->elapsed_time);

		Sleep(config::server.active_sleep_time_ms);
	}


	a_store(this_->alive, false);
	printf("::ACTIVE_SERVER_THREAD_CLOSED\n");
	return 0;
}
