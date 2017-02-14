#include "plist.h"
#include "player.h"

plist::plist()
{
	head = nullptr;
	InitializeCriticalSection(&extract_lock);
}

plist::~plist()
{
	DeleteCriticalSection(&extract_lock);
}

void plist::push_front(std::shared_ptr<player> p)
{
	auto p_n = std::make_shared<Node>();
	p_n->_t = p;
	p_n->next = head;
	while (1) { if (atomic_compare_exchange_weak(&head, &head, p_n)) break; }
}

bool plist::erase(uint64 p_eid)
{
	if (!TryEnterCriticalSection(&extract_lock))
		return false;

	auto p_n_1 = (std::shared_ptr<Node>)nullptr;
	auto p_n_2 = atomic_load(&head);

	while (1)
	{
		if (!p_n_2) break;
		if (p_n_2->_t->pid == p_eid)
		{
			if (!p_n_1)
				while (1) { if (atomic_compare_exchange_weak(&head, &head, p_n_2->next)) break; }
			else
				while (1) { if (atomic_compare_exchange_weak(&p_n_1->next, &p_n_1->next, p_n_2->next)) break; }
			LeaveCriticalSection(&extract_lock);
			return true;
		}

		p_n_1 = p_n_2;
		p_n_2 = atomic_load(&p_n_2->next);
	}

	LeaveCriticalSection(&extract_lock);
	return false;
}

plist::Node::Node() : _t(nullptr), next(nullptr) {}

