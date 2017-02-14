#include "player_rcu.h"

#include "player.h"
#include "Bounds.h"

player_rcu::player_rcu() : head(nullptr)
{
	count = 0;
	InitializeCriticalSection(&update_lock);

	printf("PLAYER_RCU_CONTRUCT\n");
}

player_rcu::~player_rcu()
{
	DeleteCriticalSection(&update_lock);

	std::shared_ptr<node> temp = head;
	head = nullptr;
	while (1)
	{
		if (!temp) break;
		temp = temp->next;
	}
}

void player_rcu::push_or_remove(std::shared_ptr<player> p, bool remove_)
{
	std::shared_ptr<op_node> n_ = std::make_shared<op_node>();
	n_->n_ = std::make_shared<node>();
	n_->n_->next = nullptr;
	n_->remove = remove_;
	n_->n_->p_ = p;
	to_op.push(n_);
	uint8 t = 0;
	while (1)
	{
		if (t > 20) break;
		if (TryEnterCriticalSection(&update_lock)) break;
		t++;
	}
	if (t > 20) return;

	while (1)
	{
		if (!to_op.try_pop(n_)) break;

		if (n_->remove)
			remove(n_->n_->p_->eid);
		else
			push_front(n_->n_);
	}
	LeaveCriticalSection(&update_lock);
}

uint64 player_rcu::get_count() const
{
	return a_load(count);
}

void player_rcu::update_players(float delta_time, float elapsed_time)
{
	std::shared_ptr<node> temp = std::atomic_load(&head);
	while (1)
	{
		if (!temp) break;
		temp->p_->update(delta_time, elapsed_time);
		temp = std::atomic_load(&temp->next);
	}
}

void player_rcu::clear()
{
}

void player_rcu::push_front(std::shared_ptr<node> n)
{
	n->next = head;
	while (1) { if (std::atomic_compare_exchange_weak(&head, &head, n)) break; }
	count++;

}

void player_rcu::remove(entityId id)
{
	std::shared_ptr<node> p_0 = nullptr;
	std::shared_ptr<node> p_1 = head;
	while (1)
	{
		if (!p_1) break;

		if (p_1->p_->eid == id) {

			if (!p_0) while (1) { if (std::atomic_compare_exchange_weak(&head, &head, head->next))break; }
			else while (1) { if (std::atomic_compare_exchange_weak(&p_0->next, &p_0->next, p_1->next))break; }

			count--;
			break;
		}

		p_0 = p_1;
		p_1 = p_1->next;
	}
}


void player_rcu::collect_visible_players(player_list & p_l, std::shared_ptr<player> p)
{
	float p_v_r[4] = {
		(p->position.x.load(std::memory_order_relaxed)) - SC_PLAYER_VISIBLE_RANGE / 2,
		(p->position.y.load(std::memory_order_relaxed)) - SC_PLAYER_VISIBLE_RANGE / 2,
		SC_PLAYER_VISIBLE_RANGE / 2,SC_PLAYER_VISIBLE_RANGE / 2, };

	std::shared_ptr<node> temp = std::atomic_load(&head);
	while (1)
	{
		if (!temp) break;
		if (rectangle_vs_point(
			temp->p_->position.x.load(std::memory_order_relaxed),
			temp->p_->position.y.load(std::memory_order_relaxed),
			p_v_r))

		{
			p_l.push_back(temp->p_);
		}

		temp = std::atomic_load(&temp->next);
	}
}

void player_rcu::collect_visible_players(player_list &p_l, float p_v_r[4], entityId me)
{
	std::shared_ptr<node> temp = std::atomic_load(&head);
	while (1)
	{
		if (!temp) break;
		if (temp->p_->eid != me &&
			rectangle_vs_point(
				temp->p_->position.x.load(std::memory_order_relaxed),
				temp->p_->position.y.load(std::memory_order_relaxed),
				p_v_r))

		{
			p_l.push_back(temp->p_);
		}

		temp = std::atomic_load(&temp->next);
	}
}
