#include "zlist.h"
#include "continent.h"

zlist::zlist()
{
	head = nullptr;
	InitializeCriticalSection(&extract_lock);
}

zlist::~zlist()
{
	DeleteCriticalSection(&extract_lock);
}

void zlist::push_front(std::shared_ptr<zone> p)
{
	auto p_n = std::make_shared<Node>();
	p_n->_t = p;
	p_n->next = nullptr;
	while (1) { if (atomic_compare_exchange_weak(&head, &head, p_n)) break; }
}

bool zlist::erase(uint32 z_id)
{
	if (!TryEnterCriticalSection(&extract_lock))
		return false;

	auto p_n_1 = (std::shared_ptr<Node>)nullptr;
	auto p_n_2 = atomic_load(&head);

	while (1)
	{
		if (!p_n_2) break;
		if (p_n_2->_t->_t->worldMapSectionId == z_id)
		{
			if (!p_n_1)
				while (1) { if (atomic_compare_exchange_weak(&p_n_2, &p_n_2, p_n_2->next)) break; }
			else
				while (1) { if (atomic_compare_exchange_weak(&p_n_1->next, &p_n_1->next, p_n_2->next)) break; }
		}
	}
	LeaveCriticalSection(&extract_lock);
	return true;
}

zlist::Node::Node() : _t(nullptr), next(nullptr) {}

