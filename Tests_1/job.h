#ifndef JOB_H
#define JOB_H

#include "typeDefs.h"
#include "jobEnums.h"
#include "playerEnums.h"
#include "win32.h"
#include "connexionEnums.h"
#include "pms.h"

#include <memory>

struct RECV_BUFFER; struct SEND_BUFFER;
class connection; class player;

struct job
{
	OVERLAPPED	ov;
	e_job_type	type;
	void*		j_;

	job(void*, e_job_type);
	job();
	~job();
};

struct job_todo
{
	ULONG_PTR	lpCompletionKey;
	job*		job; //overlapped
	ULONG_PTR	Internal;
	DWORD		dwNumberOfBytesTransferred;

	~job_todo();
	job_todo();

};


struct j_recv
{
	j_recv(std::shared_ptr<connection>, e_connexion_recv_buffer_state s);

	std::shared_ptr<connection> con;
	e_connexion_recv_buffer_state b_state;
};

struct j_send
{
	j_send(std::shared_ptr<SEND_BUFFER>);

	std::shared_ptr<SEND_BUFFER> buffer;
};

struct j_enter_world
{
	j_enter_world(std::shared_ptr<player>, uint32[2]);

	std::shared_ptr<player> p_;
	const uint32			data[2];
	//uint32	continent_id;
	//uint32	channel;
};

struct j_enter_world_fin
{
	j_enter_world_fin(std::shared_ptr<player>, bool);


	std::shared_ptr<player> p_;
	bool result;
};

struct j_exit_world
{
	j_exit_world(std::shared_ptr<player>);
	std::shared_ptr<player> p_;
};

struct j_b_despawn_me
{
	j_b_despawn_me(std::shared_ptr<player>);
	std::shared_ptr<player>p_;
};

struct j_b_spawn
{
	j_b_spawn(std::shared_ptr<player>);
	const std::shared_ptr<player> w_p_;
	player_list p_l;
};

struct j_b_despawn
{
	j_b_despawn(std::shared_ptr<player>);

	const std::shared_ptr<player> w_p_;
	player_list p_l;
};

struct j_move
{
	j_move(std::shared_ptr<player> ,float[3], e_player_move_type);

	const float p_init[3];

	const std::shared_ptr<player> p_;
	const e_player_move_type t_;
};

struct j_b_move
{
	j_b_move(std::shared_ptr<player>, e_player_move_type, const float[3]);

	const float p_init[3];
	const std::shared_ptr<player> p_;
	const e_player_move_type t_;
	
	player_list p_m;
};

#endif // ifndef JOB_H
