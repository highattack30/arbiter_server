#include "inventory.h"
#include "itemtemplate.h"
#include "passivitytemplate.h"
#include "dataservice.h"
#include "equipmentdatatemplate.h"
#include "enchantdatatemplate.h"
#include "player.h"
#include "Stream.h"
#include "connexion.h"
#include "itemEnums.h"
#include "player.h"
#include "chat.h"
#include "config.h"
#include "entity_manager.h"
#include "p_processor.h"

#include <fstream>

inventory::inventory() : parent(nullptr)
{
	InitializeCriticalSection(&inv_lock);

	slot_count = 0;
	profileItemLevel = itemLevel = 0;
	gold = 0;
}

inventory::~inventory()
{
	DeleteCriticalSection(&inv_lock);

	for (uint16 i = 0; i < PROFILE_MAX; i++)
	{
		slot_wipe(profile_slots[i]);
	}

	for (uint16 i = 0; i < slot_count; i++)
	{
		slot_wipe(inventory_slots[i]);
	}
}

bool inventory::insert_or_stack_item(item_id id, uint32 stack_count)
{
	uint32 residual = stack_count; bool result = false;
	EnterCriticalSection(&inv_lock);

	for (size_t i = 0; i < slot_count; i++)
		if (!inventory_slots[i].isEmpty && inventory_slots[i]._item &&
			inventory_slots[i]._item->item_t->id == id)
			if (!(residual = item_stack(inventory_slots[i]._item, stack_count, STACK_FRAGMENT)))
			{
				result = true;
				break;
			}


	if (residual)
		for (size_t i = 0; i < slot_count; i++)
			if (inventory_slots[i].isEmpty && slot_insert(inventory_slots[i], id, residual))
			{
				result = true;
				break;
			}


	if (result)
		recalculate_levels();

	LeaveCriticalSection(&inv_lock);
	return result;
}

bool inventory::insert_or_stack_item(std::shared_ptr<item> it)
{
	uint32 residual = it->stackCount; bool result = false;
	EnterCriticalSection(&inv_lock);

	for (size_t i = 0; i < slot_count; i++)
		if (!inventory_slots[i].isEmpty && inventory_slots[i]._item->item_t->id == it->item_t->id)
			if (!(residual = item_stack(inventory_slots[i]._item, it->stackCount, STACK_FRAGMENT)))
			{
				entity_manager::destroy_item(it->eid); //we stacked the item
				result = true;
				break;
			}


	if (residual) {
		it->stackCount = residual;
		for (size_t i = 0; i < slot_count; i++)
			if (inventory_slots[i].isEmpty)
			{
				inventory_slots[i]._item = std::move(it); //we added item into inventory
				inventory_slots[i].isEmpty = 0x00;
				result = true;
				break;
			}
	}
	if (result)
		recalculate_levels();

	LeaveCriticalSection(&inv_lock);
	return result;
}

uint32 inventory::pull_item_stack(item_id id, uint32 stack_count)
{
	EnterCriticalSection(&inv_lock);
	for (size_t i = 0; i < slot_count; i++)
	{
		if (!inventory_slots[i].isEmpty && inventory_slots[i]._item &&
			inventory_slots[i]._item->item_t->id == id && inventory_slots[i]._item->stackCount >= stack_count)
		{
			inventory_slots[i]._item->stackCount -= stack_count;
			if (inventory_slots[i]._item->stackCount <= 0)
				slot_wipe(inventory_slots[i]);

			recalculate_levels();

			LeaveCriticalSection(&inv_lock);
			return stack_count;
		}

	}

	LeaveCriticalSection(&inv_lock);
	return 0;
}

bool inventory::equipe_item(slot_id s_id)
{
	if (s_id < PROFILE_MAX) {
		chat_send_simple_system_message("Cant equipe already equiped items!", parent);
		return false;
	}

	e_profile_slot_type profile_slot = PROFILE_MAX;

	inventory_slot& s_r = inventory_slots[s_id - 40];
	const item_template* i_t = s_r._item->item_t;

	if (!s_r.isEmpty)
	{
		if (i_t->requiredLevel > parent->level) {
			message_system_send_simple("@347", parent);
			return false;
		}

		if (i_t->useOnlyByClass)
		{
			bool can = false;
			for (size_t i = 0; i < i_t->requiredClasses.size(); i++)
				if (i_t->requiredClasses[i] == player_get_class(parent->model)) {
					can = true;
					break;
				}
			if (!can)
			{
				message_system_send_simple("@28", parent); //class issue
				return false;
			}
		}

		if (s_r._item->isBinded && s_r._item->binderDBId != parent->dbid) {
			message_system_send_simple("@347", parent); //not binded to you [bind is done on db-id]
			return false;
		}
		else if (!s_r._item->isBinded && i_t->bind_type == BIND_ON_EQUIPE) {
			s_r._item->isBinded = 0x01;
			s_r._item->binderDBId = parent->dbid;
#ifdef _DEBUG
			chat_send_simple_system_message("ITEM BOUNDED!", parent);
#endif

		}


		EnterCriticalSection(&inv_lock);
		e_item_category i_c = i_t->category;
		switch (i_t->type)
		{
		case EQUIP_ACCESSORY:
		{
			if (i_c == necklace)
			{
				inventory_interchange_items((*this)[PROFILE_NECKLACE], s_r);
			}
			else if (i_c == earring)
			{
				if ((*this)[PROFILE_EARRING_L].isEmpty)
				{
					inventory_interchange_items((*this)[PROFILE_EARRING_L], s_r);
				}
				else if ((*this)[PROFILE_EARRING_R].isEmpty)
				{
					inventory_interchange_items((*this)[PROFILE_EARRING_R], s_r);
				}
				else
				{
					if ((*this)[PROFILE_RING_R]._item->item_t->id == s_r._item->item_t->id) {
						inventory_interchange_items((*this)[PROFILE_EARRING_L], s_r);
					}
					else if ((*this)[PROFILE_EARRING_L]._item->item_t->id == s_r._item->item_t->id) {
						inventory_interchange_items((*this)[PROFILE_EARRING_R], s_r);
					}
					else
						inventory_interchange_items((*this)[PROFILE_EARRING_R], s_r);
				}
			}
			else if (i_c == ring)
			{
				if ((*this)[PROFILE_RING_L].isEmpty)
				{
					inventory_interchange_items((*this)[PROFILE_RING_L], s_r);
				}
				else if ((*this)[PROFILE_RING_R].isEmpty)
				{
					inventory_interchange_items((*this)[PROFILE_RING_R], s_r);
				}
				else
				{
					if ((*this)[PROFILE_RING_R]._item->item_t->id == s_r._item->item_t->id) {
						inventory_interchange_items((*this)[PROFILE_RING_L], s_r);
					}
					else if ((*this)[PROFILE_RING_L]._item->item_t->id == s_r._item->item_t->id) {
						inventory_interchange_items((*this)[PROFILE_RING_R], s_r);
					}
					else
						inventory_interchange_items((*this)[PROFILE_RING_R], s_r);
				}
			}
			else if (i_c == brooch)
			{
				inventory_interchange_items((*this)[PROFILE_BROOCH], s_r);
			}
			else if (i_c == belt)
			{
				inventory_interchange_items((*this)[PROFILE_BELT], s_r);
			}
		}break;
		case EQUIP_WEAPON:
			inventory_interchange_items((*this)[PROFILE_WEAPON], s_r);
			break;
		case EQUIP_ARMOR_BODY:
			inventory_interchange_items((*this)[PROFILE_ARMOR], s_r);
			break;
		case EQUIP_ARMOR_ARM:
			inventory_interchange_items((*this)[PROFILE_GLOVES], s_r);
			break;
		case EQUIP_ARMOR_LEG:
			inventory_interchange_items((*this)[PROFILE_BOOTS], s_r);
			break;
		case EQUIP_STYLE_ACCESSORY:
		{
			if (i_c == style_face)
				inventory_interchange_items((*this)[PROFILE_SKIN_FACE], s_r);
			else if (i_c == style_hair)
				inventory_interchange_items((*this)[PROFILE_SKIN_HEAD], s_r); //???
		}break;
		case EQUIP_STYLE_WEAPON:
			inventory_interchange_items((*this)[PROFILE_SKIN_WEAPON], s_r);
			break;
		case EQUIP_STYLE_BODY:
			inventory_interchange_items((*this)[PROFILE_SKIN_BODY], s_r);
			break;
		case EQUIP_STYLE_BACK:
			inventory_interchange_items((*this)[PROFILE_SKIN_BACK], s_r);
			break;
		case EQUIP_UNDERWEAR:
			inventory_interchange_items((*this)[PROFILE_INNERWARE], s_r);
			break;
		default:
			LeaveCriticalSection(&inv_lock);
			return false;
			break;
		}

		LeaveCriticalSection(&inv_lock);

		player_send_external_change(parent, 1);
		player_recalculate_inventory_stats(parent);
		player_send_stats(parent);
		send();
		return true;
	}
	return false;
}

bool inventory::unequipe_item(slot_id s_id)
{
	EnterCriticalSection(&inv_lock);
	int32 i = get_empty_slot();
	if (i < 0)
	{
		LeaveCriticalSection(&inv_lock);
		message_system_send_simple("@39", parent); // inventory_full
		return false;
	}

	profile_slots[s_id - 1]._item->isBinded = 0x00;
	profile_slots[s_id - 1]._item->binderDBId = 0;

	inventory_interchange_items(profile_slots[s_id - 1], inventory_slots[i]);
	LeaveCriticalSection(&inv_lock);

	player_send_external_change(parent, 1);
	player_recalculate_inventory_stats(parent);
	player_send_stats(parent);
	send();
	return true;
}

void inventory::send(byte show)
{
	std::unique_ptr<Stream> data = std::make_unique<Stream>();
	data->Resize(59);
	data->WriteInt16(0);
	data->WriteInt16(S_INVEN);

	uint16 item_count_pos = data->NextPos();
	data->WriteInt16(0); //next

	data->WriteWorldId(parent);

	data->WriteInt64(gold);

	data->WriteUInt8(show);		//show
	data->WriteUInt8(1);		//update
	data->WriteUInt8(0);		//unk ???

	data->WriteInt32(slot_count);

	data->WriteInt32(itemLevel);
	data->WriteInt32(profileItemLevel);

	data->WriteInt32(0);
	data->WriteInt32(0);
	data->WriteInt32(0);

	data->WriteInt32(330);
	data->WriteInt32(0);

	uint16 next; uint16 count = 0; bool i_t = false; uint16 t_count = 0;
	for (size_t i = 0; i < PROFILE_MAX; i++)
	{
		if (!profile_slots[i].isEmpty)
		{
			//do resize here
			data->WriteInt16(data->_pos);
			next = data->NextPos();
			inventory_write_item(&profile_slots[i], parent->dbid, data.get());
			data->WritePos(next);
			count++;
		}
	}

	if (count) { i_t = true; }
	t_count += count;
	count = 0;
	for (size_t i = 0; i < slot_count; i++)
	{
		if (!inventory_slots[i].isEmpty)
		{
			//do resize here
			data->WriteInt16(data->_pos);
			next = data->NextPos();
			inventory_write_item(&inventory_slots[i], parent->dbid, data.get());
			data->WritePos(next);
			count++;
		}
	}

	if (count) { i_t = true; }
	t_count += count;
	if (i_t) { data->_pos = 4; data->WriteInt16(t_count); data->WriteInt16(59); data->SetEnd(); }

	data->_pos = item_count_pos;
	data->WriteInt16(count);
	data->SetEnd();

	data->WritePos(0);
	connection_send(parent->con, data.get());
}

void WINAPI inventory_write_item(inventory_slot * slot, uint32 db_id, Stream * data)
{
	data->WriteInt16((int16)slot->_item->passivities.size());
	uint16 passivities_next = data->NextPos();
	uint16 unk_next = data->NextPos();

	data->WriteInt32(slot->_item->item_t->id);

	data->WriteInt64(slot->_item->eid); //item entity id
	data->WriteInt64((int64_t)db_id);

	data->WriteInt32(slot->id);
	data->WriteInt32(0); //unk
	data->WriteInt32(slot->_item->stackCount);
	data->WriteInt32(slot->_item->enchantLevel); //unk 30
	data->WriteInt32(0); //unk 30

	data->WriteUInt8(slot->_item->isBinded); //bined?
	data->WriteInt32(slot->_item->crystals[0]);
	data->WriteInt32(slot->_item->crystals[1]);
	data->WriteInt32(slot->_item->crystals[2]);
	data->WriteInt32(slot->_item->crystals[3]);

	data->WriteInt32(0); //apearance changed? itemid
	data->WriteInt32(0); //
	data->WriteInt32(0); //unk
	data->WriteInt32(0); //unk
	data->WriteInt32(0); //unk	 effect?
	data->WriteInt32(0); //unk	 effect?
	data->WriteInt32(0); //unk	 effect?
	data->WriteInt32(0); //unk	 effect?
	data->WriteUInt8(slot->_item->isMasterworked); //_slots[i]->_info->_isMasterworked
	data->WriteUInt8(slot->_item->isEnigmatic == 1 ? 1 : 0);  //enigmatic [blue ?]
	data->WriteUInt8(slot->_item->isEnigmatic == 2 ? 1 : 0); //enigmatic [yellow ?]
	data->WriteUInt8(slot->_item->isEnigmatic == 3 ? 1 : 0);	 //enigmatic [yellow 2 ?]
	data->WriteUInt8(slot->_item->isEnigmatic == 4 ? 1 : 0);	 //enigmatic [yellow 3 ?]
	data->WriteInt32(0); //unk


	data->WriteInt32(0);			//current itemLevel
	data->WriteInt32(0);			//unk itemLevel [additional item level?]
	data->WriteInt32(0);			//maxItemLevel

	data->WriteFloat(0.0f);	  //enchantment advantage [unk formula]
	data->WriteFloat(0.0f);	  //enchantment advantage [unk formula]
	data->WriteFloat(0.0f);	  //enchantment advantage [unk formula]

	data->WriteFloat(0);
	data->WriteInt32(slot->_item->binderDBId); //binder  ?
	data->WriteInt32(0); //disabled  ?
	data->WriteUInt8(slot->_item->isAwakened); // slot->_item->isAwakened
	data->WriteInt32(0); //unk

	data->WriteInt32(0);

	data->WriteInt32(0);

	data->WriteInt16(0); //unk
	data->WritePos(unk_next);
	data->WriteInt16(0);

	for (size_t j = 0; j < slot->_item->passivities.size(); j++)
	{
		data->WritePos(passivities_next);
		data->WriteInt16(data->_pos);
		passivities_next = data->NextPos();

		data->WriteInt32(slot->_item->passivities[j]->id);
	}

	if (slot->_item->passivities.size() == 0)
		data->WriteInt16(0);
}


void inventory::send_item_levels()
{
	//todo
}

std::shared_ptr<item> inventory::get_inventory_item_by_eid(item_eid eid, byte remove /*= 0*/, uint32 stack_count /*= 1*/)
{
	EnterCriticalSection(&inv_lock);

	for (size_t i = 0; i < slot_count; i++)
	{
		if (inventory_slots[i].isEmpty == 0 && inventory_slots[i]._item->eid == eid)
			if (remove && !stack_count)
			{
				std::shared_ptr<item>  toReturn = inventory_slots[i]._item;
				inventory_slots[i]._item = nullptr;
				inventory_slots[i].isEmpty = 1;

				LeaveCriticalSection(&inv_lock);
				return toReturn;
			}
			else
			{
				if (remove && (stack_count > 0 && stack_count <= inventory_slots[i]._item->stackCount))
				{
					std::shared_ptr<item> out = entity_manager::create_item(inventory_slots[i]._item);
					out->stackCount = stack_count;
					inventory_slots[i]._item->stackCount -= stack_count;
					if (inventory_slots[i]._item->stackCount <= 0)
						slot_clear(inventory_slots[i]);

					LeaveCriticalSection(&inv_lock);
					return out;

				}
				else if (!remove)
				{
					LeaveCriticalSection(&inv_lock);
					return inventory_slots[i]._item;
				}
			}

	}

	LeaveCriticalSection(&inv_lock);
	return nullptr;
}

std::shared_ptr<item> inventory::get_inventory_item_by_id(item_id id, byte remove /*= 0*/, uint32 stack_count /*= 1*/)
{
	EnterCriticalSection(&inv_lock);

	for (size_t i = 0; i < slot_count; i++)
	{
		if (inventory_slots[i].isEmpty == 0 && inventory_slots[i]._item->item_t->id == id)
			if (remove && !stack_count)
			{
				std::shared_ptr<item>  toReturn = inventory_slots[i]._item;
				inventory_slots[i]._item = nullptr;
				inventory_slots[i].isEmpty = 1;

				LeaveCriticalSection(&inv_lock);
				return toReturn;
			}
			else
			{
				if (remove && (stack_count > 0 && stack_count <= inventory_slots[i]._item->stackCount))
				{
					std::shared_ptr<item> out = entity_manager::create_item(inventory_slots[i]._item);
					out->stackCount = stack_count;
					inventory_slots[i]._item->stackCount -= stack_count;
					if (inventory_slots[i]._item->stackCount <= 0)
						slot_clear(inventory_slots[i]);

					LeaveCriticalSection(&inv_lock);
					return out;

				}
				else if (!remove)
				{
					LeaveCriticalSection(&inv_lock);
					return inventory_slots[i]._item;
				}
			}

	}

	LeaveCriticalSection(&inv_lock);
	return nullptr;
}

std::shared_ptr<item> inventory::get_item(entityId id)
{
	for (uint8 i = 0; i < 20; i++)
	{
		if (!profile_slots[i].isEmpty && profile_slots[i]._item->eid == id) return profile_slots[i]._item;
	}

	for (uint8 i = 0; i < slot_count; i++)
	{
		if (!inventory_slots[i].isEmpty && inventory_slots[i]._item->eid == id) return inventory_slots[i]._item;
	}
}


//returns the profile_slot indicated by the id
inventory_slot& inventory::operator[](slot_id id)
{
	EnterCriticalSection(&inv_lock);
	if ((id) >= 0 && (id) <= PROFILE_MAX)
	{
		LeaveCriticalSection(&inv_lock);
		return profile_slots[(id)];
	}
	LeaveCriticalSection(&inv_lock);

	return std::move(inventory_slot(0xffff));
}

void inventory::clear()
{
	EnterCriticalSection(&inv_lock);
	for (uint16 i = 0; i < slot_count; i++)
	{
		slot_wipe(inventory_slots[i]);
	}

	for (size_t i = 0; i < PROFILE_MAX; i++)
	{
		slot_wipe(profile_slots[i]);
	}

	profileItemLevel = itemLevel = 0;
	gold = 0;

	LeaveCriticalSection(&inv_lock);
}

bool inventory::is_full()
{
	for (uint16 i = 0; i < slot_count; i++)
		if (inventory_slots[i].isEmpty)
			return false;

	return true;
}

/*return the first empty slot index inside the inventory [no profile slots]*/
int16 inventory::get_empty_slot()
{
	for (uint16 i = 0; i < slot_count; i++)
	{
		if (inventory_slots[i].isEmpty)
			return (int16)i;
	}
	return -1;
}


slot_id inventory::get_profile_item_slot_id_by_eid(item_eid eid)
{
	if (eid <= 0) return 0xffff;
	EnterCriticalSection(&inv_lock);
	for (uint16 i = 0; i < slot_count; i++)
	{
		if (!profile_slots[i].isEmpty && profile_slots[i]._item->eid == eid)
		{
			LeaveCriticalSection(&inv_lock);
			return profile_slots[i].id;
		}
	}

	LeaveCriticalSection(&inv_lock);
	return 0xffff;
}

uint8 inventory::get_profile_item_enchant_level(slot_id id)
{
	return (((id) >= 0) && ((id) < PROFILE_MAX)) ? (profile_slots[id]._item ? profile_slots[id]._item->enchantLevel : 0x00) : 0x00;
}

void inventory::get_profile_passivities(std::vector<const passivity_template*>& out)
{
	EnterCriticalSection(&inv_lock);

	for (uint32 i = 0; i < PROFILE_MAX; i++)
		if (!profile_slots[i].isEmpty)
			iventory_get_item_passivities(profile_slots[i]._item, out);


	LeaveCriticalSection(&inv_lock);
}

void inventory::refresh_enchat_effect()
{
	parent->lock_stats();
	parent->stats.clear_enchat();

	float mod = 0.0f;
	uint32 val = 0;
	for (uint8 i = 0; i < 4; i++)
		if (!profile_slots[i].isEmpty) {
			if (profile_slots[i]._item->enchantLevel && profile_slots[i]._item->item_t->enchant &&
				profile_slots[i]._item->item_t->equipmentData) {
				for (size_t j = 0; j < profile_slots[i]._item->item_t->enchant->stats.size(); j++)
				{
					if (profile_slots[i]._item->item_t->enchant->stats[j].enchantStep > 12)
					{
						if (profile_slots[i]._item->item_t->enchant->stats[j].enchantStep > profile_slots[i]._item->enchantLevel)
							break;

						mod = (profile_slots[i]._item->item_t->enchant->stats[j].rate - ((int)profile_slots[i]._item->item_t->enchant->stats[j].rate));

						switch (profile_slots[i]._item->item_t->enchant->stats[j].kind)
						{
						case attack:
							parent->stats.enchant_attack += (uint32)(profile_slots[i]._item->item_t->equipmentData->maxAttack * mod);
							break;
						case defence:
							parent->stats.enchant_defense += (uint32)(profile_slots[i]._item->item_t->equipmentData->defense * mod);
							break;
						case impact:
							parent->stats.enchant_impact += (uint32)(profile_slots[i]._item->item_t->equipmentData->impact * mod);
							break;
						case balance:
							parent->stats.enchant_balance += (uint32)(profile_slots[i]._item->item_t->equipmentData->balance * mod);
							break;

						}
					}
					else
					{
						mod = profile_slots[i]._item->enchantLevel * (profile_slots[i]._item->item_t->enchant->stats[j].rate -
							((int)profile_slots[i]._item->item_t->enchant->stats[j].rate)); //get only fractional part

						switch (profile_slots[i]._item->item_t->enchant->stats[j].kind)
						{
						case attack:
							parent->stats.enchant_attack += (uint32)(profile_slots[i]._item->item_t->equipmentData->maxAttack * mod);
							break;
						case defence:
							parent->stats.enchant_defense += (uint32)(profile_slots[i]._item->item_t->equipmentData->defense * mod);
							break;
						case impact:
							parent->stats.enchant_impact += (uint32)(profile_slots[i]._item->item_t->equipmentData->impact * mod);
							break;
						case balance:
							parent->stats.enchant_balance += (uint32)(profile_slots[i]._item->item_t->equipmentData->balance * mod);
							break;

						}
					}
				}
			}
		}

	parent->unlock_stats();
}

void inventory::refresh_items_modifiers()
{
	parent->lock_stats();
	parent->stats.claer_modifiers();
	for (uint8 i = 0; i < 4; i++)
		if (!profile_slots[i].isEmpty && profile_slots[i]._item->item_t->equipmentData) {
			parent->stats.attack_modifier += profile_slots[i]._item->item_t->equipmentData->maxAttack;
			parent->stats.defense_modifier += profile_slots[i]._item->item_t->equipmentData->defense;
			parent->stats.impact_modifier += profile_slots[i]._item->item_t->equipmentData->impact;
			parent->stats.balance_modifier += profile_slots[i]._item->item_t->equipmentData->balance;
		}
	parent->unlock_stats();
}

bool inventory::add_gold(uint64 v)
{
	if (gold + v > config::player.max_glod) return false;

	gold += v;
	return true;
}

item_id inventory::get_item(slot_id id)
{
	if ((id - 1) < 0 || (id - 1) >= slot_count)
		return 0;

	if (!inventory_slots[id - 1].isEmpty && inventory_slots[(id - 1)]._item && inventory_slots[id - 1]._item->item_t)
		return inventory_slots[id - 1]._item->item_t->id;
	return 0;
}

bool inventory::move_item(uint32 i_0, uint32 i_1)
{
	EnterCriticalSection(&inv_lock);
	if ((i_0 < 40 || i_0 >= (slot_count + 40)) || (i_1 < 40 || i_1 >= (slot_count + 40))) { LeaveCriticalSection(&inv_lock);  return false; }

	inventory_slot &s_0 = inventory_slots[i_0 - 40];
	inventory_slot &s_1 = inventory_slots[i_1 - 40];

	if (s_0.isEmpty) { LeaveCriticalSection(&inv_lock); printf("INVEN_MOVE S_0[EMPTY]!!!\n"); return false; }
	else if (s_1.isEmpty) { inventory_interchange_items(s_0, s_1); LeaveCriticalSection(&inv_lock); return true; }

	if (s_0._item->item_t->id == s_1._item->item_t->id && (s_1._item->item_t->maxStack - s_1._item->stackCount) > 0)
	{
		printf("TODO INVEN_MOVE_STACK!!!\n");
	}
	else
	{
		inventory_interchange_items(s_0, s_1);
	}
	LeaveCriticalSection(&inv_lock);
	return true;
}

item_id inventory::get_profile_item(slot_id id)
{
	return (((id) >= 0) && ((id) < PROFILE_MAX)) ? (profile_slots[id]._item ? profile_slots[id]._item->item_t->id : 0x00) : 0x00;
}


Stream * inventory::get_raw()
{
	Stream* out = new Stream();
	EnterCriticalSection(&inv_lock);

	out->WriteInt32(0); //next & profile_item_count 
	out->WriteInt32(0); //next & inventory_item_count

	uint16 count = 0;
	out->WritePos(0);
	for (uint32 i = 0; i < PROFILE_MAX; i++)
	{
		if (!profile_slots[i].isEmpty)
		{
			count++;
			out->WriteUInt32(i);
			out->WriteUInt32(profile_slots[i]._item->item_t->id);

			out->WriteUInt8(profile_slots[i]._item->hasCrystals);
			for (byte j = 0; j < profile_slots[i]._item->hasCrystals; j++)
				out->WriteInt32(profile_slots[i]._item->crystals[j]);

			out->WriteUInt8(profile_slots[i]._item->itemLevel);
			out->WriteUInt8(profile_slots[i]._item->isMasterworked);
			out->WriteFloat(profile_slots[i]._item->masterworkRate);
			out->WriteUInt8(profile_slots[i]._item->isAwakened);
			out->WriteUInt8(profile_slots[i]._item->isEnigmatic);
			out->WriteUInt8(profile_slots[i]._item->enchantLevel);
			out->WriteUInt8(profile_slots[i]._item->isBinded);
			out->WriteInt32(profile_slots[i]._item->binderDBId);
			out->WriteUInt8(profile_slots[i]._item->isCrafted);
			out->WriteInt32(profile_slots[i]._item->crafterDBId);
			out->WriteInt32(profile_slots[i]._item->stackCount);

			uint16 passCount = out->NextPos();
			uint16 pCount = 0;
			for (size_t j = 0; j < profile_slots[i]._item->passivities.size(); j++)
			{
				if (profile_slots[i]._item->passivities[j])
				{
					out->WriteInt32(profile_slots[i]._item->passivities[j]->id);
					pCount++;
				}
				else
					out->WriteInt32(0);
			}
			out->_pos = passCount;
			out->WriteInt16(pCount);
			out->SetEnd();
		}
	}

	out->_pos = 2;
	out->WriteInt16(count);
	out->SetEnd();

	count = 0;
	out->WritePos(4);
	for (uint32 i = 0; i < slot_count; i++)
	{
		if (!inventory_slots[i].isEmpty)
		{
			count++;
			out->WriteUInt32(i);
			out->WriteUInt32(inventory_slots[i]._item->item_t->id);

			out->WriteUInt8(inventory_slots[i]._item->hasCrystals);
			for (byte j = 0; j < inventory_slots[i]._item->hasCrystals; j++)
				out->WriteInt32(inventory_slots[i]._item->crystals[j]);

			out->WriteUInt8(inventory_slots[i]._item->itemLevel);
			out->WriteUInt8(inventory_slots[i]._item->isMasterworked);
			out->WriteFloat(inventory_slots[i]._item->masterworkRate);
			out->WriteUInt8(inventory_slots[i]._item->isAwakened);
			out->WriteUInt8(inventory_slots[i]._item->isEnigmatic);
			out->WriteUInt8(inventory_slots[i]._item->enchantLevel);
			out->WriteUInt8(inventory_slots[i]._item->isBinded);
			out->WriteInt32(inventory_slots[i]._item->binderDBId);
			out->WriteUInt8(inventory_slots[i]._item->isCrafted);
			out->WriteInt32(inventory_slots[i]._item->crafterDBId);
			out->WriteInt32(inventory_slots[i]._item->stackCount);

			short passCount = out->NextPos();
			short pCount = 0;
			for (size_t j = 0; j < inventory_slots[i]._item->passivities.size(); j++)
			{
				if (inventory_slots[i]._item->passivities[j])
				{
					out->WriteInt32(inventory_slots[i]._item->passivities[j]->id);
					pCount++;
				}
				else
					out->WriteInt32(0);
			}
			out->_pos = passCount;
			out->WriteInt16(pCount);
			out->SetEnd();


		}
	}

	out->_pos = 6;
	out->WriteInt16(count);
	out->SetEnd();

	LeaveCriticalSection(&inv_lock);

	return out;
}

void inventory::lock()
{
	EnterCriticalSection(&inv_lock);
}

void inventory::unlock()
{
	LeaveCriticalSection(&inv_lock);
}

void inventory::recalculate_levels()
{
	//todo
}



uint32 WINAPI item_stack(std::shared_ptr<item> i, uint32 stack_count, byte mode)
{
	uint32 out = 0;

	if ((stack_count + i->stackCount) > i->item_t->maxStack)
	{
		if (!mode)
		{
			out = (stack_count - i->item_t->maxStack) + i->stackCount;
			i->stackCount = i->item_t->maxStack;
		}
		else
			out = stack_count;
	}
	else
	{
		i->stackCount += stack_count;
	}

	return out;
}

bool WINAPI slot_insert(inventory_slot & j, item_id id, uint32 stack_count){
	if (!j.isEmpty)
		return false;

	if (inventory_build_item(j, id))
	{
		j._item->stackCount = stack_count;
		return true;
	}

	slot_clear(j);
	return false;
}

void WINAPI slot_wipe(inventory_slot &s){
	if (!s.isEmpty) {
		entity_manager::destroy_item(s._item->eid);
		s._item = nullptr;
		s.isEmpty = 1;
	}
}

void WINAPI slot_clear(inventory_slot& s)
{
	s.isEmpty = 1;
	s._item = nullptr;
}



std::shared_ptr<item> WINAPI inventory_create_item(item_id id)
{
	const item_template * t = data_service::data_resolve_item(id);
	if (!t) return nullptr;

	std::shared_ptr<item> i = entity_manager::create_item();

	i->item_t = t;
	i->stackCount = 1;
	i->itemLevel = 1;

	if (t->category == twohand || t->category == axe || t->category == circle || t->category == bow || t->category == staff || t->category == rod || t->category == lance ||
		t->category == dual || t->category == blaster || t->category == gauntlet || t->category == shuriken ||

		t->category == bodyMail || t->category == handMail || t->category == feetMail ||
		t->category == bodyLeather || t->category == handLeather || t->category == feetLeather ||
		t->category == bodyRobe || t->category == handRobe || t->category == feetRobe)
	{
		switch (t->type)
		{

		case EQUIP_WEAPON:
		{



		}break;
		case EQUIP_ARMOR_BODY:
		{



		}break;
		case EQUIP_ARMOR_LEG:
			break;
		case EQUIP_ARMOR_ARM:
			break;
		}
	}
	return i;
}

bool WINAPI inventory_build_item(inventory_slot& slot, item_id id)
{
	if (id <= 0 || !slot.isEmpty)
		return false;

	if (!(slot._item = inventory_create_item(id))) return false;
	slot.isEmpty = 0;

	const item_template * t = slot._item->item_t;
	
	passivity_roll_item(slot._item);

	return true;
}

bool WINAPI inventory_new(std::shared_ptr<player> p)
{
	if (!p) return false;
	e_player_class pClass = p->pClass;
	inventory * inv = &p->i_;


	if (pClass == WARRIOR || pClass == ARCHER || pClass == SLAYER /*|| pClass == REAPER*/)
	{
		inventory_build_item((*inv)[PROFILE_ARMOR], 15004);
		inventory_build_item((*inv)[PROFILE_GLOVES], 15005);
		inventory_build_item((*inv)[PROFILE_BOOTS], 15006);
	}
	else if (pClass == BERSERKER || pClass == LANCER || pClass == ENGINEER || pClass == FIGHTER)
	{
		(inventory_build_item((*inv)[PROFILE_ARMOR], 15001));
		(inventory_build_item((*inv)[PROFILE_GLOVES], 15002));
		(inventory_build_item((*inv)[PROFILE_BOOTS], 15003));
	}
	else if (pClass == SORCERER || pClass == PRIEST || pClass == MYSTIC || pClass == ASSASSIN)
	{
		(inventory_build_item((*inv)[PROFILE_ARMOR], 15007));
		(inventory_build_item((*inv)[PROFILE_GLOVES], 15008));
		(inventory_build_item((*inv)[PROFILE_BOOTS], 15009));
	}

	switch (pClass)
	{
	case WARRIOR:
		(inventory_build_item((*inv)[PROFILE_WEAPON], 10001));
		break;
	case BERSERKER:
		(inventory_build_item((*inv)[PROFILE_WEAPON], 10004));
		break;
	case FIGHTER:
		(inventory_build_item((*inv)[PROFILE_WEAPON], 82005));
		break;
	case ENGINEER:
		(inventory_build_item((*inv)[PROFILE_WEAPON], 55005));
		break;
	case PRIEST:
		(inventory_build_item((*inv)[PROFILE_WEAPON], 10007));
		break;
	case MYSTIC:
		(inventory_build_item((*inv)[PROFILE_WEAPON], 10008));
		break;
	case ASSASSIN:
		(inventory_build_item((*inv)[PROFILE_WEAPON], 58171));
		break;
	case LANCER:
		(inventory_build_item((*inv)[PROFILE_WEAPON], 10002));
		break;
	case SLAYER:
		(inventory_build_item((*inv)[PROFILE_WEAPON], 10003));
		break;
	case SORCERER:
		(inventory_build_item((*inv)[PROFILE_WEAPON], 10005));
		break;
	case ARCHER:
		(inventory_build_item((*inv)[PROFILE_WEAPON], 10006));
		break;
	case REAPER:
		(inventory_build_item((*inv)[PROFILE_WEAPON], 80396));
		break;
	}

	//insert additional items here TODO

	return true;
}

void WINAPI inventory_init(std::shared_ptr<player> p, uint16 slot_count)
{
	for (uint16 i = 0; i < 20; i++)
	{
		p->i_.profile_slots[i].id = i + 1;
	}
	for (uint16 i = 0; i < SC_INVENTORY_MAX_SLOTS; i++)
	{
		p->i_.inventory_slots[i].id = i + 40;
	}

	if (slot_count > SC_INVENTORY_MAX_SLOTS) slot_count = SC_INVENTORY_MAX_SLOTS;
	if (slot_count < SC_INVENTORY_MIN_SLOTS) slot_count = SC_INVENTORY_MIN_SLOTS;

	p->i_.slot_count = slot_count;
	p->i_.parent = std::move(p);
	return;
}

void WINAPI iventory_get_item_passivities(std::shared_ptr<item> it, std::vector<const passivity_template*>& out)
{
	for (size_t i = 0; i < it->passivities.size(); i++) out.push_back(it->passivities[i]);

	if (it->isAwakened || it->isMasterworked)
	{
		for (size_t i = 0; i < it->item_t->masterpiecePassivities.size(); i++) out.push_back(it->item_t->masterpiecePassivities[i]);
	}
	else
	{
		for (size_t i = 0; i < it->item_t->passivities.size(); i++) out.push_back(it->item_t->passivities[i]);
	}

	if (it->item_t->equipmentData)
		out.push_back(it->item_t->equipmentData->passivityG);
}

void WINAPI send_item_tooltip(p_ptr p, entityId eid, uint32 t){
	std::shared_ptr<item> i = entity_manager::get_item(eid);
	if (!i) return;
	send_item_tooltip(p, i, t);
	return;
}

void WINAPI send_item_tooltip(p_ptr p, std::shared_ptr<item> i, uint32 t){
	std::unique_ptr<Stream> data = std::make_unique<Stream>();
	data->Clear();
	data->WriteInt16(0);
	data->WriteInt16(S_SHOW_ITEM_TOOLTIP);

	data->WriteInt16(i->hasCrystals);
	uint16 crystalsOffset = data->NextPos();

	data->WriteInt16(i->passivities.size());
	uint16 passivitiesOffset = data->NextPos();

	uint16 crafterOffset = data->NextPos();
	uint16 soulbindOffset = data->NextPos();

	data->WriteInt32(t);
	data->WriteInt64(i->eid);
	data->WriteInt32(i->item_t->id);
	data->WriteInt64(i->eid);

	data->WriteInt32(0);//owner id?
	data->WriteInt32(23);
	data->WriteInt32(0);//slot id

	data->WriteInt32(9); //unk
	data->WriteInt32(23); //unk
	data->WriteInt32(i->stackCount); //unk
	data->WriteInt32(i->enchantLevel); //enchant

	data->WriteInt32(1); //unk

	data->WriteUInt8(i->isBinded ? 1 : 0);

	data->WriteUInt8(1);
	data->WriteUInt8(1);
	data->WriteUInt8(1);
	data->WriteInt32(3434); //unk
	data->WriteFloat(23); //unk

	data->WriteInt32(12);
	data->WriteInt32(13);
	data->WriteInt32(14);
	data->WriteInt32(15);
	data->WriteInt32(16);
	data->WriteInt32(17);
	data->WriteInt32(18);
	data->WriteInt32(19);
	data->WriteInt32(10);

	data->WriteInt32(1);
	data->WriteInt32(1);
	data->WriteInt32(1);
	data->WriteInt32(1);
	data->WriteInt32(1);
	data->WriteInt32(1);
	data->WriteInt32(1);
	data->WriteInt32(1);
	data->WriteInt32(1);
	data->WriteInt32(1);
	data->WriteInt32(1);
	data->WriteInt32(1);
	data->WriteInt32(1);

	data->WriteInt32(1);
	data->WriteInt32(1);

	data->WriteInt32(5511);
	data->WriteInt32(5511);
	data->WriteInt32(5511);
	data->WriteInt32(5511);
	data->WriteInt32(5511);
	data->WriteInt32(5511);//31




	data->WriteUInt8(3);

	data->WriteInt64(123123);			 //0xFEFEFEFEFEFEFEFE
	data->WriteInt64(123123);			 //0xFEFEFEFEFEFEFEFE
	data->WriteInt64(123123);			 //0xFEFEFEFEFEFEFEFE
	data->WriteInt64(123123);			 //0xFEFEFEFEFEFEFEFE
	data->WriteInt64(123123);			 //0xFEFEFEFEFEFEFEFE
	data->WriteInt64(123123);			 //0xFEFEFEFEFEFEFEFE
	data->WriteInt64(123123);			 //0xFEFEFEFEFEFEFEFE
	data->WriteInt64(123123);			 //0xFEFEFEFEFEFEFEFE


	data->WriteUInt8(i->isEnigmatic); //enigmatic 1
	data->WriteUInt8(0); //enigmatic 2
	data->WriteUInt8(0); //enigmatic 3
	data->WriteUInt8(0); //enigmatic 4
	data->WriteUInt8(i->isMasterworked); //masterworked?
	data->WriteUInt8(1); //masterworked?
	data->WriteUInt8(1); //masterworked?
	data->WriteUInt8(1); //masterworked?
	data->WriteUInt8(1); //masterworked?
	data->WriteUInt8(0); //comapre stats

	data->WriteInt32(0);     //attack base range 2
	data->WriteInt32(0);	    //total when equiped base defense
	data->WriteInt32(0);	    //total when equiped base impact
	data->WriteInt32(0);	    //total when equiped base balance

	data->WriteFloat(0);	    //total when equiped base crifactor
	data->WriteFloat(0);	    //total when equiped base crit resist factor
	data->WriteFloat(0);	    //total when equiped base crit power

	data->WriteInt32(0);	    //total when equiped base impact factor
	data->WriteInt32(0);	    //total when equiped base balance factor
	data->WriteInt32(0);	    //total when equiped base attackSpeed
	data->WriteInt32(0);	    //total when equiped base movementSpeed

	data->WriteFloat(0);	    //total when equiped base weakening effect (green)
	data->WriteFloat(0);	    //total when equiped base periodic damage (purple)
	data->WriteFloat(0);	    //total when equiped base stun resist

	data->WriteInt32(0);  //attack add range 2
	data->WriteInt32(0);   //additional defense
	data->WriteInt32(0);   //additional impact
	data->WriteInt32(0);   //additional balance

	data->WriteFloat(0);	  //additional crifactor
	data->WriteFloat(0);	  //additional crit resist factor
	data->WriteFloat(0);	  //additional crit power 

	data->WriteInt32(0);	  //additional impact factor
	data->WriteInt32(0);	  //additional balance factor
	data->WriteInt32(0);	  //additional attack speed
	data->WriteInt32(0);	  //additional movement speed

	data->WriteFloat(0);	  //additional weakening (green)
	data->WriteFloat(0);	  //additional periodic (purple)
	data->WriteFloat(0);	  //additional stun

	data->WriteInt32(0); //attack base range 1
	data->WriteInt32(0); //attack additional range 1

	data->WriteInt32(0);
	data->WriteInt32(0); //add ilv + base ilv??

	data->WriteUInt8(0);
	data->WriteUInt8(0);
	data->WriteUInt8(0); //appearance changed
	data->WriteUInt8(0);

	data->WriteInt64(0);
	data->WriteInt32(0);
	data->WriteInt32(0);
	data->WriteInt32(0);
	data->WriteInt32(0);
	data->WriteInt32(0); //current ilv value item->_itemLevel

	data->WriteInt32(0);  //ilv? item->_itemLevel
						  //ilv?
	data->WriteInt32(0); //max ilv value item->_item->_itemLevel
	data->WriteInt32(0);
	data->WriteInt32(0);




	data->WriteInt32(0);
	data->WriteInt32(0);
	data->WriteInt32(0);
	data->WriteInt32(0);
	data->WriteInt32(0);
	data->WriteInt32(0);
	data->WriteInt64(0);
	data->WriteInt32(0);
	data->WriteInt32(0);
	data->WriteInt32(2890);//feedstock count

	data->WriteInt32(0);
	data->WriteInt32(0);
	data->WriteInt32(0);

	data->WriteInt32(0);
	data->WriteInt32(0);
	data->WriteInt16(0);

	data->WritePos(crafterOffset);
	data->WriteInt16(0);

	data->WritePos(soulbindOffset);
	data->WriteInt16(0);

	for (uint8 j = 0; j < i->hasCrystals; j++)
	{
		data->WritePos(crystalsOffset);
		data->WriteInt16(data->_pos);
		crystalsOffset = data->NextPos();
		data->WriteInt32(i->crystals[j]);
	}

	for (size_t j = 0; j < i->passivities.size(); j++)
	{
		data->WritePos(passivitiesOffset);
		data->WriteInt16(data->_pos);
		passivitiesOffset = data->NextPos();

		data->WriteInt32(i->passivities[j]->id);
	}

	data->WritePos(0);

	connection_send(p->con, data.get());
	return;
}

void WINAPI inventory_interchange_items(inventory_slot &s1, inventory_slot& s2)
{

	if (!s1.isEmpty) {
		s1._item->isBinded = 0x00;
		s1._item->binderDBId = 0;
	}

	byte temp_empty_flag = s1.isEmpty;
	s1.isEmpty = s2.isEmpty;
	s2.isEmpty = temp_empty_flag;

	std::shared_ptr<item> temp_item = std::move(s1._item);
	s1._item = std::move(s2._item);
	s2._item = std::move(temp_item);
}



inventory_slot::inventory_slot(inventory_slot &i) :id(i.id), isEmpty(i.isEmpty), _item(i._item) {}
inventory_slot::inventory_slot() : id(0), isEmpty(1), _item(nullptr) {}
inventory_slot::inventory_slot(slot_id i) : id(i), isEmpty(1), _item(nullptr) {}

void inventory_slot::operator=(inventory_slot &)
{
}


item::item(entityId id) : entity(id), stackCount(1), binderDBId(0), crafterDBId(0), hasCrystals(0), isBinded(0), isAwakened(0), isMasterworked(0), isEnigmatic(0),
enchantLevel(0), itemLevel(0), isCrafted(0), masterworkRate(0), item_t(0) {}

