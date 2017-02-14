#include "chat.h"
#include "arbiter_server.h"
#include "config.h"
#include "chatEnums.h"
#include "chat_data.h"
#include "Stream.h"
#include "opcodeEnum.h"
#include "player.h"
#include "itemEnums.h"
#include "connexion.h"
#include "inventory.h"
#include "entity_manager.h"
#include "stringUtils.h"
#include "world_server.h"
#include "active_server.h"
#include "p_processor.h"

bool WINAPI chat_init()
{
	chat_s.run.store(true);

	chat_s.chat_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (!chat_s.chat_iocp) return false;

	chat_s.start_count = config::chat.worker_threads_count;
	chat_s.worker_threads = new HANDLE[config::chat.worker_threads_count];
	for (uint32 i = 0; i < config::chat.worker_threads_count; i++)
	{
		chat_s.worker_threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)chat_worker_thread, NULL, 0, 0);
		if (!chat_s.worker_threads[i]) chat_s.start_count--;
	}
	printf("::chat_init() started_threads[%d]\n", chat_s.start_count);

	return true;
}

void WINAPI chat_release()
{
	chat_s.run.store(false);
	CloseHandle(chat_s.chat_iocp);
	PostQueuedCompletionStatus(chat_s.chat_iocp, 0, NULL, NULL);
	WaitForMultipleObjects(chat_s.start_count, chat_s.worker_threads, TRUE, INFINITE);

	if (chat_s.worker_threads)
		delete[] chat_s.worker_threads;
}

DWORD WINAPI chat_worker_thread(LPVOID argv)
{
	OVERLAPPED_ENTRY* ov = new OVERLAPPED_ENTRY[config::chat.job_pull_count];
	memset(ov, 0, sizeof(OVERLAPPED_ENTRY) * config::chat.job_pull_count);

	ULONG out_jobs = 0;
	BOOL result = 0;
	while (1)
	{
		if (!chat_s.run.load()) { break; }
		result = GetQueuedCompletionStatusEx(chat_s.chat_iocp, ov, config::chat.job_pull_count, &out_jobs, INFINITE, FALSE);
		if (!result) break;

		for (ULONG i = 0; i < out_jobs; i++)
		{
			if (!ov[i].dwNumberOfBytesTransferred) {
				result = FALSE; break;
			}

			chat_process_message((chat_message*)ov[i].lpOverlapped);

			delete ov[i].lpOverlapped;
			ov[i].lpOverlapped = nullptr;
		}
		if (!result) break;
	}

	for (size_t i = 0; i < config::chat.job_pull_count; i++)
		if (ov[i].lpOverlapped) delete ov[i].lpOverlapped;

	delete[] ov;

	printf("CHAT_THREAD_CLOSED\n");
	return result;
}

void WINAPI chat_process_message(chat_message * msg)
{
	if (!msg) return;

	if (config::chat.enable_gm_commands && chat_process_gm_commands(msg)) {
		return;
	}

	Stream data;

	data.Resize(27);
	data.WriteInt16(0);
	data.WriteInt16(S_CHAT);

	data._pos += 4;

	data.WriteInt32(msg->t_);
	data.WriteWorldId(msg->p_);

	data.WriteUInt8(0);	//unk1
	data.WriteUInt8(msg->is_gm);
	data.WriteUInt8(0);	//unk2

	data.WritePos(4);
	data.WriteString(msg->p_->name);
	data.WritePos(6);
	data.WriteString(msg->msg_);

	data.WritePos(0);

	switch (msg->t_)
	{
	case MAIN_CHAT:
		connection_send(msg->p_->con, &data);
		msg->p_->spawn.bordacast(&data);
		break;
	case PARTY_CHAT:
		break;
	case GUILD_CHAT:
		break;
	case SHAUT_CHAT:
		break;
	case TRADE_CHAT:
		break;
	case RAID_CHAT:
		break;
	case CLUB_CHAT:
		break;
	case PRIVATE_CHAT:
		break;
	case WHISHPER_CHAT:
		break;
	case GREATING_CHAT:
		break;
	case UNK_1_CHAT:
		break;
	case CANAL_1_CHAT:
		break;
	case CANAL_2_CHAT:
		break;
	case CANAL_3_CHAT:
		break;
	case CANAL_4_CHAT:
		break;
	case CANAL_5_CHAT:
		break;
	case CANAL_6_CHAT:
		break;
	case CANAL_7_CHAT:
		break;
	case CANAL_8_CHAT:
		break;
	case TRADER_CHAT:
		break;
	case LFP_CHAT:
		break;
	case NOTICE_CHAT:
		break;
	case ADMIN_CHAT:
		break;
	case UNK_2_CHAT:
		break;
	case SYSTEM_CHAT:
		connection_send(msg->p_->con, &data);
		break;
	case RAID_LEADER_CHAT:
		break;
	case UNK_3:
		break;
	case GLOBAL_CHAT:
		break;
	case ALLIANCE_CHAT:
		break;
	case ECHELON_CHAT:
		break;
	case EXARCH_CHAT:
		break;
	default:
		break;
	}

	return;
}

bool WINAPI chat_process_gm_commands(chat_message * msg)
{
	if (msg->msg_[6] == '.' && msg->msg_[7] == '/')
	{
		if (msg->p_->con->_account.isGm)
		{
			std::string cmd;
			for (size_t i = 8; i < msg->msg_.size() - 7; i++)
				cmd += msg->msg_[i];

			if (stringStartsWith(cmd, "exit"))
			{
				connection_close(msg->p_->con);
			}
			else if (stringStartsWith(cmd, "item"))
			{
				uint32 item_i; uint32 stack = 1;
				sscanf_s(cmd.c_str(), "item %d %d", &item_i, &stack);

				msg->p_->i_.insert_or_stack_item(item_i, stack);
				msg->p_->i_.send();
			}
			else if (stringStartsWith(cmd, "cli"))
			{
				msg->p_->i_.clear();
				msg->p_->i_.send();
				player_send_external_change(msg->p_, 1);
			}
			else if (stringStartsWith(cmd, "itemc"))
			{
				uint64 item_count = entity_manager::item_count();
				chat_send_simple_system_message("ITEM_COUNT:" + std::to_string(item_count), msg->p_);
			}
			else if (stringStartsWith(cmd, "gold"))
			{
				uint64 a = 0;

				sscanf_s(cmd.c_str(), "gold %llu", &a);

				if (a > 0)
					if (msg->p_->i_.add_gold(a))
						msg->p_->i_.send();
			}
			else if (stringStartsWith(cmd, "level"))
			{
				uint32 a = 0;
				sscanf_s(cmd.c_str(), "level %d", &a);
				s_stats_process_progress(msg->p_, a);
			}
			else if (stringStartsWith(cmd, "rstats")) {
				player_recalculate_stats(msg->p_);
			}
			else if (stringStartsWith(cmd, "rlevel")) {
				msg->p_->level = 1;
				player_recalculate_stats(msg->p_);

				player_send_stats(msg->p_);
				player_send_external_change(msg->p_, 1);

				std::unique_ptr<Stream> data = std::make_unique<Stream>();
				data->Clear();
				data->WriteInt16(16);
				data->WriteInt16(S_USER_LEVELUP);
				data->WriteWorldId(msg->p_);
				data->WriteInt32(1);

				msg->p_->spawn.bordacast(data.get());
				connection_send(msg->p_->con, data.get());
			}
			else if (stringStartsWith(cmd, "status"))
			{
				uint32 a = 0;

				sscanf_s(cmd.c_str(), "status %lu", &a);

				msg->p_->status.store((uint8)a);

				player_send_stats(msg->p_);

				std::unique_ptr<Stream> data = std::make_unique<Stream>();
				data->Resize(19);
				data->WriteInt16(19);
				data->WriteInt16(S_USER_STATUS);
				data->WriteWorldId(msg->p_);
				data->WriteInt32(a);
				data->WriteInt16(0);
				data->WriteUInt8(0);
				connection_send(msg->p_->con, data.get());
				msg->p_->spawn.bordacast(data.get());
			}
			else if (stringStartsWith(cmd, "conc"))
			{
				std::string m = "CONNECTIONS_COUNT[" + std::to_string(arbiter_get_connection_count()) + "]";
				chat_send_simple_system_message(m, msg->p_);
			}
			else if (stringStartsWith(cmd, "enchant"))
			{
				uint32 val = 0; uint32 slot = 0;
				sscanf_s(cmd.c_str(), "enchant %d %d", &slot, &val);
				if (val <= 15 && slot < 20 && !msg->p_->i_.profile_slots[slot].isEmpty)
				{
					msg->p_->i_.profile_slots[slot]._item->enchantLevel = (uint8)val;
					if (val > 9 && val <= 12) {
						msg->p_->i_.profile_slots[slot]._item->isMasterworked = 0x01;
					}
					else {
						msg->p_->i_.profile_slots[slot]._item->isMasterworked = 0x00;
					}


					if (val > 12 && val <= 15) {
						msg->p_->i_.profile_slots[slot]._item->isMasterworked = 0x01;
						msg->p_->i_.profile_slots[slot]._item->isAwakened = 0x01;
					}
					else {
						msg->p_->i_.profile_slots[slot]._item->isAwakened = 0x00;
					}


					msg->p_->i_.refresh_enchat_effect();
					msg->p_->i_.send(0);
					player_send_stats(msg->p_);
					player_send_external_change(msg->p_, 1);
				}
			}
			else if (stringStartsWith(cmd, "plc"))
			{
				std::string m = "WORLD_PLAYERS_COUNT[" + std::to_string(world_get_player_count()) + "]";
				chat_send_simple_system_message(m, msg->p_);
			}
			else if (stringStartsWith(cmd, "itmc"))
			{
				std::string m = "ITEMS_COUNT[" + std::to_string(entity_manager::item_count()) + "]";
				chat_send_simple_system_message(m, msg->p_);
			}
			else if (stringStartsWith(cmd, "sysmsg")) {
				uint32 id = 0;
				sscanf_s(cmd.c_str(), "sysmsg %d", &id);

				message_system_send_simple("@" + std::to_string(id), msg->p_);
			}
			else if (stringStartsWith(cmd, "lobby")) {
				world_server_process_job_async(new j_exit_world(msg->p_), J_W_PLAYER_EXIT_WORLD);
				active_remove_player(msg->p_);

				Sleep(500);
				Stream d;
				d.Resize(4);
				d.WriteInt16(4);
				d.WriteInt16(S_RETURN_TO_LOBBY);


				connection_send(msg->p_->con, &d);
				Sleep(100);
				msg->p_->con->_inLobby = false;
			}
			else if (stringStartsWith(cmd, "social")) {
				uint32 id = 0;
				sscanf_s(cmd.c_str(), "social %d", &id);
				send_social(id, msg->p_);
			}
			else if (stringStartsWith(cmd, "csocial")) {
				send_social_cancel(msg->p_);
			}
			else if (stringStartsWith(cmd, "expandi")) {
				msg->p_->i_.lock();

				if (msg->p_->i_.slot_count < 104)
					msg->p_->i_.slot_count += 8;
				else
					chat_send_simple_system_message("Max inventory space reached!", msg->p_);

				msg->p_->i_.unlock();
				msg->p_->i_.send();
			}
			else if (stringStartsWith(cmd, "external")) {
				player_send_external_change(msg->p_, 1);
			}
			else if (stringStartsWith(cmd, "roll")) {
				uint32 slot = 0;
				sscanf_s(cmd.c_str(), "roll %d", &slot);

				if (slot < 20) {
					if (!msg->p_->i_.profile_slots[slot].isEmpty) {
						passivity_roll_item(msg->p_->i_.profile_slots[slot]._item);
					}
					else {
						chat_send_simple_system_message("Slot empty!", msg->p_);
						return true;
					}
				}
				else if (slot >= 40 && (slot-40) < msg->p_->i_.slot_count) {
					if (!msg->p_->i_.inventory_slots[slot - 40].isEmpty) {
						passivity_roll_item(msg->p_->i_.inventory_slots[slot - 40]._item);
					}
					else {
						chat_send_simple_system_message("Slot empty!", msg->p_);
						return true;
					}
				}
				else {
					chat_send_simple_system_message("Index not inside inventory or profile!", msg->p_);
					return true;
				}

				chat_send_simple_system_message("Rolled index:" + std::to_string(slot), msg->p_);
				msg->p_->i_.send();
			}
			else
			{
				std::string m = "UNKNOWN_COMMAND[" + cmd + "]";
				chat_send_simple_system_message(m, msg->p_);
			}
		}
		else
		{
			chat_send_simple_system_message("YOU'RE NOT GM!", msg->p_);
		}
		return true;
	}
	return false;
}

bool WINAPI chat_process_message_async(chat_message* msg)
{
	return PostQueuedCompletionStatus(chat_s.chat_iocp, 1, 0, (LPOVERLAPPED)msg) == TRUE ? true : false;
}

void WINAPI chat_send_simple_system_message(std::string msg, std::shared_ptr<player> p)
{
	std::unique_ptr<chat_message> msg_t = std::make_unique<chat_message>(SYSTEM_CHAT, p, msg, 0x00);
	PostQueuedCompletionStatus(chat_s.chat_iocp, 1, 0, (LPOVERLAPPED)msg_t.release());
}

void WINAPI send_social(uint32 animation, p_ptr target) {
	Stream data = Stream(21);
	data.WriteUInt16(21);
	data.WriteUInt16(S_SOCIAL);
	data.WriteWorldId(target);
	data.WriteUInt32(animation);
	data.WriteUInt32(0); //unk1
	data.WriteUInt8(0); //unk2

	connection_send(target->con, &data);
	target->spawn.bordacast(&data);
}

void WINAPI send_social_cancel(p_ptr target) {
	Stream data = Stream(12);
	data.WriteUInt16(12);
	data.WriteUInt16(S_SOCIAL_CANCEL);
	data.WriteWorldId(target);

	connection_send(target->con, &data);
	target->spawn.bordacast(&data);
}

void WINAPI message_system_send_simple(std::string m, std::shared_ptr<player> p) {
	Stream data;
	data.WriteUInt16(0);
	data.WriteUInt16(S_SYSTEM_MESSAGE);
	data.WriteInt16(6);
	data.WriteString(m);
	data.WritePos(0);

	connection_send(p->con, &data);
}
