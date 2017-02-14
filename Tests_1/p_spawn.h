#ifndef P_SPAWN_H
#define P_SPAWN_H

#include "typeDefs.h"
#include "win32.h"
#include "DBHandler.h"

#include <concurrent_queue.h>
#include <memory>
#include <atomic>
#include <mutex>

#ifdef _DEBUG
#include <assert.h>
#endif

class player; class Stream; struct p_wms;

class p_spawn
{
	CRITICAL_SECTION update_lock;
	concurrency::concurrent_queue<std::shared_ptr<player>> to_add;
	concurrency::concurrent_queue<entityId> to_remove;

public:
	struct Node { std::shared_ptr<player> p; std::shared_ptr<Node> next; };

	p_spawn();
	~p_spawn();

	void init(std::shared_ptr<player>);


	void add_erase(std::shared_ptr<player>, bool add = true);
	void bordacast(Stream *);
	void clear();
	void filter(player_list&);



	bool inside(entityId);
private:



	void remove(entityId);
	void push_back(std::shared_ptr<player>);

	std::shared_ptr<player> parent;
	std::shared_ptr<Node> head;
};

#endif
