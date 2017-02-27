#include "world_server.h"
#include "arbiter_server.h"
#include "player.h"
#include "connexion.h"
#include "Stream.h"
#include "config.h"
#include "connexionEnums.h"
#include "serverEnums.h"
#include "itemEnums.h"

#include "Bounds.h"
#include "XMLDocumentParser.h"

#include <unordered_map>


bool WINAPI world_server_init()
{
	w_server.w_data = std::make_shared <world_data>();
	//init world_data*********************************************************************************************************

	std::vector<XMLDocumentParser::XMLNode*> nodes;
	std::string file_name;
	file_name += config::dir.dataWorld;
	file_name += "//world.xml";

	if (XMLDocumentParser::ParseXMLDocument(nodes, file_name.c_str()))
		return false;
	XMLDocumentParser::XMLNodeArgument * arg = nullptr;
	int32 c_count = 0;
	std::cout << "WORLD_LOADING_CONTINETS:\n";
	for (size_t i = 0; i < nodes.size(); i++)
	{
		if (nodes[i] && nodes[i]->tagName == "World")
		{
			if ((arg = nodes[i]->ConsumeArgument("name")))
				strcpy_s(w_server.name, arg->argumentValue.c_str());

			if ((arg = nodes[i]->ConsumeArgument("continentCount")))
				c_count = atoi(arg->argumentValue.c_str());

			if (c_count <= 7 && c_count > 0)
				for (int32 j = 0; j < c_count; j++)
				{
					continent c_;
					if (!c_.load_from_xml_node(nodes[i]->childNodes[j])) { c_count = -1; break; }
					std::cout << "WORLD_LOADED_CONTINET [" << c_.name << "]\n";

					w_server.w_data->_w.push_back(std::move(c_));

				}
			else
			{
				c_count = -1;
				break;
			}

		}

	}

	for (size_t i = 0; i < nodes.size(); i++)
		if (nodes[i])
		{
			delete nodes[i];
			nodes[i] = nullptr;
		}

	if (c_count > 0)
		std::cout << "WORLD_LOADED [" << c_count << "] CONTINENTS\n";
	else
		std::cout << "WORLD::LOAD_ERROR. [CHECK world.xml FILE]\n";

	if (c_count == -1)return  false;




	//init_world_workers*********************************************************************************************************
	GetSystemInfo(&w_server.sys_info);
	w_server.no_of_threads = w_server.sys_info.dwNumberOfProcessors * config::server.world_no_of_threads_per_cpu;

	if (WSA_INVALID_EVENT == (w_server.shutdown_event = WSACreateEvent()))
		return false;


	w_server.w_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (!w_server.w_iocp) return false;

	w_server.world_worker_threads = new s_worker_thread*[w_server.no_of_threads];
	for (uint32 i = 0; i < w_server.no_of_threads; i++)
	{
		w_server.world_worker_threads[i] = new s_worker_thread;
		w_server.world_worker_threads[i]->id = i;
		w_server.world_worker_threads[i]->noOfBytesTransfered = 0;
		w_server.world_worker_threads[i]->out_jobs = 0;
		w_server.world_worker_threads[i]->j_c = NULL;
	}
	for (uint32 i = 0; i < w_server.no_of_threads; i++)
		if (!(w_server.world_worker_threads[i]->w_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)world_worker, (LPVOID)w_server.world_worker_threads[i], 0, 0)))
			return false;


	return true;
}

void WINAPI world_server_release()
{
	SetEvent(w_server.shutdown_event);
	CloseHandle(w_server.w_iocp);
	PostQueuedCompletionStatus(w_server.w_iocp, 0, NULL, NULL);

	if (w_server.world_worker_threads)
	{
		for (uint32 i = 0; i < w_server.no_of_threads; i++)
			if (w_server.world_worker_threads[i])
				WaitForSingleObject(w_server.world_worker_threads[i]->w_thread, INFINITE);


		delete[] w_server.world_worker_threads;
		w_server.world_worker_threads = NULL;
	}

	WSACloseEvent(w_server.shutdown_event);

	//todo release world_data ?

	return;
}

uint64 WINAPI world_get_player_count()
{
	return w_server.p_count.load(std::memory_order_relaxed);
}



DWORD WINAPI world_worker(void* argv)
{
	s_worker_thread * this_ = (s_worker_thread *)argv;
	this_->j_c = new job_todo[config::server.world_job_pull_cout];
	memset(this_->j_c, 0, sizeof(job_todo)* config::server.world_job_pull_cout);

	while (1)
	{
		BOOL ret_val = GetQueuedCompletionStatusEx(w_server.w_iocp, (LPOVERLAPPED_ENTRY)this_->j_c, config::server.world_job_pull_cout, (PULONG)&this_->out_jobs,
			INFINITE, FALSE);

		if (WAIT_OBJECT_0 == WaitForSingleObject(w_server.shutdown_event, 0) || (!ret_val  && ERROR_ABANDONED_WAIT_0 == WSAGetLastError()))
			break;

		if (!this_->j_c) continue;

		for (ULONG i = 0; i < this_->out_jobs; i++)
		{
			if (this_->j_c[i].dwNumberOfBytesTransferred)
			{
				job* j = this_->j_c[i].job;
				world_process_job(j->j_, j->type, this_);
				this_->j_c[i].job = nullptr;
			}
			else
			{
				ret_val = FALSE;
				break;
			}
		}

		if (!ret_val) break;
	}

	for (size_t i = 0; i < config::server.world_job_pull_cout; i++)
		if (this_->j_c[i].job) delete this_->j_c[i].job;

	delete[] this_->j_c;
	this_->j_c = nullptr;

	std::cout << "WORLD_WORKER_THREAD_CLOSED\n";
	return 0;
}


void WINAPI world_process_job(void* j, e_job_type type, s_worker_thread * this_)
{
	switch (type)
	{
	case  J_W_PLAYER_ENTER_WORLD:
	{
		world_player_enter_world((j_enter_world*)j);
	}break;

	case J_W_PLAYER_EXIT_WORLD:
	{
		world_player_exit_world((j_exit_world*)j);
	}break;

	case J_W_PLAYER_MOVE:
	{
		world_player_move((j_move*)j);
	}break;

	case J_W_PLAYER_VISIT_NEW_SECTION:
		//	this_->w_t->player_visit_new_zone(j); 
		break;

	default:
		break;
	}


	delete j;
	return;
}

bool WINAPI world_server_process_job_async(void* j, e_job_type t)
{
	if (!j) return false;

	job* newJob = new job(j, t);
	BOOL result = PostQueuedCompletionStatus(w_server.w_iocp, sizeof(job), NULL, (LPOVERLAPPED)newJob);
	if (!result)
	{
		delete newJob;
		w_server.no_of_droped_jobs++;

#ifdef _DEBUG
		printf("WORLD DROPPED JOB! TYPE[%hu] TOTAL_DROPPED_COUNT[%llu]\n", (uint32)t, w_server.no_of_droped_jobs);
#endif
	}
	return result ? true : false;
}


int32 WINAPI world_get_contienent_by_identity(uint32 identity)
{
	for (int8 i = 0; i < 7; i++)
	{
		for (int8 j = 0; j < 6; j++)
		{
			if (w_server.w_data->_w[i].identity[j] == 0)break;
			else if (w_server.w_data->_w[i].identity[j] == identity)
				return i;
		}
	}
	return -1;
}

void WINAPI world_player_enter_world(j_enter_world * j)
{
#ifdef _DEBUG
	assert(j != NULL);
#endif
	std::shared_ptr<player> p = j->p_;

	bool result = false;
	p->p_c_c_id = (uint8)world_get_contienent_by_identity(j->data[0]);
	if (p->p_c_c_id >= 0 && p->p_c_c_id < 7)
	{
		area * a = w_server.w_data->_w[p->p_c_c_id].get_area(p);
		if (a)
		{
			zone * z = a->get_zone(p, p->channel);
			if (z)
			{
				zone::section * s = z->get_section(p);
				if (s)
				{
					s->players.push_or_remove(p);
					result = true;
				}
			}
		}
	}

	if (result)
	{
		std::unique_ptr<j_b_spawn> j_b = std::make_unique<j_b_spawn>(p);
		w_server.w_data->_w[p->p_c_c_id].areas[p->p_c_a].collect_visible_players(j_b->p_l, p);
		if (j_b->p_l.size() > 0)
		{
			arbiter_server_process_job_async(j_b.release(), J_A_BROADCAST_PLAYER_SPAWN);
		}

		w_server.p_count++;
	}


	arbiter_server_process_job_async(new j_enter_world_fin(j->p_, result), J_A_PLAYER_ENTER_WORLD_FIN);
	return;
}

void WINAPI world_player_exit_world(j_exit_world * j)
{
	std::shared_ptr<player> p = j->p_;

	j_b_despawn_me* j_b = new j_b_despawn_me(p);
	w_server.w_data->_w[p->p_c_c_id].areas[p->p_c_a].channels[p->channel][p->p_c_z].sections[p->p_c_s].players.push_or_remove(p, true);

	arbiter_server_process_job_async(j_b, J_A_BROADCAST_PLAYER_DESPAWN_ME);
	w_server.p_count--;
	return;
}

void WINAPI world_player_move(j_move * j)
{
	float p_r[4];
	p_r[0] = j->p_->position.x.load() - SC_PLAYER_VISIBLE_RANGE / 2;
	p_r[1] = j->p_->position.y.load() - SC_PLAYER_VISIBLE_RANGE / 2;
	p_r[2] = p_r[3] = SC_PLAYER_VISIBLE_RANGE / 2;

	area * a_ = &w_server.w_data->_w[j->p_->p_c_c_id.load()].areas[j->p_->p_c_a.load()];
	zone* z_ = &a_->channels[j->p_->channel.load()][j->p_->p_c_z];
	zone::section * s_ = &z_->sections[j->p_->p_c_s];

	if (z_->inflates_bounds(j->p_, true)) //out of zone
	{
		uint8 ch = j->p_->channel.load();

		z_ = a_->get_zone(j->p_, ch);
		s_->players.push_or_remove(j->p_, true);
		s_ = z_->get_section(j->p_);
		s_->players.push_or_remove(j->p_);

		printf("changed zone\n");

	}
	else if (!rectangle_vs_point(j->p_, s_->fences)) //out of section
	{
		s_->players.push_or_remove(j->p_, true);
		s_ = z_->get_section(j->p_);
		s_->players.push_or_remove(j->p_);
		printf("changed section\n");
	}

	if (!a_ || !z_ || !s_)
	{
		printf("whaaaat\n");
		return;
	}

	std::unique_ptr<j_b_move> j_m = std::make_unique<j_b_move>(j->p_, j->t_, j->p_init);
	a_->collect_visible_players(j_m->p_m, j->p_);
	arbiter_server_process_job_async(j_m.release(), J_A_BROADCAST_PLAYER_MOVE);
	return;
}

