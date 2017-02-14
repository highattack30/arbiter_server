#ifndef CONTRACT_H
#define CONTRACT_H

#include "typeDefs.h"
#include "win32.h"
#include "ContractEnum.h"
#include "Stream.h"

struct contract
{
	friend struct contract_manager;

	contract(e_contract_type,uint32);
	virtual ~contract();

	virtual bool init(p_ptr, contract_manager*, Stream &);
	
	virtual void cancel();
protected:
	virtual bool internal_init() = 0;

	contract_manager*		root;
	const e_contract_type	type;
	const uint32			id;
	Stream					data;
	p_ptr					owner;
};



#endif
