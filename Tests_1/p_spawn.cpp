#include "p_spawn.h"
#include "player.h"

#include "connexion.h"
#include "pms.h"
#include "Bounds.h"


void p_spawn::push_back(std::shared_ptr<player> p)
{
	std::shared_ptr<Node> n = nullptr;
	n = std::make_shared<Node>();
	n->next = head;
	n->p = p;
	while (1) { if (std::atomic_compare_exchange_weak(&head, &head, n))break; }
}

void p_spawn::remove(entityId id)
{
	std::shared_ptr<Node> t_0 = nullptr;
	std::shared_ptr<Node> t_1 = head;
	while (1)
	{
		if (!t_1) break;

		if (t_1->p->eid == id)
		{
			if (!t_0) { while (1) { if (std::atomic_compare_exchange_weak(&head, &head, head->next))break; } }
			else {
				while (1) { if (std::atomic_compare_exchange_weak(&t_0->next, &t_0->next, t_1->next))break; }
			}

			t_1 = nullptr;

			break;
		}


		t_0 = t_1;
		t_1 = t_1->next;
	}
}

p_spawn::p_spawn() : head(nullptr)
{
	InitializeCriticalSection(&update_lock);
}

p_spawn::~p_spawn()
{
	DeleteCriticalSection(&update_lock);

}

void p_spawn::init(std::shared_ptr<player> p)
{
	parent = std::move(p);
}

void p_spawn::add_erase(std::shared_ptr<player> p, bool add)
{
	if (add) to_add.push(std::move(p)); else to_remove.push(p->eid);

	byte t_c = 0;
	while (1) {
		if (++t_c > 10 /*TODO: no. of instructions it takes for the function to exit the while_loop and leave the critical_section*/) return;
		if (TryEnterCriticalSection(&update_lock)) break;
	}

	std::shared_ptr<player> temp = nullptr;
	while (1)
	{
		if (!to_add.try_pop(temp)) break;
		push_back(temp);
	}


	entityId temp_id;
	while (1)
	{
		if (!to_remove.try_pop(temp_id)) break;
		remove(temp_id);
	}

	LeaveCriticalSection(&update_lock);
}

void p_spawn::bordacast(Stream * data_m)
{
	std::shared_ptr<Node> temp = std::atomic_load(&head);
	while (1)
	{
		if (!temp) break;
		connection_send(temp->p->con, data_m);
		temp = temp->next;
	}

}

void p_spawn::clear()
{
	Stream data_m = Stream();
	data_m.Resize(16);
	data_m.WriteUInt16(16);
	data_m.WriteUInt16(S_DESPAWN_USER);
	data_m.WriteWorldId(parent);

	EnterCriticalSection(&update_lock);

	while (1)
	{
		if (!head) break;

		head->p->spawn.add_erase(parent, false);

		connection_send(head->p->con, &data_m);

		head = head->next;
	}
	LeaveCriticalSection(&update_lock);
}

void p_spawn::filter(player_list &p_l)
{
	player_list to_depsawn;
	player_list to_spawn;
	std::shared_ptr<Node> temp = std::atomic_load(&head);
	if (p_l.size() > 0)
	{
		while (1)
		{
			if (!temp) break;

			bool in = false;
			for (size_t i = 0; i < p_l.size(); i++)
			{
				if (p_l[i]->eid == temp->p->eid)
				{
					in = true; break;
				}
			}

			if (!in)
			{
				to_depsawn.push_back(temp->p);

				temp->p->spawn.add_erase(parent, false);
				add_erase(temp->p, false);


			}
			temp = temp->next;
		}


		for (size_t i = 0; i < p_l.size(); i++)
		{
			if (!inside(p_l[i]->eid))
			{
				add_erase(p_l[i]);
				p_l[i]->spawn.add_erase(parent);

				to_spawn.push_back(p_l[i]);
			}
		}
	}
	else
	{
		while (1)
		{
			if (!head) break;

			head->p->spawn.add_erase(parent, false);
			to_depsawn.push_back(head->p);
			head = head->next;
		}
	}

	Stream data_y, data_m;
#pragma region despawn
	if (to_depsawn.size() > 0)
	{
		data_m.Resize(16);
		data_m.WriteInt16(16);
		data_m.WriteInt16(S_DESPAWN_USER);
		data_m.WriteWorldId(parent);


		data_y.Resize(16);
		data_y.WriteInt16(16);
		data_y.WriteInt16(S_DESPAWN_USER);
		for (size_t i = 0; i < to_depsawn.size(); i++)
		{
			data_y._pos = 4;
			data_y.WriteWorldId(to_depsawn[i]);

			connection_send(parent->con, &data_y);
			connection_send(to_depsawn[i]->con, &data_m);
		}

		data_y.Clear();
		data_m.Clear();
		to_depsawn.clear();
	}
#pragma endregion

#pragma region spawn
	if (to_spawn.size() > 0) {
		player_write_spawn_packet(parent, data_m);

		for (size_t i = 0; i < to_spawn.size(); i++)
		{
			data_y.Clear();
			player_write_spawn_packet(to_spawn[i], data_y);

			connection_send(to_spawn[i]->con, &data_m);
			connection_send(parent->con, &data_y);
		}

		data_m.Clear();
		data_y.Clear();
		to_spawn.clear();
	}
#pragma endregion
}

bool p_spawn::inside(entityId id)
{
	std::shared_ptr<Node> temp = std::atomic_load(&head);
	while (1)
	{
		if (!temp) break;
		if (temp->p->eid == id) return true;
		temp = temp->next;
	}
	return false;
}
