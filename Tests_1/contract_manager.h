#ifndef CONTRACT_MANAGER_H
#define CONTRACT_MANAGER_H

#include "typeDefs.h"
#include "win32.h"
#include "ContractEnum.h"
#include "Stream.h"

#include <vector>

struct contract;

struct contract_manager
{
	contract_manager();
	~contract_manager();

	void init(p_ptr);

	contract * request_contract(e_contract_type, Stream&);

	void end_contract(const uint32, byte by_type = 0);

	bool has_contract(e_contract_type)const;
	bool has_contract(const uint32)const;

	contract * get_contract(e_contract_type);
	contract * get_contract(const uint32);

private:
	p_ptr	owner;
	uint32	contract_id_pool;
	std::vector<contract*> contracts;
};
#endif
