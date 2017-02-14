#ifndef PMS_H
#define PMS_H

#include "typeDefs.h"
#include "playerEnums.h"
class player;

//player_move_struct
struct p_ms
{
	float position[3];
	e_player_move_type type;
}; //todo:: optimize!!



#endif
