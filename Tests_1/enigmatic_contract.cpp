#include "enigmatic_contract.h"
#include "ContractEnum.h"
#include "player.h"
#include "inventory.h"
#include "connexion.h"
#include "chat.h"
#include "skylake_stats.h"
#include "p_processor.h"
#include "passivitytemplate.h"
#include "dataservice.h"

enigmatic_contract::enigmatic_contract(const uint32 i) : contract(ENIGMATIC_CONTRACT, i), i_(nullptr), scroll_id(0), scroll_stack(0), identify_id(0), identify_stack(0), sp_id(0), sp_stack(0) {}

bool enigmatic_contract::add_item(uint32 item, uint32 scroll, uint32 identify) {

	if (i_) {
		if (!owner->i_.insert_or_stack_item(i_)) {
			//send by percel
		}
		i_ = nullptr;
	}

	if (scroll_stack) {
		if (!owner->i_.insert_or_stack_item(scroll_id, scroll_stack)) {
			//send by percel
		}
		scroll_stack = scroll_id = 0;
	}

	if (identify_stack) {
		if (!owner->i_.insert_or_stack_item(identify_id, identify_stack)) {
			//send by percel
		}
		identify_stack = identify_id = 0;
	}

	if (sp_stack) {
		if (!owner->i_.insert_or_stack_item(sp_id, sp_stack)) {
			//send by percel
		}
		sp_stack = sp_id = 0;
	}

	i_ = owner->i_.get_inventory_item_by_eid(item, 1);
	if (!i_) {
		chat_send_simple_system_message("ERROR 6000", owner);
		cancel();
		return false;
	}

	scroll_stack = owner->i_.pull_item_stack(scroll, 1);
	identify_stack = owner->i_.pull_item_stack(identify, 1);

	if (scroll_stack && identify_stack) {
		scroll_id = scroll;
		identify_id = identify;
	}

	Stream st = Stream();
	st.Resize(5);
	st.WriteInt16(5);
	st.WriteInt16(S_CHECK_UNIDENTIFY_ITEMS);
	st.WriteUInt8((scroll_stack && identify_stack) ? 0x01 : 0x00);
	if (!connection_send(owner->con, &st)) {
		cancel();
		return false;
	}

	return true;
}

bool enigmatic_contract::execute() {

	byte was_m = 0x00;
	if (!i_->isMasterworked &&  e_get_masterworked(scroll_id)) {
		i_->isMasterworked = 0x01;
		was_m = 0x01;
	}
	else {
		//TODO CALCULATE NEW RATE
	}

	std::vector<const passivity_template*> temp;
	for (size_t i = 0; i < locked_passivities.size(); i++) {
		temp.push_back(i_->passivities[locked_passivities[i]]);
	}

	passivity_roll_item(i_);

	for (size_t i = 0; i < locked_passivities.size(); i++) {
		i_->passivities[locked_passivities[i]] = temp[i];
	}

	send_item_tooltip(owner, i_, 9);

	if (!owner->i_.insert_or_stack_item(i_)) {
		//send by percel
	}
	i_ = nullptr;
	owner->i_.send();

	Stream data;
	data.WriteInt16(0);
	data.WriteInt16(S_UNIDENTIFY_EXECUTE);
	data.WriteInt16(15);
	uint16 next = data.NextPos();
	data.WriteUInt8(1);
	for (size_t i = 0; i < 15; i++)
	{
		data.WritePos(next);
		data.WriteInt16(data._pos);
		next = data.NextPos();

		data.WriteInt32(350002);
	}
	data.WriteUInt8(1);
	data.WritePos(0);

	if (!connection_send(owner->con, &data))
		return false;

	sp_id = 0;
	sp_stack = 0;
	identify_id = 0;
	identify_stack = 0;
	scroll_id = 0;
	scroll_stack = 0;

	data.Clear();
	data.Resize(6);
	data.WriteInt16(6);
	data.WriteInt16(S_ITEM_IDENTIFY_RESULT);
	data.WriteUInt8(1);
	data.WriteUInt8(was_m); //first time masterwork
	if (!connection_send(owner->con, &data))
		return false;

	return true;
}

bool enigmatic_contract::lock_passivity(std::vector<uint32>& v) {

	if (i_->passivities.size() == 0) {
		cancel();
		chat_send_simple_system_message("", owner);
		return false;
	}

	uint32 n_stack = 0;
	std::vector<uint32> temp;


	if (!sp_id) {
		sp_id = e_get_spellbind_id(owner);
	}

	for (size_t x = 0; x < v.size(); x++)
		for (size_t y = 0; y < i_->passivities.size(); y++)
			if (i_->passivities[y]->id == v[x])
			{
				bool has = false;
				for (size_t k = 0; k < temp.size(); k++)
					if (temp[k] == y) {
						has = true; break;
					}
				if (!has) {
					temp.push_back(y);
					n_stack += e_get_sp_cost();
					break;
				}
			}

	byte result = 0x00;
	if (n_stack) {
		int32 delta = sp_stack - n_stack;

		if (delta > 0) {
			if (!owner->i_.insert_or_stack_item(sp_id, delta)) {
				//todo send percel
			}

			sp_stack -= delta;
			result = 0x01;
		}
		else if (delta < 0) {
			uint32 s_temp = owner->i_.pull_item_stack(sp_id, abs(delta));
			if (s_temp) {
				sp_stack += s_temp;
				result = 0x01;
			}
		}
	}
	else if (sp_stack) {
		if (!owner->i_.insert_or_stack_item(sp_id, sp_stack)) {
			//todo send percel
		}
		sp_stack = 0;	result = 0x01;
	}


	if (result) {
		locked_passivities.clear();
		for (size_t i = 0; i < temp.size(); i++)
			locked_passivities.push_back(temp[i]);
	}

	Stream st = Stream();
	st.Resize(9);
	st.WriteInt16(9);
	st.WriteInt16(S_RANDOM_PASSIVE_LOCK);
	st.WriteUInt8(result);
	st.WriteInt32(sp_stack);

	if (!connection_send(owner->con, &st)) {
		cancel();
		return false;
	}
	st.Clear();

	owner->i_.send();

	return true;
}

void enigmatic_contract::cancel() {
	if (i_) {
		if (!owner->i_.insert_or_stack_item(i_)) {
			//send by percel
		}
		i_ = nullptr;
	}

	if (scroll_stack) {
		if (!owner->i_.insert_or_stack_item(scroll_id, scroll_stack)) {
			//send by percel
		}
		scroll_stack = scroll_id = 0;
	}

	if (identify_stack) {
		if (!owner->i_.insert_or_stack_item(identify_id, identify_stack)) {
			//send by percel
		}
		identify_stack = identify_id = 0;
	}

	if (sp_stack) {
		if (!owner->i_.insert_or_stack_item(sp_id, sp_stack)) {
			//send by percel
		}
		sp_stack = sp_id = 0;
	}

	contract::cancel();
}

bool enigmatic_contract::internal_init() {
	Stream data;
	data.WriteInt16(0);
	data.WriteInt16(S_REQUEST_CONTRACT);
	uint16 namePos = data.NextPos();
	uint16 unkPos = data.NextPos();
	uint16 unkPos2 = data.NextPos();
	uint16 unkPos3 = data.NextPos();
	data.WriteWorldId(owner);
	data.WriteUInt64(0);
	data.WriteUInt32(type);
	data.WriteUInt64(id);
	data.WriteUInt32(0);
	data.WritePos(namePos);
	data.WriteString(owner->name);
	data.WritePos(unkPos);
	data.WriteUInt16(0); //unk  text?
	data.WritePos(unkPos2);
	data.WritePos(0);

	return connection_send(owner->con, &data);
}
