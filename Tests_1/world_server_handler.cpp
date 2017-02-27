#include "world_server_handler.h"
#include "config.h"
#include "arbiter_server.h"
#include "job.h"
#include "w_node_opcodes.h"
#include "connexion.h"

bool WINAPI w_server_handler_init() {
	for (uint32 i = 0; i < SC_WORLD_HANDLER_MAX_NODES; i++) {
		nodes[i].id = i;
		nodes[i].active = false;
	}

	ws_handler.iocp = CreateIoCompletionPort((HANDLE)-1, NULL, 0, 0);
	if (!ws_handler.iocp) {
		printf("NULL == CreateIoCompletionPort\n");
		return false;
	}

	ws_handler.socket_data.sin_addr.S_un.S_addr = config::server.world_server_listening_ip == "0.0.0.0" ? INADDR_ANY : inet_addr(config::server.world_server_listening_ip);
	ws_handler.socket_data.sin_family = AF_INET;
	ws_handler.socket_data.sin_port = htons(config::server.world_server_listening_port);

	if (
		((ws_handler.socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) ||
		(SOCKET_ERROR == (bind(ws_handler.socket, (const sockaddr *)&ws_handler.socket_data, sizeof(ws_handler.socket_data)))) ||
		(SOCKET_ERROR == listen(ws_handler.socket, 0))
		)
	{
		printf("WSASocket || bind failed %d\n", WSAGetLastError());
		return false;
	}

	if (NULL == (ws_handler.accept_event = WSACreateEvent()))
		return false;

	if (SOCKET_ERROR == WSAEventSelect(ws_handler.socket, ws_handler.accept_event, FD_ACCEPT)) {
		return false;
	}


	a_store(ws_handler.accept_run, true);
	ws_handler.accept_thread = CreateThread(NULL, 0, w_server_accept, NULL, 0, 0);
	if (!ws_handler.accept_thread) return false;

	a_store(ws_handler.run, true);
	for (uint32 i = 0; i < SC_WORLD_WORKING_THREADS; i++) {
		ws_handler.threads[i].id = i;
		ws_handler.threads[i].current_buffer = nullptr;
		ws_handler.threads[i].thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)w_server_worker, (LPVOID)&ws_handler.threads[0], 0, 0);
	}

	printf("STARTED [%d] WORLD_HANDLER_THREADS!!\n", SC_WORLD_WORKING_THREADS);
	return true;
}

void WINAPI w_server_handler_release() {
	a_store(ws_handler.run, false);

	Stream data;
	data.Resize(4);
	data.WriteUInt16(4);
	data.WriteUInt16(S_W_CLOSE);
	for (uint32 i = 0; i < SC_WORLD_HANDLER_MAX_NODES; i++) {
		if (nodes[i].active) {
			w_server_send(i, &data);
		}
	}

	w_server_close();
	CloseHandle(ws_handler.iocp);
	PostQueuedCompletionStatus(ws_handler.iocp, 0, NULL, NULL);
	for (uint32 i = 0; i < SC_WORLD_WORKING_THREADS_MAX; i++) {
		if (ws_handler.threads[i].active) {
			WaitForSingleObject(ws_handler.threads, INFINITE);
		}
	}
	return;
}

void WINAPI w_server_close() {
	shutdown(ws_handler.socket, 2);
	closesocket(ws_handler.socket);
	return;
}

void WINAPI w_server_recv(uint8 state, uint32 node_id) {
	if (!nodes[node_id].active) return;

	nodes[node_id].data.recv_state = state;
	nodes[node_id].data.wsa_buff.buf = (char*)&nodes[node_id].data.data._raw[nodes[node_id].data.data._pos];
	nodes[node_id].data.wsa_buff.len = nodes[node_id].data.data._size - nodes[node_id].data.data._pos;
	DWORD flags = 0;
	int32 result = WSARecv(nodes[node_id].socket, &nodes[node_id].data.wsa_buff, 1, NULL, &flags, (LPOVERLAPPED)&nodes[node_id].data, NULL);
	if (result == SOCKET_ERROR && !(WSA_IO_PENDING == WSAGetLastError())) {
		int res = WSAGetLastError();
		printf("ERROR ON RECEIVING wsa[%d]\n", res);

		if (res == 10054) {
			nodes[node_id].active = false;
			nodes[node_id].area_id = nodes[node_id].channel_id = nodes[node_id].continent_id = 0;
			nodes[node_id].data.data.Clear();
			closesocket(nodes[node_id].socket);
			printf("WORLD_NODE_LOST!\n");
		}
		return;
	}

	//printf("POSTED RECV RESULT[%d] WSA[%d]\n", result, WSAGetLastError());
	return;
}

void WINAPI w_server_send_async(uint32 node_id, Stream * data) {
	std::unique_ptr<buffer> j_s = std::make_unique<buffer>();
	j_s->parent = node_id;
	j_s->type = 1;
	ZeroMemory(&j_s->ov, sizeof(OVERLAPPED));

	j_s->data.Write(data->_raw, data->_size);
	j_s->wsa_buff.buf = (char*)j_s->data._raw;
	j_s->wsa_buff.len = (uint32)j_s->data._size;
	int32 result = WSASend(nodes[node_id].socket, &j_s->wsa_buff, 1, NULL, 0, (LPWSAOVERLAPPED)j_s.get(), NULL);
	if (result == SOCKET_ERROR && (WSA_IO_PENDING != WSAGetLastError())) {
		printf("ERROR ON SENDING ASYNC WSA-ERROR[%d]\n", WSAGetLastError());
		return;
	}
	j_s.release();
	return;
}

void WINAPI w_server_send(uint32 node_id, Stream *data) {
	WSABUF wsa_buf;
	wsa_buf.buf = (char*)data->_raw;
	wsa_buf.len = data->_size;
	DWORD byte_sent = (DWORD)data->_size;
	int32 result = WSASend(nodes[node_id].socket, &wsa_buf, 1, &byte_sent, 0, NULL, NULL);
	if (result == SOCKET_ERROR && !(WSA_IO_PENDING == WSAGetLastError())) {
		printf("ERROR ON SENDING NON ASYNC WSA-ERROR[%d]\n", WSAGetLastError());
		return;
	}

	return;
}

bool WINAPI w_server_get_visible_list_async(std::shared_ptr<connection> c, e_io_opcodes op, Stream &data_) {
	if (c->node_id < SC_WORLD_HANDLER_MAX_NODES && nodes[c->node_id].active) {
		Stream data;
		data.Resize(10 + data_._size);
		data.WriteUInt16(10 + data_._size);
		data.WriteUInt16(S_W_GET_VISIBLE_PLAYERS);
		data.WriteUInt32(c->id);
		data.WriteUInt16(op);

		if (data_._size)
			data.Write(data_._raw, data_._size);

		w_server_send(c->node_id, &data);
		return true;
	}
	return false;
}



DWORD WINAPI w_server_worker(LPVOID arg) {
	w_s_working_thread * this_ = (w_s_working_thread *)arg;
	this_->active = true;
	OVERLAPPED * ov = NULL;

	BOOL result; uint16 packet_size;
	while (1) {
		result = GetQueuedCompletionStatus(ws_handler.iocp, (LPDWORD)&this_->bytes_transfered, (PULONG_PTR)&this_->key, &ov, INFINITE);
		if (!a_load(ws_handler.run) || ((result == FALSE) && (ERROR_ABANDONED_WAIT_0 == WSAGetLastError()))) {
			break;
		}

		buffer *b = (buffer*)ov;
		if (b) {
			if (this_->bytes_transfered == 0) {
				printf("WORLD_SERVER_ERROR-> TRANSFERE_BYTES[0]\nNODE LOST ID[%d]\n", b->parent);
				nodes[b->parent].active = false;
				nodes[b->parent].area_id = nodes[b->parent].channel_id = nodes[b->parent].continent_id = 0;
				nodes[b->parent].data.data.Clear();
				closesocket(nodes[b->parent].socket);
			}
			else {
				if (!b->type) {
					w_server_process_recv(this_, b);
				}
			}

			if (b->type) {
				delete b;
				b = NULL;
			}
		}
	}

	printf("WORLD_THREAD_CLOSED id[%d]!\n", this_->id);
	this_->active = false;
	return 0;
}

DWORD WINAPI w_server_accept(LPVOID) {
	WSANETWORKEVENTS WSAEvents;

	while (1)
	{
		if (!a_load(ws_handler.accept_run)) break;

		if (WSA_WAIT_TIMEOUT != WSAWaitForMultipleEvents(1, &ws_handler.accept_event, false, 1000, false))
		{
			WSAEnumNetworkEvents(ws_handler.socket, ws_handler.accept_event, &WSAEvents);
			if ((WSAEvents.lNetworkEvents & FD_ACCEPT) && (0 == WSAEvents.iErrorCode[FD_ACCEPT_BIT]))
			{
				sockaddr_in client_address;
				int addressSize = sizeof(client_address);

				SOCKET client_socket = accept(ws_handler.socket, (sockaddr*)&client_address, &addressSize);
				if (client_socket != INVALID_SOCKET)
					w_server_accept_node(client_socket);
			}
		}
	}

	printf("WORLD_SERVER_ACCEPT_THREAD_CLOSED!\n");
	return 0;
}

void WINAPI w_server_accept_node(SOCKET socket) {
	HANDLE iocp = CreateIoCompletionPort((HANDLE)socket, ws_handler.iocp, (ULONG_PTR)0, 0);
	if (iocp == NULL) {
		printf("FAILED TO ASSOCIATE WORLD_NODE_SOCKET WITH IOCP\n");
		shutdown(socket, 2);
		closesocket(socket);
		return;
	}

	w_server_node * node_out = nullptr;
	for (uint32 i = 0; i < SC_WORLD_HANDLER_MAX_NODES; i++)
		if (!node_out && !nodes[i].active) {
			node_out = &nodes[i]; break;
		}

	if (!node_out) {
		printf("WORLD_NODE_LIST_FULL!!\n"); {
			shutdown(socket, 2);
			closesocket(socket);
			return;
		}
	}

	node_out->active = true;
	node_out->socket = socket;
	node_out->data.data.Resize(4);
	w_server_recv(0, node_out->id);
	printf("ACCEPTED_WORLD_NODE ID[%d]\nWAITING FOR INIT_DATA...\n", node_out->id);

	return;
}

uint8 WINAPI w_server_process_node_init_data(uint32 area, uint32 channel, uint32 contient) {
	for (uint32 i = 0; i < SC_WORLD_HANDLER_MAX_NODES; i++) {
		if (nodes[i].active) {
			if (nodes[i].area_id == area) {
				return 0x01;
			}
			else if (nodes[i].channel_id == channel) {
				return 0x02;
			}
			//do check for contient conflict here
		}
	}
	return 0x00;
}

bool WINAPI w_server_process_recv(w_s_working_thread *t, buffer* buffer) {
	if (!buffer->recv_state) {
		uint16 size = buffer->data.ReadUInt16();
		nodes[buffer->parent].opcode = (e_world_node_opcode)buffer->data.ReadUInt16();
		printf("RECV-WORLD_NODE DATA %hu %hu\n", nodes[buffer->parent].opcode, size);
		if (size == 4) {
			t->current_buffer = buffer;
			w_opcode_fn fn = get_w_op_fn(nodes[buffer->parent].opcode);
			if (fn) fn(t);
			else
				printf("OPCODE NULL[%d]\n", nodes[buffer->parent].opcode);
			w_server_recv(0, buffer->parent);
		}
		else {

			buffer->data.Clear();
			buffer->data.Resize(size - 4);
			w_server_recv(1, buffer->parent);
		}
	}
	else {
		buffer->data._pos += t->bytes_transfered;
		if (buffer->data._size > buffer->data._pos) {
			w_server_recv(1, buffer->parent);
			return true;
		}

		buffer->data._pos = 0;
		t->current_buffer = buffer;
		w_opcode_fn fn = get_w_op_fn(nodes[buffer->parent].opcode);
		if (fn) fn(t);
		else
			printf("OPCODE NULL[%d]\n", nodes[buffer->parent].opcode);

		buffer->data.Clear();
		buffer->data.Resize(4);
		w_server_recv(0, buffer->parent);
	}
	return true;
}

w_server_node * WINAPI w_server_get_node(uint32 id) {
	if (id < SC_WORLD_HANDLER_MAX_NODES) {
		return &nodes[id];
	}
	return nullptr;
}

w_server_node * WINAPI w_server_get_node(uint32 continent, uint32 area, uint32 channel) {
	for (uint32 i = 0; i < SC_WORLD_HANDLER_MAX_NODES; i++)
		if (nodes[i].active &&
			nodes[i].continent_id == continent &&
			nodes[i].area_id == area &&
			nodes[i].channel_id == channel)
			return &nodes[i];

	return nullptr;
}

bool WINAPI w_server_remove_player(std::shared_ptr<connection> c) {
	if (c->node_id < SC_WORLD_HANDLER_MAX_NODES && nodes[c->node_id].active) {
		Stream data;
		data.Resize(14);
		data.WriteUInt16(14);
		data.WriteUInt16(S_W_PLAYER_LEAVE_WORLD);
		data.WriteUInt32(c->id);
		data.WriteUInt32(0); //next id
		data.WriteUInt16(IO_REMOVE_PLAYER);
		w_server_send(c->node_id, &data);
		return true;
	}

	return false;
}

buffer::buffer() :type(0), recv_state(0), parent(0) { ZeroMemory(&ov, 0, sizeof(OVERLAPPED)); }

buffer::buffer(uint8 t, uint32	p, uint8 r) : type(t), recv_state(r), parent(p) { ZeroMemory(&ov, 0, sizeof(OVERLAPPED)); }
