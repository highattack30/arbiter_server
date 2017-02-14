#ifndef BINDCONTRACT_H
#define BINDCONTRACT_H

#include "contract.h"

struct bind_contract : public contract
{
	bind_contract(const uint32);
	
	void execute(uint32 unk);
	void begin(uint32 unk);

protected:
	int32	progress_begin;

	bool internal_init();
};

#endif
