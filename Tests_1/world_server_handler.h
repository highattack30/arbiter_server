#ifndef WORLD_SERVER_HANDLER_H
#define WORLD_SERVER_HANDLER_H

#include "serverEnums.h"
#include "typeDefs.h"
#include "win32.h"
#include "Stream.h"
#include "world_server_opcodes.h"
#include "internalOp.h"
#include <string>

struct buffer {
	buffer();
	buffer(uint8, uint32, uint8 r = 0);
	OVERLAPPED					ov;
	Stream						data;
	WSABUF						wsa_buff;
	uint32						parent;
	uint8						type;
	uint8						recv_state;
};

struct w_server_node {
	buffer						data;
	uint32						id;
	SOCKET						socket;
	std::string					name;
	uint32						channel_id;
	uint32						continent_id;
	uint32						area_id;
	bool						active;
	e_world_node_opcode			opcode;
};


struct w_s_working_thread {
	HANDLE						thread;
	uint32						id;
	bool						active;
	uint32						bytes_transfered;
	uint64						key;
	buffer*						current_buffer;
};

struct w_server_handler {
	w_server_node				nodes[SC_WORLD_HANDLER_MAX_NODES];
	std::atomic_bool			run;

	HANDLE						iocp;
	SOCKET						socket;
	sockaddr_in					socket_data;
	HANDLE						accept_thread;
	WSAEVENT					accept_event;
	std::atomic_bool			accept_run;
	w_s_working_thread			threads[SC_WORLD_WORKING_THREADS_MAX];
} static ws_handler;

static w_server_node nodes[SC_WORLD_HANDLER_MAX_NODES];

bool WINAPI w_server_handler_init();
void WINAPI w_server_handler_release();

static void WINAPI w_server_close();




bool WINAPI w_server_get_visible_list_async(std::shared_ptr<connection>, e_io_opcodes, Stream& data);

static DWORD WINAPI w_server_worker(LPVOID);
static DWORD WINAPI w_server_accept(LPVOID);
void WINAPI w_server_accept_node(SOCKET);
uint8 WINAPI w_server_process_node_init_data(uint32, uint32, uint32);

static void WINAPI w_server_recv_async(uint8, uint32);
void WINAPI w_server_send_async(uint32, Stream*);
void WINAPI w_server_send(uint32 node_id, Stream *data);
bool WINAPI w_server_process_recv(w_s_working_thread *t, buffer* buffer);



w_server_node* WINAPI w_server_get_node(uint32);
w_server_node* WINAPI w_server_get_node(uint32 continent, uint32 area, uint32 channel);

bool WINAPI w_server_remove_player(std::shared_ptr<connection>);
#endif