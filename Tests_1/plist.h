#ifndef PLIST_H
#define PLIST_H

#include "typeDefs.h"
#include "socket.h"

class player;

#include <memory>
#include <atomic>

class plist
{
	struct Node { Node(); std::shared_ptr<player> _t; std::shared_ptr<Node> next; };
public:
	plist();
	~plist();

	void push_front(std::shared_ptr<player>);

	bool erase(uint64 p_eid);


	std::shared_ptr<Node> head;
private:
	CRITICAL_SECTION extract_lock;

};


#endif
