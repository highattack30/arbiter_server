#include "arbiter_server.h"
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
#include "world_server_handler.h"

bool WINAPI arbiter_server_init() {

	InitializeCriticalSection(&a_server.log_sync);
	a_server.clients[0] = std::make_shared<connection>(0, 0);
	memset(a_server.clients[0].get(), 0, sizeof(connection));

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
	a_server.listenerData.sin_port = htons(config::net.port);

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

bool WINAPI arbiter_server_connexion_remove(uint32 id) {
	if (id >= SBS_SERVER_MAX_CLIENTS)
		return false;
	std::shared_ptr<connection> c = a_server.clients[id];
	if (c) {
		if (!c->inLobby)
		{

			//active_remove_player(c->_players[c->_selected_player]);

			return w_server_remove_player(c);
		}

		{
			std::lock_guard<std::mutex> locker(a_server.clientListMutex);
			a_server.clients[id] = nullptr;
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

HANDLE WINAPI arbiter_get_iocp() {
	return a_server.server_iocp;
}

bool WINAPI arbiter_server_process_job_async(void* j, e_job_type type)
{
	if (!j) return false;
	std::unique_ptr<job>newJob = std::make_unique<job>(j, type);

	BOOL result = PostQueuedCompletionStatus(a_server.server_iocp, 1, NULL, (LPOVERLAPPED)newJob.get());
	if (!result)
	{
		a_server.no_of_droped_jobs++;

#ifdef _DEBUG
		printf("ARBITER DROPPED JOB! TYPE[%d]  TOTAL_DROPPED_COUNT[%d]\n", type, a_server.no_of_droped_jobs);
#endif
	}
	else
		newJob.release();

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

	while (1)
	{
		BOOL retVal = GetQueuedCompletionStatusEx(a_server.server_iocp, (LPOVERLAPPED_ENTRY)this_->j_c,
			config::server.arbiter_job_pull_cout, &this_->out_jobs, INFINITE, false);

		if (WAIT_OBJECT_0 == WaitForSingleObject(a_server.shutdownEvent, 0) || ERROR_ABANDONED_WAIT_0 == WSAGetLastError())
			break;

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
									result = j_->con->session->send_server_key(j_->con->socket, 1);

								if (result)
								{
									j_->con->recvBuffer.recvStatus = CP_KEY_2;
									result = connection_recv(j_->con);
								}
							}break;

							case CP_KEY_2:
							{

								result = connexion_fill_key(j_->con, 3);
								if (result)
									result = j_->con->session->send_server_key(j_->con->socket, 2);
								if (result)
									result = connexion_crypt_init(j_->con);
								if (result)
								{
									j_->con->recvBuffer.data.Clear();
									j_->con->recvBuffer.data.Resize(4);
									j_->con->recvBuffer.recvStatus = CP_TERA_PACKET_HEAD;
									result = connection_recv(j_->con);
								}
							}
							}break;

							if (!result)
							{
								printf("CONNEXION LOST:[%d]\n", j_->con->id);
								arbiter_server_connexion_remove(j_->con->id);
							}
						}
						else
						{
							printf("CONNEXION LOST:[%d]\n", j_->con->id);
							arbiter_server_connexion_remove(j_->con->id);
						}
					}
					else
					{
						printf("RECV INTERNAL ERROR\n");
					}

				}break;

				case J_A_SEND: {
					//just delete the job ?
				}break;

					//case J_W_RECV_FROM_WORLD_SERVER_NODE: {
					//	w_server_process_recv((j_recv_from_node*)j);
					//}break;

					//case J_W_SEND_TO_WORLD_SERVER_NODE: {
					//	//just delete the job ?
					//}break;

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

	default:
		break;
	}

	return;
}

std::shared_ptr<player> WINAPI arbiter_get_player(uint32 id) {
	if (id < SBS_SERVER_MAX_CLIENTS)
		return a_server.clients[id]->players[a_server.clients[id]->selected_player];
	return nullptr;
}

std::shared_ptr<connection> WINAPI arbiter_get_connection(uint32 id) {
	if (id < SBS_SERVER_MAX_CLIENTS)
		return a_server.clients[id];

	return nullptr;
}

void WINAPI arbiter_send(uint32 id, Stream * data) {
	if (id < SBS_SERVER_MAX_CLIENTS)
		connection_send(a_server.clients[id], data);
	return;
}

uint64 WINAPI arbiter_get_connection_count()
{
	return a_server.clientsCount.load(std::memory_order_relaxed);
}
