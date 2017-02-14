#ifndef CONTRACT_FACTORY_H
#define CONTRACT_FACTORY_H

#include "typeDefs.h"
#include "ContractEnum.h"

struct contract;
struct contract_factory
{
	static contract* new_contract(e_contract_type, const uint32);
};


#endif
