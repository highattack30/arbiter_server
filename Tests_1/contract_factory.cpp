#include "contract_factory.h"
#include "bind_contract.h"
#include "enchant_contract.h"
#include "enigmatic_contract.h"

contract* contract_factory::new_contract(e_contract_type t, const uint32 id)
{
	switch (t)
	{
	case BIND_CONTRACT:
		return new bind_contract(id);
	case ENCHANT_CONTRACT:
		return new enchant_contract(id);
	case ENIGMATIC_CONTRACT:
		return new enigmatic_contract(id);
	case FUSION_CONTRACT:
		return nullptr;
	}
	return nullptr;
}
