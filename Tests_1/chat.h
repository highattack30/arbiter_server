#ifndef CHAT_HANDLER_H
#define CHAT_HANDLER_H

#include "typedefs.h"
#include "win32.h"

#include <vector>
#include <atomic>
#include <memory>

class player;
struct chat_message;

struct c_s
{
	HANDLE*					worker_threads;
	HANDLE					chat_iocp;
	std::atomic_bool		run;
	uint32					start_count;
} static chat_s;

bool WINAPI chat_init();
void WINAPI chat_release();

static DWORD WINAPI chat_worker_thread(LPVOID argv);
static void WINAPI chat_process_message(chat_message*);
static bool WINAPI chat_process_gm_commands(chat_message*);

bool WINAPI chat_process_message_async(chat_message*);
void WINAPI chat_send_simple_system_message(std::string, std::shared_ptr<player>);

void WINAPI send_social(uint32, p_ptr);
void WINAPI send_social_cancel(p_ptr);

void WINAPI message_system_send_simple(std::string, std::shared_ptr<player>);
#endif
