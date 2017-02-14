#ifndef SLIST_H
#define SLIST_H

#include <memory>
#include <atomic>

#include <Windows.h>

template<typename T> class slist
{
	
public:
	struct Node { Node();  std::shared_ptr<T>  t; std::shared_ptr<Node> next; };

	slist();
	~slist();

	void push_front(std::shared_ptr<T> t);
	auto extract(std::shared_ptr<T> t);

	std::atomic<unsigned long long> size;
	std::shared_ptr<Node> head;

private:
	CRITICAL_SECTION extract_lock;
};


template<typename T>
slist<T>::slist() : head(nullptr) { size.store(0); InitializeCriticalSection(&extract_lock); }

template<typename T>
slist<T>::~slist()
{
	DeleteCriticalSection(&extract_lock);
}


template<typename T>
void slist<T>::push_front(std::shared_ptr<T> t)
{
	auto p = std::make_shared<Node>();
	p->t = t;
	p->next = head;
	auto h = atomic_load(&head);
	while (!atomic_compare_exchange_weak(&head, &p->next, p)) {}


	unsigned long long ex = size.load();
	while (!size.compare_exchange_weak(ex, ex + 1)) {}
}

template<typename T>
auto slist<T>::extract(std::shared_ptr<T> t)
{
	if (!TryEnterCriticalSection(&extract_lock))
		return false;

	auto p = std::make_shared<Node>();
	p = nullptr;

	auto p2 = atomic_load(&head);

	while (1)
	{
		if (!p2) break;
		if (p2->t == t)
		{
			if (!p)
				while (1) { if (atomic_compare_exchange_weak(&p2, &p2, p2->next)) break; }
			else
				while (1) { if (atomic_compare_exchange_weak(&p->next, &p->next, p2->next)) break; }

			unsigned long long ex = size.load();
			while (!size.compare_exchange_weak(ex, ex - 1)) {}

			break;
		}

		p = p2;
		p2 = atomic_load(&p2->next);

	}

	LeaveCriticalSection(&extract_lock);
	return true;
}



#endif

