#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include "typeDefs.h"
#include "win32.h"

#include "entity.h"

#include <atomic>
#include <memory>

class player;
struct item;

class connection;
class entity_manager
{
public:

	struct p_node { std::shared_ptr<player> p_; std::shared_ptr<p_node> next; };
	struct i_node { std::shared_ptr<item> i_; std::shared_ptr<i_node> next; };                       


	static void init();
	static void release();

	static std::shared_ptr<player>		create_player(std::shared_ptr<connection> , uint32);
	static std::shared_ptr<item>		create_item();
	static std::shared_ptr<item>		create_item(std::shared_ptr<item>);

	static std::shared_ptr<item>		get_item(entityId);
	static std::shared_ptr<player>		get_player(entityId);

	static uint64						item_count();
	static uint64						players_count();

	static void							destroy_player(entityId);
	static void							destroy_item(entityId);
private:
	static void							add_player(std::shared_ptr<player>);
	static void							add_item(std::shared_ptr<item>);

	

private:
	static CRITICAL_SECTION				p_critical;
	static CRITICAL_SECTION				i_critical;

	static std::atomic_uint64_t			i_count;
	static std::atomic_uint64_t			p_count;

	static std::shared_ptr<p_node>		p_head;
	static std::shared_ptr<i_node>		i_head;

	static std::atomic_uint64_t			p_eid_pool;
	static std::atomic_uint64_t			i_eid_pool;
};

#endif
