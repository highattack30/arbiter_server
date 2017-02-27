#ifndef CHAT_HANDLER_DATA_H
#define CHAT_HANDLER_DATA_H

#include "typeDefs.h"
#include "serverEnums.h"
#include "chatEnums.h"
#include "win32.h"

#include <memory>

class player;
struct chat_message {
	chat_message(e_chat_type, std::shared_ptr<player>, std::string, byte, uint64 t = 0);

	OVERLAPPED						ov;
	std::vector<uint32>				p_l;
	std::string						msg_;
	const e_chat_type				t_;
	const std::shared_ptr<player>	p_;
	const byte						is_gm;
	const uint64					time_stamp;
};


#endif
