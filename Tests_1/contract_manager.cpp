#include "contract_manager.h"
#include "contract.h"
#include "chat.h"
#include "contract_factory.h"

contract_manager::contract_manager() : contract_id_pool(0), owner(nullptr) {}

contract_manager::~contract_manager()
{
	for (size_t i = 0; i < contracts.size(); i++)
	{
		if (contracts[i]) {
			delete contracts[i];
			contracts[i] = nullptr;
		}
	}
	contracts.clear();
}

void contract_manager::init(p_ptr p) {
	owner = std::move(p);
}

contract* contract_manager::request_contract(e_contract_type t, Stream & d)
{
	if (has_contract(t)) return nullptr;
	contract* out = contract_factory::new_contract(t, ++contract_id_pool);
	if (out) {
		if (!out->init(owner, this, d)) {
			delete out; return nullptr;
		}

		contracts.push_back(out);
	}

	return out;
}


void contract_manager::end_contract(const uint32 c, byte by_type)
{
	for (size_t i = 0; i < contracts.size(); i++) {
		if (by_type) {
			if (contracts[i]->type == (e_contract_type)c) {
				delete contracts[i];
				contracts[i] = nullptr;
				contracts.erase(contracts.begin() + i);
				break;
			}
		}
		else {
			if (contracts[i]->id == c) {
				delete contracts[i];
				contracts[i] = nullptr;
				contracts.erase(contracts.begin() + i);
				break;
			}
		}
	}

	send_social_cancel(owner);
}

bool contract_manager::has_contract(e_contract_type t) const
{
	for (size_t i = 0; i < contracts.size(); i++)
		if (contracts[i]->type == t) return true;
	return false;
}

bool contract_manager::has_contract(const uint32 c) const
{
	for (size_t i = 0; i < contracts.size(); i++)
		if (contracts[i]->id == c) return true;
	return false;
}

contract* contract_manager::get_contract(e_contract_type t)
{
	for (size_t i = 0; i < contracts.size(); i++)
		if (contracts[i]->type == t) return contracts[i];

	return nullptr;
}

contract* contract_manager::get_contract(const uint32 c)
{
	for (size_t i = 0; i < contracts.size(); i++)
		if (contracts[i]->id == c) return contracts[i];

	return nullptr;
}

