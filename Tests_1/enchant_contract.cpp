#include "enchant_contract.h"
#include "player.h"
#include "inventory.h"
#include "entity_manager.h"
#include "opcodeEnum.h"
#include "connexion.h"
#include "chat.h"
#include "itemtemplate.h"
#include "passivitytemplate.h"
#include "skylake_stats.h"

enchant_contract::enchant_contract(const uint32 id, byte type_) : contract(ENCHANT_CONTRACT, id), i_(nullptr), type_unk(type_), action(nullptr) {}

bool enchant_contract::internal_init() {
	Stream st;
	st.WriteInt16(0);
	st.WriteInt16(S_REQUEST_CONTRACT);
	uint16 namePos = st.NextPos();
	uint16 unkPos = st.NextPos();
	uint16 unkPos2 = st.NextPos();
	uint16 unkPos3 = st.NextPos();
	//st.WriteUInt16(17); //0x0011

	st.WriteWorldId(owner);
	st.WritePos(unkPos3);
	st.WriteUInt64(0);
	st.WriteInt32(type);
	st.WriteInt64(id);
	st.WriteInt32(0);
	st.WritePos(namePos);
	st.WriteString(owner->name);

	st.WritePos(unkPos);
	st.WriteInt16(0); //unk  text?

	st.WritePos(unkPos2);
	st.WriteInt32(0);
	st.WriteInt32(0);
	st.WriteInt32(0);
	st.WriteInt32(0);
	st.WriteUInt8(0); //awakened enchantment
	st.WritePos(0);

	if (!connection_send(owner->con, &st))
		return false;

	return true;
}

void enchant_contract::try_enchant() {
	if (!action) {
		cancel();
	}

	int32 m_id_0 = 0, index_0 = -1, index_1 = -1; bool remove_0 = false, remove_1 = false;
	owner->i_.lock();
	for (size_t i = 0; i < owner->i_.slot_count; i++) {
		if (!owner->i_.inventory_slots[i].isEmpty) {
			if (owner->i_.inventory_slots[i]._item->eid == action->material_eid_1) {
				index_0 = i;
				if (index_1 >= 0) break;
			}
			else if (owner->i_.inventory_slots[i]._item->eid == action->material_eid_2) {
				index_1 = i;
				if (index_0 >= 0) break;
			}
		}
	}

	if (index_0 >= 0) {
		owner->i_.inventory_slots[index_0]._item->stackCount -= action->m_1_count;
		if (owner->i_.inventory_slots[index_0]._item->stackCount <= 0) {
			owner->i_.inventory_slots[index_0].isEmpty = 0x01;
			owner->i_.inventory_slots[index_0]._item = nullptr;
			remove_0 = true;
		}
	}
	if (index_1 >= 0) {
		owner->i_.inventory_slots[index_1]._item->stackCount -= action->m_2_count;
		if (owner->i_.inventory_slots[index_1]._item->stackCount <= 0) {
			owner->i_.inventory_slots[index_1].isEmpty = 0x01;
			owner->i_.inventory_slots[index_1]._item = nullptr;
			remove_1 = true;
		}
	}
	owner->i_.unlock();

	if (index_1 < 0 && action->material_eid_2 > 0) {
		chat_send_simple_system_message("INTERNAL_ERROR 5003", owner);
		cancel();
		return;
	}
	else if (action->material_eid_2 > 0 && index_0 < 0) {
		chat_send_simple_system_message("INTERNAL_ERROR 5003", owner);
		cancel();
		return;
	}

	if (remove_0)
		entity_manager::destroy_item(action->material_eid_1);
	if (remove_1)
		entity_manager::destroy_item(action->material_eid_2);



	bool success = e_stats_calculate_enchant(i_->enchantLevel + 1, m_id_0, action->m_1_count);
	if (success) {
		i_->enchantLevel++;
		send_social(20, owner);
	}
	else {
		send_social(30, owner);
	}

	Stream data;
	data.Resize(13);
	data.WriteUInt16(13);
	data.WriteUInt16(S_TEMPER_RESULT);
	data.WriteUInt8(success ? 0x01 : 0x00);
	data.WriteUInt32(i_->item_t->id);
	data.WriteUInt32(i_->enchantLevel);
	connection_send(owner->con, &data);
	data.Clear();

	if (success) {
		if ((i_->isAwakened && i_->enchantLevel == 15) || (i_->isMasterworked && i_->enchantLevel == 12) || (i_->enchantLevel == 9)) {
			if (!owner->i_.insert_or_stack_item(i_)) {
				//todo send by percel
			}

			i_ = nullptr;
		}
	}

	std::shared_ptr<item> temp = i_;
	i_ = nullptr;
	enchant_write_materials();
	i_ = temp;
	enchant_write_materials();

	owner->i_.send();
}

void enchant_contract::set_byte(byte type_) {
	type_unk = type_;
}

void enchant_contract::cancel_temper()
{
	send_social_cancel(owner);
	enchant_write_materials();
	owner->i_.send();
}

void enchant_contract::cancel() {
	if (i_) {
		if (!owner->i_.insert_or_stack_item(i_)) {
			//todo send by percel
		}
	}
	owner->i_.send();

	contract::cancel();
}

bool enchant_contract::add_item_to_enchant(int32 unk1, item_eid i_eid, int32 unk2, int32 unk3)
{
	if (!action) {
		action = std::make_unique<enchant_action>();
	}

	if (i_) {//there is an item inside the temper window
		if (!owner->i_.insert_or_stack_item(i_)) {
			chat_send_simple_system_message("You need more space in inventory to do that!", owner);
			return false;
		}
		else {
			i_ = nullptr;
		}
	}

	i_ = owner->i_.get_inventory_item_by_eid(i_eid, 1, 0);
	if (!i_) {
		chat_send_simple_system_message("Could not add item to temper!", owner);
		return false;
	}
	action->i_eid = i_eid;

	if (!enchant_write_materials())
		return false;

	owner->i_.send();
	return true;
}



bool enchant_contract::add_material_to_enchant_process(item_eid i_eid, int32 count, uint32 slot)
{
	if (!action) {
		action = std::make_unique<enchant_action>();
	}

	if (slot == 1) {
		action->m_1_count = count;
		action->material_eid_1 = i_eid;
	}
	else if (slot == 2) {
		action->m_2_count = count;
		action->material_eid_2 = i_eid;
	}

	return true;
}

bool enchant_contract::enchant_write_materials() {
	Stream  data;

	if (!i_) {
		data.WriteUInt16(8);
		data.WriteUInt16(S_TEMPER_MATERIALS);
		data.WriteUInt16(0);
		data.WriteUInt16(0);
		return connection_send(owner->con, &data);
	}

	data.WriteUInt16(0);
	data.WriteUInt16(S_TEMPER_MATERIALS);

	data.WriteUInt16(1);
	data.WriteUInt16(8);
	data.WriteUInt16(8);
	data.WriteUInt16(0);

	data.WriteUInt64(0);

	data.WriteUInt32(i_->item_t->id);
	data.WriteUInt64(i_->eid);

	data.WriteInt64(0); //? 210A3400 00000000 

	data.WriteUInt64(40);

	data.WriteInt32(2);//??
	data.WriteInt32(1);//??
	data.WriteInt32(i_->enchantLevel);
	data.WriteInt32(3);
	data.WriteInt32(4);

	data.WriteUInt8(1);
	for (size_t i = 0; i < i_->passivities.size(); i++) {
		data.WriteUInt32(i_->passivities[i]->id);
	}

	uint32 size = 13 - i_->passivities.size();
	if (size > 0)
		for (uint32 i = 0; i < size; i++) {
			data.WriteUInt32(0);
		}

	data.WriteInt64(0);

	data.WriteUInt8(i_->isMasterworked);
	data.WriteUInt8(i_->isEnigmatic);

	data.WriteInt32(0);//levels
	data.WriteInt32(0);//levels
	data.WriteInt32(0);//levels

	data.WriteInt32(0);//crystals?
	//for (size_t i = 0; i < 4; i++)
	//{
	//	short next = data.NextPos();
	//	data.WritePos(next);
	//	data.WriteInt16(data._pos);
	//
	//	data.WriteInt32(5259); //crystal id
	//}

	data.WritePos(0);

	return connection_send(owner->con, &data);
}

enchant_contract::enchant_action::enchant_action() :i_eid(0), material_eid_1(0), material_eid_2(0), m_1_count(0), m_2_count(0) {}

