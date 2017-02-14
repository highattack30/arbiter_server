#ifndef ZLIST_H
#define ZLIST_H

#include "typeDefs.h"
#include "socket.h"

class zone;

#include <memory>
#include <atomic>

class zlist
{
	struct Node { Node(); std::shared_ptr<zone> _t; std::shared_ptr<Node> next; };
public:
	zlist();
	~zlist();

	void push_front(std::shared_ptr<zone>);
	bool erase(uint32 z_id);


	std::shared_ptr<Node> head;
private:
	CRITICAL_SECTION extract_lock;

};


#endif

