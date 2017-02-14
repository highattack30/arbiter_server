#include "job.h"

job_todo::~job_todo()
{
	if (job) delete job;
}

job_todo::job_todo()
{
	job = nullptr;
	lpCompletionKey = 0;
	Internal = 0;
	dwNumberOfBytesTransferred = 0;
}





j_recv::j_recv(std::shared_ptr<connection> c, e_connexion_recv_buffer_state s) : con(c), b_state(s) {}

j_send::j_send(std::shared_ptr<SEND_BUFFER> b) : buffer(b) {}

j_enter_world::j_enter_world(std::shared_ptr<player> p, uint32 d[2]) : p_(p), data{ d[0],d[1] } {}

j_enter_world_fin::j_enter_world_fin(std::shared_ptr<player> p, bool r)
{
	result = r;
	p_ = p;
}

j_exit_world::j_exit_world(std::shared_ptr<player> p) : p_(std::move(p)) {}

j_b_despawn_me::j_b_despawn_me(std::shared_ptr<player> p) : p_(std::move(p)) {}

j_b_spawn::j_b_spawn(std::shared_ptr<player> w_p) : w_p_(std::move(w_p)) {}

j_move::j_move(std::shared_ptr<player> p, float p_i[3], e_player_move_type t) : p_(p), p_init{ p_i[0],p_i[1],p_i[2] }, t_(t) {}

j_b_despawn::j_b_despawn(std::shared_ptr<player> w_p) : w_p_(w_p) {}




job::job(void * j, e_job_type t) : j_(j), type(t) { ZeroMemory(&ov, sizeof ov); }

job::job() : j_(0), type(J_MAX) { ZeroMemory(&ov, sizeof ov); }

job::~job()
{
	if (j_) delete j_;
}

j_b_move::j_b_move(std::shared_ptr<player> p , e_player_move_type t, const float p_i[3]) : p_(p), p_init{ p_i[0],p_i[1],p_i[2] }, t_(t) {}
