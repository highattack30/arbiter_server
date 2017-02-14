#ifndef PLYAER_RCU
#define PLYAER_RCU

#include "win32.h"
#include "typeDefs.h"

#include <concurrent_queue.h>

class player;

class player_rcu
{
	struct node { std::shared_ptr<player> p_; std::shared_ptr<node> next; };
	struct op_node { std::shared_ptr<node> n_; bool remove; };

	CRITICAL_SECTION update_lock;
	concurrency::concurrent_queue<std::shared_ptr<op_node>> to_op;
	std::atomic_int64_t count;

	std::shared_ptr<node> head;



	void push_front(std::shared_ptr<node>);
	void remove(entityId);
public:
	player_rcu();
	~player_rcu();

	void push_or_remove(std::shared_ptr<player>, bool remove = false);
	uint64 get_count() const;

	void update_players(float ,float);
	void clear();

	void collect_visible_players(player_list& out_collection, std::shared_ptr<player>);
	void collect_visible_players(player_list& out_collection, float[4], entityId);
};
#endif



