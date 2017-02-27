#include "connexion.h"
#include "serverEnums.h"
#include "OpCodes.h"
#include "job.h"
#include "arbiter_server.h"
#include "Stream.h"
#include "account.h"
#include "entity_manager.h"
#include "world_server_opcodes.h"
#include <string>
#include <stdio.h>
#include <random>
#include <iomanip>
#include <sstream>


//init
connection::connection(SOCKET sock, uint32 id_) {
	if (id_ == 0) return;
	selected_player = 0;
	session = std::make_unique<Session>();

	session->fill_key(NULL, 2); //get 2 random keys [skey1,skey2]

	InitializeCriticalSection(&recvLock);
	InitializeCriticalSection(&sendLock);

	memset(&account, 0, sizeof account);

	for (uint8 i = 0; i < SC_PLAYER_MAX_CHARACTER_COUNT; i++)
		players[i] = nullptr;

	inLobby = true;
	socket = sock;
	id = id_;

	//recvBuffer = std::make_shared<RECV_BUFFER>();
	recvBuffer.data = Stream();
	recvBuffer.data.Resize(128);
	recvBuffer.recvStatus = CP_KEY_1;
	recvBuffer.flags = recvBuffer.transferBytes = 0;
	recvBuffer.connexion_id = id;
	peddingPacketCount = 0;

	p_s.Resize(40);
}

connection::~connection()
{
	for (uint8 i = 0; i < SC_PLAYER_MAX_CHARACTER_COUNT; i++)
	{
		if (players[i])
		{
			entity_manager::destroy_player(players[i]->eid);
			players[i] = nullptr;
		}
	}


	DeleteCriticalSection(&sendLock);
	DeleteCriticalSection(&recvLock);
}

bool WINAPI connection_init(std::shared_ptr<connection> c, HANDLE master_iocp)
{
	c->iocp = CreateIoCompletionPort((HANDLE)c->socket, master_iocp, NULL, 0);
	if (NULL == c->iocp)
		return false;

	if (!connection_send_init(c))
	{
		connection_close(c);
		return false;
	}

	if (!connection_recv(c))
	{
		connection_close(c);
		return false;
	}



	return true;
}


bool WINAPI connection_close(std::shared_ptr<connection> c)
{
	shutdown(c->socket, SD_SEND);
	closesocket(c->socket);
	return true;
}

std::shared_ptr<connection> WINAPI connection_create(SOCKET sock, HANDLE masterIOCP, uint32 id)
{
	std::shared_ptr<connection> out = std::make_shared<connection>(sock, id);
	if (!connection_init(out, masterIOCP))
	{
#ifdef _DEBUG
		printf("SERVER FAILED TO CREATE CONNEXION\n");
#endif
		return nullptr;
	}
	return out;
}

bool WINAPI connection_send_init(std::shared_ptr<connection> c)
{
	if (!c) return false;
	char initData[4] = { 1,0,0,0 };
	if (!send(c->socket, initData, 4, 0))
		return false;
	return true;
}


//crypt
bool WINAPI connexion_crypt_init(std::shared_ptr<connection> c)
{
	return c->session->init_session();
}

bool WINAPI connexion_fill_key(std::shared_ptr<connection> c, uint8 keyNo)
{
	return c->session->fill_key(c->recvBuffer.data._raw, keyNo);
}



//recv
bool WINAPI connection_recv(std::shared_ptr<connection> c)
{
	c->recvBuffer.flags = 0;
	c->recvBuffer.wsaBuff.buf = (char*)&c->recvBuffer.data._raw[c->recvBuffer.data._pos];
	c->recvBuffer.wsaBuff.len = (ULONG)c->recvBuffer.data._size - c->recvBuffer.data._pos;

	job* newJob = new job(new j_recv(c, c->recvBuffer.recvStatus), J_A_RECV);
	int32 ret = WSARecv(c->socket, &c->recvBuffer.wsaBuff, 1, &c->recvBuffer.transferBytes, &c->recvBuffer.flags, (LPWSAOVERLAPPED)newJob, NULL);
	if (ret == SOCKET_ERROR)
	{
		ret = WSAGetLastError();
		if (ret != WSA_IO_PENDING)
		{
			delete newJob;
			printf("RECV ERROR RET[%d] WSA-ERROR-CODE:[%d]\n", ret, WSAGetLastError());
			return false;
		}

	}

	return true;
}

bool WINAPI connection_process_recv_packet(std::shared_ptr<connection> c, void * argv[], uint32 noOfBytesTrasfered)
{
	if (c->recvBuffer.recvStatus == CP_TERA_PACKET_HEAD)
	{
		c->session->Decrypt(c->recvBuffer.data._raw, c->recvBuffer.data._size);
		uint16 size = c->recvBuffer.data.ReadUInt16();
		uint16 opcode = c->recvBuffer.data.ReadUInt16();
		if (size == 4)
		{
			op_function op_fn = opcode_resolve((e_opcode)opcode);
			if (op_fn)
			{
				if (!op_fn(c, argv))
					return false;
				c->recvBuffer.data.Clear();
				c->recvBuffer.data.Resize(4);
				return	connection_recv(c);
			}
			else
				printf("OPCODE [%04x] NULL\n", opcode);

			return false;
		}

		c->recvBuffer.data.Clear();
		c->recvBuffer.data.Resize(size - 4);
		c->recvBuffer.recvStatus = CP_TERA_PACKET_BODY;
		c->recvBuffer.opcode = (e_opcode)opcode;
		return	connection_recv(c);
	}
	else if (c->recvBuffer.recvStatus == CP_TERA_PACKET_BODY)
	{
		c->recvBuffer.data._pos += noOfBytesTrasfered;
		if (c->recvBuffer.data._pos < c->recvBuffer.data._size)
			return connection_recv(c);

		c->recvBuffer.data._pos = 0;
		c->session->Decrypt(c->recvBuffer.data._raw, c->recvBuffer.data._size);

		op_function op_fn = opcode_resolve(c->recvBuffer.opcode);
		if (op_fn)
			op_fn(c, argv);
		else
			printf("OPCODE [%04x] NULL\n", c->recvBuffer.opcode);

		c->recvBuffer.opcode = OPCODE_MAX;
		c->recvBuffer.recvStatus = CP_TERA_PACKET_HEAD;
		c->recvBuffer.data.Clear();
		c->recvBuffer.data.Resize(4);
		return	connection_recv(c);
	}
	return false;
}

bool WINAPI connection_send(std::shared_ptr<connection> c)
{
	std::shared_ptr<SEND_BUFFER> s_b = std::make_shared<SEND_BUFFER>();
	s_b->data.Write(c->recvBuffer.data._raw, c->recvBuffer.data._size);
	s_b->wsaBuff.buf = (char*)s_b->data._raw;
	s_b->wsaBuff.len = (ULONG)s_b->data._size;
	s_b->flags = 0;

	EnterCriticalSection(&c->sendLock);
	c->session->Encrypt(s_b->data._raw, s_b->data._size);
	LeaveCriticalSection(&c->sendLock);

	job * newJob = new job(new j_send(s_b), J_A_SEND);
	uint32 ret = WSASend(c->socket, &s_b->wsaBuff, 1, &s_b->transferBytes, s_b->flags, (LPWSAOVERLAPPED)newJob, NULL);	if (ret == SOCKET_ERROR)
	{
		ret = WSAGetLastError();
		if (ret != WSA_IO_PENDING)
		{
			c->droppedPacketCount++;

			printf("SEND PACKET DROPED droppedCount[%d] client[%d] wsa-error[%d]\n", c->droppedPacketCount, c->id, ret);
			delete newJob;

			if (c->droppedPacketCount >= CS_MAX_DROP_PACKET_COUNT)
				arbiter_server_connexion_remove(c->id);

			return false;
		}
	}

	return true;
}
//send
bool WINAPI connection_send(std::shared_ptr<connection> c, Stream * data)
{
	if (!c) return false;
	std::shared_ptr<SEND_BUFFER> s_b = std::make_shared<SEND_BUFFER>();
	s_b->data.Write(data->_raw, data->_size);
	s_b->wsaBuff.buf = (char*)s_b->data._raw;
	s_b->wsaBuff.len = (ULONG)s_b->data._size;
	s_b->flags = 0;

	EnterCriticalSection(&c->sendLock);
	c->session->Encrypt(s_b->data._raw, s_b->data._size);
	LeaveCriticalSection(&c->sendLock);

	std::unique_ptr<job> newJob = std::make_unique<job>(new j_send(s_b), J_A_SEND);
	uint32 ret = WSASend(c->socket, &s_b->wsaBuff, 1, &s_b->transferBytes, s_b->flags, (LPWSAOVERLAPPED)newJob.get(), NULL);
	if (ret == SOCKET_ERROR && WSA_IO_PENDING != WSAGetLastError()) {
		c->droppedPacketCount++;
		printf("SEND[DATA] PACKET DROPED droppedCount[%d] client[%d] wsa-error[%d]\n", c->droppedPacketCount, c->id, ret);
		if (c->droppedPacketCount >= CS_MAX_DROP_PACKET_COUNT)
			arbiter_server_connexion_remove(c->id);
		return false;
	}
	newJob.release();

	return true;
}

bool WINAPI connection_send(std::shared_ptr<connection> c, BYTE * data, uint32 size)
{
	return send(c->socket, (const char*)data, size, 0) == 0 ? false : true;
}

int8 WINAPI connection_can_create_player(std::shared_ptr<connection> c)
{

	for (uint8 i = 0; i < SC_PLAYER_MAX_CHARACTER_COUNT; i++)
	{
		if (!c->players[i])
			return i;
	}

	return -1;
}

bool WINAPI connection_select_player(std::shared_ptr<connection> c, uint32 dbid)
{
	for (int32 i = 0; i < SC_PLAYER_MAX_CHARACTER_COUNT; i++)
	{
		if (c->players[i] && c->players[i]->dbid == dbid)
		{
			c->selected_player = i;
			return true;
		}
	}
	return false;
}

