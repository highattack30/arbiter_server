#include "arbiter_server.h"
#include "world_server.h"
#include "config.h"
#include "connexion.h"
#include "job.h"
#include "Stream.h"
#include "DBHandler.h"
#include "itemEnums.h"
#include "internalOp.h"
#include "inventory.h"
#include "servertime.h"
#include "continent.h"
#include "active_server.h"

bool WINAPI arbiter_server_init() {

	InitializeCriticalSection(&a_server.log_sync);

	a_server.noOfThreads = config::server.arbiter_no_of_threads;
	a_server.worker_threads = new a_worker_thread[a_server.noOfThreads];
	if (WSAStartup(WSA_VERSION, &a_server.wsaData) != NO_ERROR)
		return false;
	a_server.shutdownEvent = CreateEvent(NULL, true, false, NULL);

	if (NULL == (a_server.server_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)))
		return false;


	for (uint32 i = 0; i < a_server.noOfThreads; i++)
	{
		a_server.worker_threads[i].noOfBytesTransfered = 0;
		a_server.worker_threads[i].out_jobs = 0;
		a_server.worker_threads[i].j_c = NULL;
		a_server.worker_threads[i].id = i;
		if (!(a_server.worker_threads[i].a_thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)arbiter_server_worker_func, &a_server.worker_threads[i], 0, NULL))) return false;
	}


	/*-----------------------------------------------------------*/
	a_server.listenerData.sin_addr.S_un.S_addr = config::net.localhost ? INADDR_ANY : inet_addr(config::net.ip);
	a_server.listenerData.sin_family = AF_INET;
	a_server.listenerData.sin_port = htons(config::net.port); // config.net.port

	if (
		((a_server.listeningSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) ||
		(SOCKET_ERROR == (bind(a_server.listeningSocket, (const sockaddr *)&a_server.listenerData, sizeof(a_server.listenerData)))) ||
		(SOCKET_ERROR == (listen(a_server.listeningSocket, SOMAXCONN))) ||
		((a_server.acceptEvent = WSACreateEvent()) == WSA_INVALID_EVENT) ||
		(SOCKET_ERROR == (WSAEventSelect(a_server.listeningSocket, a_server.acceptEvent, FD_ACCEPT)))
		)
		return false;

	a_server.acceptThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)arbiter_server_accept_func, (void*)&a_server.listeningSocket, 0, NULL);

	return true;
}

void WINAPI arbiter_server_release()
{
	CloseHandle(a_server.server_iocp);
	SetEvent(a_server.shutdownEvent);
	WaitForSingleObject(a_server.acceptThread, INFINITE);

	PostQueuedCompletionStatus(a_server.server_iocp, 0, (DWORD)NULL, NULL);

	for (uint32 i = 0; i < a_server.noOfThreads; i++)
		WaitForSingleObject(a_server.worker_threads[i].a_thread, INFINITE);
	delete[] a_server.worker_threads;


	WSACloseEvent(a_server.acceptEvent);
	WSACloseEvent(a_server.shutdownEvent);

	closesocket(a_server.listeningSocket);
	shutdown(a_server.listeningSocket, SD_BOTH);

	for (uint32 i = 0; i < SBS_SERVER_MAX_CLIENTS; i++)
	{
		if (a_server.clients[i])
		{
			connection_close(a_server.clients[i]);
			a_server.clients[i] = nullptr;
		}
	}

	DeleteCriticalSection(&a_server.log_sync);
	WSACleanup();
	LOG("\n\nServer Closed!\n");

}

bool WINAPI arbiter_server_connexion_add(SOCKET sock, sockaddr_in sockData)
{
	if (a_server.clientsCount + 1 > SBS_SERVER_MAX_CLIENTS)
		return false;

	std::lock_guard<std::mutex> locker(a_server.clientListMutex);
	for (uint32 i = 0; i < SBS_SERVER_MAX_CLIENTS; i++)
	{
		if (!a_server.clients[i])
		{
			std::shared_ptr<connection> newConnexion = connection_create(sock, a_server.server_iocp, i);
			if (newConnexion)
			{
				a_server.clients[i] = std::move(newConnexion);
				a_server.clientsCount++;
				printf("new connection\n");
				return true;
			}
			else
				break;
		}
	}

	return false;
}

bool WINAPI arbiter_server_connexion_remove(uint32 id)
{
	if (id >= SBS_SERVER_MAX_CLIENTS)
		return false;
	std::shared_ptr<connection> c = NULL;

	{
		std::lock_guard<std::mutex> locker(a_server.clientListMutex);
		if (a_server.clients[id])
		{
			c = a_server.clients[id];
			a_server.clients[id] = nullptr;
		}

	}

	if (c)
	{
		if (!c->_inLobby)
		{
			world_server_process_job_async(new j_exit_world(c->_players[c->_selected_player]), J_W_PLAYER_EXIT_WORLD);
			active_remove_player(c->_players[c->_selected_player]);
		}

		connection_close(c);
		a_server.clientsCount--;

		printf("CONNEXION_REMOVED ID[%d]\n", id);
		return true;
	}

	return false;
}


void server_lock_log()
{
	EnterCriticalSection(&a_server.log_sync);
}

void server_unlock_log()
{
	LeaveCriticalSection(&a_server.log_sync);
}

bool WINAPI arbiter_server_process_job_async(void* j, e_job_type type)
{
	if (!j) return false;
	job* newJob = new job(j, type);

	j_enter_world_fin * f = (j_enter_world_fin*)j;

	BOOL result = PostQueuedCompletionStatus(a_server.server_iocp, 1, NULL, (LPOVERLAPPED)newJob);
	if (!result)
	{
		delete newJob;
		a_server.no_of_droped_jobs++;

#ifdef _DEBUG
		printf("ARBITER DROPPED JOB! TYPE[%d]  TOTAL_DROPPED_COUNT[%d]\n", type, a_server.no_of_droped_jobs);
#endif
	}
	return result ? true : false;
}

DWORD WINAPI arbiter_server_worker_func(void* argv)
{
	a_worker_thread * this_ = (a_worker_thread*)argv;
	this_->j_c = new job_todo[config::server.arbiter_job_pull_cout];
	memset(this_->j_c, 0, sizeof(job_todo) * config::server.arbiter_job_pull_cout);

	sql::Connection * sqlCon = mysql_get_driver()->open();
	if (!sqlCon)
	{
		printf("::WORKER_THREAD::SQL CONNECTION BAD!!\n");

		delete[] this_->j_c;
		this_->j_c = nullptr;
		return 1;
	}

	while (WAIT_OBJECT_0 != WaitForSingleObject(a_server.shutdownEvent, 0))
	{
		BOOL retVal = GetQueuedCompletionStatusEx(a_server.server_iocp, (LPOVERLAPPED_ENTRY)this_->j_c,
			config::server.arbiter_job_pull_cout, &this_->out_jobs, INFINITE, false);

		if (WAIT_OBJECT_0 == WaitForSingleObject(a_server.shutdownEvent, 0) || ERROR_ABANDONED_WAIT_0 == WSAGetLastError())
			break;

		if (!this_->j_c) continue;
	
		for (uint32 i = 0; i < this_->out_jobs; i++)
		{
			this_->noOfBytesTransfered = this_->j_c[i].dwNumberOfBytesTransferred;

			job* j = this_->j_c[i].job;
			if (j)
			{
				switch (j->type)
				{
				case J_A_RECV:
				{
					j_recv * j_ = (j_recv*)j->j_;
					if (j_)
					{
						if (this_->noOfBytesTransfered)
						{
							bool result = false;
							switch (j_->b_state)
							{
							case CP_TERA_PACKET_HEAD:
							case CP_TERA_PACKET_BODY:
							{
								result = connection_process_recv_packet(j_->con, (void**)&sqlCon, this_->noOfBytesTransfered);
							}break;

							case CP_KEY_1:
							{
								result = connexion_fill_key(j_->con, 1);
								if (result)
									result = j_->con->_session->send_server_key(j_->con->_socket, 1);

								if (result)
								{
									j_->con->_recvBuffer.recvStatus = CP_KEY_2;
									result = connection_recv(j_->con);
								}
							}break;

							case CP_KEY_2:
							{

								result = connexion_fill_key(j_->con, 3);
								if (result)
									result = j_->con->_session->send_server_key(j_->con->_socket, 2);
								if (result)
									result = connexion_crypt_init(j_->con);
								if (result)
								{
									j_->con->_recvBuffer.data.Clear();
									j_->con->_recvBuffer.data.Resize(4);
									j_->con->_recvBuffer.recvStatus = CP_TERA_PACKET_HEAD;
									result = connection_recv(j_->con);
								}
							}
							}break;

							if (!result)
							{
								printf("CONNEXION LOST:[%d]\n", j_->con->_id);
								arbiter_server_connexion_remove(j_->con->_id);
							}
						}
						else
						{
							printf("CONNEXION LOST:[%d]\n", j_->con->_id);
							arbiter_server_connexion_remove(j_->con->_id);
						}
					}
					else
					{
						printf("RECV INTERNAL ERROR\n");
					}

				}break;

				case J_A_SEND:
				{

				}break;


				default:
				{
					arbiter_server_process_job(j->j_, j->type, sqlCon);
				}break;

				}


				delete this_->j_c[i].job;
				this_->j_c[i].job = nullptr;
			}
			else
			{
				printf("ERROR_ARBITER_WORKER_THREAD::JOB NULL\n");
			}
		}
	}

	for (uint32 i = 0; i < config::server.arbiter_job_pull_cout; i++)
		if (this_->j_c[i].job) { delete this_->j_c[i].job;  this_->j_c[i].job = nullptr; }


	delete[] this_->j_c;
	this_->j_c = nullptr;

	if (sqlCon)
	{
		sqlCon->close();
		delete sqlCon;
	}

	printf("ARBITER_WORKER_THREAD_CLOSED!\n");
	return 0;
}

DWORD WINAPI arbiter_server_accept_func(void* argv)
{
	SOCKET listenSock = *(SOCKET*)argv;

	WSANETWORKEVENTS WSAEvents;

	while (WAIT_OBJECT_0 != WaitForSingleObject(a_server.shutdownEvent, 0))
	{
		if (WSA_WAIT_TIMEOUT != WSAWaitForMultipleEvents(1, &a_server.acceptEvent, false, 200, false))
		{
			WSAEnumNetworkEvents(listenSock, a_server.acceptEvent, &WSAEvents);
			if ((WSAEvents.lNetworkEvents & FD_ACCEPT) && (0 == WSAEvents.iErrorCode[FD_ACCEPT_BIT]))
			{
				sockaddr_in client_address;
				int addressSize = sizeof(client_address);

				SOCKET client_socket = accept(listenSock, (sockaddr*)&client_address, &addressSize);
				if (client_socket != INVALID_SOCKET)
					arbiter_server_connexion_add(client_socket, client_address);
			}
		}
	}
	return 0;
}

void WINAPI arbiter_server_process_job(void* j, e_job_type type, sql::Connection * con)
{
	switch (type)
	{
	case J_A_BROADCAST:
	{
		//Stream * data = (Stream *)j->argv[0];
		//broadcast_clients * clients = (broadcast_clients*)j->argv[1];
		//for (uint32 i = 0; i < clients->size(); i++)
		//{
		//	if ((*clients)[i] >= 0 && (*clients)[i] < a_server.clientsCount)
		//		connection_send(a_server.clients[(*clients)[i]], data);
		//}
	}break;

	case J_A_BROADCAST_PLAYER_MOVE:
	{
		j_b_move * j_ = (j_b_move*)j;

		//Stream data = Stream();
		//data.Resize(47);
		//data.WriteInt16(47);
		//data.WriteInt16(S_USER_LOCATION);
		//data.WriteWorldId(j_->p_);
		//data.WriteFloat(j_->p_init[0]);
		//data.WriteFloat(j_->p_init[1]);
		//data.WriteFloat(j_->p_init[2]);
		//data.WriteInt32(j_->p_->position.heading.load());
		//data.WriteInt16(j_->p_->stats.get_movement_speed(j_->p_->status)); //todo
		//data.WriteFloat(j_->p_->position.x.load());
		//data.WriteFloat(j_->p_->position.y.load());
		//data.WriteFloat(j_->p_->position.z.load());
		//data.WriteInt32(j_->t_);
		//data.WriteUInt8(1);



		j_->p_->spawn.filter(j_->p_m);
		//j_->p_->spawn.bordacast(&data);

	}break;

	case J_A_BROADCAST_PLAYER_SPAWN:
	{
		j_b_spawn* j_ = (j_b_spawn*)j;

		Stream data_y, data_m;
		player_write_spawn_packet(j_->w_p_, data_m);

		for (size_t i = 0; i < j_->p_l.size(); i++)
		{
			data_y.Clear();
			player_write_spawn_packet(j_->p_l[i], data_y);

			connection_send(j_->p_l[i]->con, &data_m);
			connection_send(j_->w_p_->con, &data_y);
		}

		for (size_t i = 0; i < j_->p_l.size(); i++)
		{
			j_->w_p_->spawn.add_erase(j_->p_l[i]);
			j_->p_l[i]->spawn.add_erase(j_->w_p_);
		}

	}break;

	case J_A_BROADCAST_PLAYER_DESPAWN_ME:
	{
		//despawn me from you
		j_b_despawn_me * j_ = (j_b_despawn_me*)j;
		j_->p_->spawn.clear();
	}break;

	case J_A_BROADCAST_PLAYER_DESPAWN:
	{
		//despawn me form you and you from me
		j_b_despawn * j_ = (j_b_despawn*)j;
		std::shared_ptr<player> me_p = std::move(j_->w_p_);
		Stream data_y, data_m;
		data_m.Resize(16);
		data_m.WriteInt16(16);
		data_m.WriteInt16(S_DESPAWN_USER);
		data_m.WriteWorldId(j_->w_p_);

		data_y.Resize(16);
		data_y.WriteInt16(16);
		data_y.WriteInt16(S_DESPAWN_USER);

		for (uint32 i = 0; i < j_->p_l.size(); i++)
		{
			data_y._pos = 4;
			data_y.WriteWorldId(j_->p_l[i]);

			connection_send(j_->w_p_->con, &data_y);
			connection_send(j_->p_l[i]->con, &data_m);

			j_->p_l[i]->spawn.add_erase(j_->w_p_, false);
			j_->w_p_->spawn.add_erase(j_->p_l[i], false);
		}
	}break;

	case J_A_PLAYER_ENTER_WORLD_FIN:
	{
		io_load_topo_fin((j_enter_world_fin*)j);
	}break;

	default:
		break;
	}

	return;
}

uint64 WINAPI arbiter_get_connection_count()
{
	return a_server.clientsCount.load(std::memory_order_relaxed);
}
