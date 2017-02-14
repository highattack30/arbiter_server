#ifndef ENIGMATIC_CONTRACT_H
#define ENIGMATIC_CONTRACT_H

#include "contract.h"

struct item;
struct enigmatic_contract : public contract
{
	enigmatic_contract(const uint32);

	bool add_item(uint32 item, uint32 scroll, uint32 identify);
	bool execute();
	bool lock_passivity(std::vector<uint32>&);

	void cancel();
private:
	std::shared_ptr<item> i_;

	std::vector<uint32> locked_passivities;

	uint32
		scroll_id,
		scroll_stack,

		identify_id,
		identify_stack,

		sp_id,
		sp_stack;

	
	virtual bool internal_init() override;
};
#endif
