#include "chat_data.h"

chat_message::chat_message(e_chat_type t, std::shared_ptr<player> p, std::string msg, byte is_g, uint64 time) :p_(p), t_(t), msg_(std::move(msg)), is_gm(is_g), time_stamp(time)
{
	memset(&ov, 0, sizeof(ov));
}
