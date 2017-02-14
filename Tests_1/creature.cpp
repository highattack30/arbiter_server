#include "creature.h"
#include "serverEnums.h"


creature::creature(e_creature_type type) : colideable(0, 0)
{
	this->type = type;
}

creature::~creature()
{

}
