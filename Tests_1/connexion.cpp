#include "connexion.h"
#include "serverEnums.h"
#include "OpCodes.h"
#include "job.h"
#include "arbiter_server.h"
#include "Stream.h"
#include "account.h"
#include "entity_manager.h"

#include <string>
#include <stdio.h>
#include <random>
#include <iomanip>
#include <sstream>


//init
connection::connection(SOCKET sock, uint32 id)
{
	_testId = 0;
	_selected_player = 0;
	_session = std::make_unique<Session>();

	_session->fill_key(NULL, 2); //get 2 random keys [skey1,skey2]

	InitializeCriticalSection(&_recvLock);
	InitializeCriticalSection(&_sendLock);

	memset(&_account, 0, sizeof account);

	for (uint8 i = 0; i < SC_PLAYER_MAX_CHARACTER_COUNT; i++)
		_players[i] = nullptr;

	_inLobby = true;
	_socket = sock;
	_id = id;

	//_recvBuffer = std::make_shared<RECV_BUFFER>();
	_recvBuffer.data = Stream();
	_recvBuffer.data.Resize(128);
	_recvBuffer.recvStatus = CP_KEY_1;
	_recvBuffer.flags = _recvBuffer.transferBytes = 0;
	_recvBuffer.connexion_id = id;
	_peddingPacketCount = 0;


}

connection::~connection()
{
	for (uint8 i = 0; i < SC_PLAYER_MAX_CHARACTER_COUNT; i++)
	{
		if (_players[i])
		{
			entity_manager::destroy_player(_players[i]->eid);
			_players[i] = nullptr;
		}
	}


	DeleteCriticalSection(&_sendLock);
	DeleteCriticalSection(&_recvLock);
}

bool WINAPI connection_init(std::shared_ptr<connection> c, HANDLE master_iocp)
{
	c->_iocp = CreateIoCompletionPort((HANDLE)c->_socket, master_iocp, NULL, 0);
	if (NULL == c->_iocp)
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
	shutdown(c->_socket, SD_SEND);
	closesocket(c->_socket);
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
	if (!send(c->_socket, initData, 4, 0))
		return false;
	return true;
}


//crypt
bool WINAPI connexion_crypt_init(std::shared_ptr<connection> c)
{
	return c->_session->init_session();
}

bool WINAPI connexion_fill_key(std::shared_ptr<connection> c, uint8 keyNo)
{
	return c->_session->fill_key(c->_recvBuffer.data._raw, keyNo);
}



//recv
bool WINAPI connection_recv(std::shared_ptr<connection> c)
{
	c->_recvBuffer.flags = 0;
	c->_recvBuffer.wsaBuff.buf = (char*)&c->_recvBuffer.data._raw[c->_recvBuffer.data._pos];
	c->_recvBuffer.wsaBuff.len = (ULONG)c->_recvBuffer.data._size - c->_recvBuffer.data._pos;

	job* newJob = new job(new j_recv(c, c->_recvBuffer.recvStatus), J_A_RECV);
	int32 ret = WSARecv(c->_socket, &c->_recvBuffer.wsaBuff, 1, &c->_recvBuffer.transferBytes, &c->_recvBuffer.flags, (LPWSAOVERLAPPED)newJob, NULL);
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
	if (c->_recvBuffer.recvStatus == CP_TERA_PACKET_HEAD)
	{
		c->_session->Decrypt(c->_recvBuffer.data._raw, c->_recvBuffer.data._size);
		uint16 size = (uint16)((c->_recvBuffer.data._raw[1] << 8) | c->_recvBuffer.data._raw[0]);
		uint16 opcode = (uint16)((c->_recvBuffer.data._raw[3] << 8) | c->_recvBuffer.data._raw[2]);
		if (size == 4)
		{
			op_function op_fn = opcode_resolve((e_opcode)opcode);
			if (op_fn)
			{
				if (!op_fn(c, argv))
					return false;
				c->_recvBuffer.data.Clear();
				c->_recvBuffer.data.Resize(4);
				return	connection_recv(c);
			}
			else
				printf("OPCODE [%04x] NULL\n", opcode);

			return false;
		}

		c->_recvBuffer.data.Clear();
		c->_recvBuffer.data.Resize(size - 4);
		c->_recvBuffer.recvStatus = CP_TERA_PACKET_BODY;
		c->_recvBuffer.opcode = (e_opcode)opcode;
		return	connection_recv(c);
	}
	else if (c->_recvBuffer.recvStatus == CP_TERA_PACKET_BODY)
	{
		c->_recvBuffer.data._pos += noOfBytesTrasfered;
		if (c->_recvBuffer.data._pos < c->_recvBuffer.data._size)
			return connection_recv(c);

		c->_recvBuffer.data._pos = 0;
		c->_session->Decrypt(c->_recvBuffer.data._raw, c->_recvBuffer.data._size);

		op_function op_fn = opcode_resolve(c->_recvBuffer.opcode);
		if (op_fn)
			op_fn(c, argv);
		else
			printf("OPCODE [%04x] NULL\n", c->_recvBuffer.opcode);

		c->_recvBuffer.opcode = OPCODE_MAX;
		c->_recvBuffer.recvStatus = CP_TERA_PACKET_HEAD;
		c->_recvBuffer.data.Clear();
		c->_recvBuffer.data.Resize(4);
		return	connection_recv(c);
	}
	return false;
}

bool WINAPI connection_send(std::shared_ptr<connection> c)
{
	std::shared_ptr<SEND_BUFFER> s_b = std::make_shared<SEND_BUFFER>();
	s_b->data.Write(c->_recvBuffer.data._raw, c->_recvBuffer.data._size);
	s_b->wsaBuff.buf = (char*)s_b->data._raw;
	s_b->wsaBuff.len = (ULONG)s_b->data._size;
	s_b->flags = 0;

	EnterCriticalSection(&c->_sendLock);
	c->_session->Encrypt(s_b->data._raw, s_b->data._size);
	LeaveCriticalSection(&c->_sendLock);

	job * newJob = new job(new j_send(s_b), J_A_SEND);
	uint32 ret = WSASend(c->_socket, &s_b->wsaBuff, 1, &s_b->transferBytes, s_b->flags, (LPWSAOVERLAPPED)newJob, NULL);	if (ret == SOCKET_ERROR)
	{
		ret = WSAGetLastError();
		if (ret != WSA_IO_PENDING)
		{
			c->_droppedPacketCount++;

			printf("SEND PACKET DROPED droppedCount[%d] client[%d] wsa-error[%d]\n", c->_droppedPacketCount, c->_id, ret);
			delete newJob;

			if (c->_droppedPacketCount >= CS_MAX_DROP_PACKET_COUNT)
				arbiter_server_connexion_remove(c->_id);

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

	EnterCriticalSection(&c->_sendLock);
	c->_session->Encrypt(s_b->data._raw, s_b->data._size);
	LeaveCriticalSection(&c->_sendLock);

	job * newJob = new job(new j_send(s_b), J_A_SEND);
	uint32 ret = WSASend(c->_socket, &s_b->wsaBuff, 1, &s_b->transferBytes, s_b->flags, (LPWSAOVERLAPPED)newJob, NULL);	if (ret == SOCKET_ERROR)
	{
		ret = WSAGetLastError();
		if (ret != WSA_IO_PENDING)
		{
			c->_droppedPacketCount++;

			printf("SEND[DATA] PACKET DROPED droppedCount[%d] client[%d] wsa-error[%d]\n", c->_droppedPacketCount, c->_id, ret);
			delete newJob;

			if (c->_droppedPacketCount >= CS_MAX_DROP_PACKET_COUNT)
				arbiter_server_connexion_remove(c->_id);

			return false;
		}
	}

	return true;
}

bool WINAPI connection_send(std::shared_ptr<connection> c, BYTE * data, uint32 size)
{
	return send(c->_socket, (const char*)data, size, 0) == 0 ? false : true;
}

int8 WINAPI connection_can_create_player(std::shared_ptr<connection> c)
{

	for (uint8 i = 0; i < SC_PLAYER_MAX_CHARACTER_COUNT; i++)
	{
		if (!c->_players[i])
			return i;
	}

	return -1;
}

bool WINAPI connection_select_player(std::shared_ptr<connection> c, uint32 dbid)
{
	for (int32 i = 0; i < SC_PLAYER_MAX_CHARACTER_COUNT; i++)
	{
		if (c->_players[i] && c->_players[i]->dbid == dbid)
		{
			c->_selected_player = i;
			return true;
		}
	}
	return false;
}

