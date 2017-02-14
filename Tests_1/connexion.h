#ifndef CONNEXION_H
#define CONNEXION_H

#include <iostream>
#include <memory>

#include "win32.h"
#include "typeDefs.h"
#include "serverEnums.h"
#include "connexionEnums.h"
#include "opcodeEnum.h"
#include "Crypt.hpp"
#include "player.h"
#include "DBHandler.h"
#include "Stream.h"
#include "account.h"

using namespace Crypt;
class connection; struct job; 

#ifndef OP_FUNCTION
typedef bool(WINAPI *op_function)(std::shared_ptr<connection>, void* argv[]);
#endif

struct SEND_BUFFER
{
	WSABUF		wsaBuff;
	DWORD		transferBytes;
	DWORD		flags;
	DWORD		connexion_id;

	Stream		data;
};

struct RECV_BUFFER
{
	WSABUF		wsaBuff;
	DWORD		transferBytes;
	DWORD		flags;
	e_opcode    opcode;
	Stream		data;

	e_connexion_recv_buffer_state recvStatus;
	uint32 connexion_id;
};

class connection
{
public:
	connection(SOCKET sock, uint32 id);
	~connection();

	std::shared_ptr<player> _players[SC_PLAYER_MAX_CHARACTER_COUNT];

	RECV_BUFFER _recvBuffer;

	CRITICAL_SECTION _recvLock;
	CRITICAL_SECTION _sendLock;
	SOCKET _socket;
	HANDLE _iocp;

	bool 
		_inLobby;

	uint32
		_selected_player,
		_id,
		_testId;

	std::atomic<uint16> _peddingPacketCount;
	uint8 _droppedPacketCount;


	
	account  _account;
	std::unique_ptr<Session>  _session;
};

bool WINAPI connection_init(std::shared_ptr<connection> c, HANDLE master_iocp);

bool WINAPI connection_close(std::shared_ptr<connection> c);
std::shared_ptr<connection>  WINAPI connection_create(SOCKET s, HANDLE masterIOCP, uint32 id);
bool WINAPI connection_send_init(std::shared_ptr<connection> c);

bool WINAPI connexion_fill_key(std::shared_ptr<connection> c, uint8 keyNo);
bool WINAPI connexion_crypt_init(std::shared_ptr<connection> c);

bool WINAPI connection_recv(std::shared_ptr<connection> c);
bool WINAPI connection_process_recv_packet(std::shared_ptr<connection> c, void * argv[], uint32 noOfBytesTrasfered);

bool WINAPI connection_send(std::shared_ptr<connection> c);
bool WINAPI connection_send(std::shared_ptr<connection> c, Stream * data);
bool WINAPI connection_send(std::shared_ptr<connection> c, BYTE * data, uint32 size);


int8 WINAPI connection_can_create_player(std::shared_ptr<connection> c);
bool WINAPI connection_select_player(std::shared_ptr<connection> c, uint32 dbid);

#endif