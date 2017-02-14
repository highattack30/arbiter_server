#include "entity_manager.h"
#include "player.h"
#include "inventory.h"

void entity_manager::init()
{
	InitializeCriticalSection(&p_critical);
	InitializeCriticalSection(&i_critical);
}

void entity_manager::release()
{
	while (p_head)
	{
		p_head = p_head->next;
	}

	while (i_head)
	{
		i_head = i_head->next;
	}

	p_count = 0;
	i_count = 0;

	//todo clear lists


	DeleteCriticalSection(&p_critical);
	DeleteCriticalSection(&i_critical);
}

std::shared_ptr<player> entity_manager::create_player(std::shared_ptr<connection> c, uint32 db_id)
{
	std::shared_ptr<player> p = std::make_shared<player>(++p_eid_pool, db_id, std::move(c));
	add_player(p);
	return std::move(p);
}

std::shared_ptr<item> entity_manager::create_item()
{
	std::shared_ptr<item> p = std::make_shared<item>(++i_eid_pool);
	add_item(p);
	return std::move(p);
}

std::shared_ptr<item> entity_manager::create_item(std::shared_ptr<item> i)
{
	std::shared_ptr<item> p = std::make_shared<item>(++i_eid_pool);

	p->item_t = i->item_t;

	p->stackCount = i->stackCount;
	p->itemLevel = i->itemLevel;
	p->binderDBId = i->binderDBId;
	p->crafterDBId = p->crafterDBId;
	memcpy(p->crystals, i->crystals, sizeof(uint32) * 4);
	p->enchantLevel = i->enchantLevel;
	p->hasCrystals = i->hasCrystals;
	p->isAwakened = i->isAwakened;
	p->isBinded = i->isBinded;
	p->isCrafted = i->isCrafted;
	p->isEnigmatic = i->isEnigmatic;
	p->isMasterworked = i->isMasterworked;


	add_item(p);
	return std::move(p);
}

std::shared_ptr<item> entity_manager::get_item(entityId id)
{
	std::shared_ptr<i_node> temp = std::atomic_load(&i_head);

	while (1)
	{
		if (!temp) break;

		if (temp->i_->eid == id)
			return temp->i_;

		temp = std::atomic_load(&temp->next);
	}

	return nullptr;
}

std::shared_ptr<player> entity_manager::get_player(entityId id)
{
	std::shared_ptr<p_node> temp = std::atomic_load(&p_head);

	while (1)
	{
		if (!temp) break;

		if (temp->p_->eid == id)
			return temp->p_;

		temp = std::atomic_load(&temp->next);
	}

	return nullptr;
}

uint64 entity_manager::item_count()
{
	return i_count.load(std::memory_order_relaxed);
}

uint64 entity_manager::players_count()
{
	return p_count.load(std::memory_order_relaxed);
}





void entity_manager::add_player(std::shared_ptr<player> p)
{
	std::shared_ptr<p_node> n = std::make_shared<p_node>();
	n->p_ = std::move(p);
	
	EnterCriticalSection(&p_critical);
	n->next = p_head;
	while (1) { if (std::atomic_compare_exchange_weak(&p_head, &p_head, n))break; }
	LeaveCriticalSection(&p_critical);

	p_count++;
}

void entity_manager::add_item(std::shared_ptr<item> i) 
{
	std::shared_ptr<i_node> n = std::make_shared<i_node>();
	n->i_ = std::move(i);

	EnterCriticalSection(&i_critical);
	n->next = i_head;
	while (1) { if (std::atomic_compare_exchange_weak(&i_head, &i_head, n))break; }
	LeaveCriticalSection(&i_critical);

	i_count++;
}

void entity_manager::destroy_player(entityId id)
{
	EnterCriticalSection(&p_critical);

	std::shared_ptr<p_node> p_0 = nullptr;
	std::shared_ptr<p_node> p_1 = p_head;

	while (1)
	{
		if (!p_1) break;

		if (p_1->p_->eid == id)
		{
			if (!p_0) { while (1) { if (std::atomic_compare_exchange_weak(&p_head,&p_head, p_head->next)) break; } }
			else { while (1) { if (std::atomic_compare_exchange_weak(&p_0->next, &p_0->next, p_1->next)) break; } }

			p_count--;
			break;
		}

		p_0 = p_1;
		p_1 = p_1->next;
	}
	LeaveCriticalSection(&p_critical);
}

void entity_manager::destroy_item(entityId id)
{
	EnterCriticalSection(&i_critical);

	std::shared_ptr<i_node> p_0 = nullptr;
	std::shared_ptr<i_node> p_1 = i_head;

	while (1)
	{
		if (!p_1) break;

		if (p_1->i_->eid == id)
		{
			if (!p_0) { while (1) { if (std::atomic_compare_exchange_weak(&i_head, &i_head, i_head->next)) break; } }
			else { while (1) { if (std::atomic_compare_exchange_weak(&p_0->next, &p_0->next, p_1->next)) break; } }

			i_count--;
			break;
		}

		p_0 = p_1;
		p_1 = p_1->next;
	}

	LeaveCriticalSection(&i_critical);
}


std::atomic_uint64_t						entity_manager::i_count;
std::atomic_uint64_t						entity_manager::p_count;

CRITICAL_SECTION							entity_manager::p_critical;
CRITICAL_SECTION							entity_manager::i_critical;

std::shared_ptr<entity_manager::p_node>		entity_manager::p_head;
std::shared_ptr<entity_manager::i_node>		entity_manager::i_head;

std::atomic_uint64_t						entity_manager::p_eid_pool;
std::atomic_uint64_t						entity_manager::i_eid_pool;