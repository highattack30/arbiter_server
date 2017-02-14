#include "bind_contract.h"
#include "Stream.h"
#include "connexion.h"
#include "chat.h"
#include "itemtemplate.h"
#include "servertime.h"

bind_contract::bind_contract(const uint32 i) :contract(BIND_CONTRACT, i), progress_begin(0) {}

void bind_contract::execute(uint32 unk)
{
	data._pos = 32;
	slot_id s_id = data.ReadUInt32();
	owner->i_.equipe_item(s_id);

	owner->i_.send();

	root->end_contract(id);
}

void bind_contract::begin(uint32 unk)
{
	progress_begin = timer_get_day_seconds(); //??
	send_social(11, owner);
	//todo ?
}

bool bind_contract::internal_init()
{
	if (a_load(owner->status) > 0)
	{
		chat_send_simple_system_message("Can not bind while you're in combat!", owner);
		return false;
	}

	data._pos = 32;
	slot_id s_id = data.ReadUInt32();

	if (owner->i_.inventory_slots[s_id - 40].isEmpty) return false;

	inventory_slot& i_ = owner->i_.inventory_slots[s_id - 40];

	if (i_._item->item_t->requiredLevel > owner->level) {
		message_system_send_simple("@29", owner);
		return false;
	}

	bool can_equipe = false;
	for (size_t i = 0; i < i_._item->item_t->requiredClasses.size(); i++)
		if (i_._item->item_t->requiredClasses[i] == owner->pClass) {
			can_equipe = true;
			break;
		}
	if (!can_equipe && i_._item->item_t->requiredClasses.size() > 0) return false;

	if (i_._item->item_t->bind_type != BIND_ON_EQUIPE) {
		chat_send_simple_system_message("Item can not be bound by equiping!", owner);
		return false;
	}

	if (i_._item->isBinded && i_._item->binderDBId != owner->dbid) {
		message_system_send_simple("@347", owner);
		return false;
	}

	Stream d_0;

	d_0.WriteInt16(0);
	d_0.WriteInt16(S_REQUEST_CONTRACT);
	uint16 name_pos = d_0.NextPos();
	uint16 unk_pos_0 = d_0.NextPos();
	uint16 unk_pos_1 = d_0.NextPos();
	uint16 unk_pos_2 = d_0.NextPos();

	d_0.WriteWorldId(owner);
	d_0.WritePos(unk_pos_2);

	d_0.WriteUInt64(0);
	d_0.WriteUInt32(type);
	d_0.WriteUInt64(id);
	d_0.WriteInt32(0);

	d_0.WritePos(name_pos);
	d_0.WriteString(owner->name);

	d_0.WritePos(unk_pos_0);
	d_0.WriteInt16(0); //text?

	d_0.WritePos(unk_pos_1);
	d_0.WriteUInt64(i_._item->eid);
	d_0.WriteUInt32(0);
	d_0.WriteUInt32(s_id);
	d_0.WriteUInt32(0);
	d_0.WriteUInt32(0);
	d_0.WritePos(0);

	if (!connection_send(owner->con, &d_0))
		return false;

	return true;
}
