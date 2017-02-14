#ifndef WORLDENUMS_H
#define WORLDENUMS_H


#include "typeDefs.h"

enum e_world_zone_state : uint64
{
	ZONE_ACTIVE,
	ZONE_INACTIVE,
	ZONE_FREEZ
};

enum e_worker_state :uint64
{
	WORKER_INACTIVE,
	WORKER_ACTIVE,
	WORKER_DEAD
};

#endif
