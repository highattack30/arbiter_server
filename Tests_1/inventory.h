#ifndef INVENTORY_H
#define INVENTORY_H

#include "typeDefs.h"
#include "win32.h"
#include "serverEnums.h"
#include "itemEnums.h"

#include "entity.h"

#include <vector>
#include <memory>
#include <atomic>

class connection;
class player;
class Stream;

struct item_template;
struct passivity_template;

struct item : public entity
{
	item(entityId);

	int32
		stackCount;
	uint32
		crystals[4],
		binderDBId,
		crafterDBId;


	byte
		hasCrystals,
		isBinded,
		enchantLevel,
		isMasterworked,
		isEnigmatic,
		isAwakened,
		itemLevel,
		isCrafted;

	float
		masterworkRate;
	std::vector<const passivity_template *> passivities;


	const item_template* item_t;
};

struct inventory_slot
{
	inventory_slot();
	inventory_slot(slot_id);
	inventory_slot(inventory_slot&);
	slot_id					id;
	byte					isEmpty;
	std::shared_ptr<item>	_item;

	void operator=(inventory_slot&);
};

class inventory
{
public:
	inventory();
	~inventory();

	bool					insert_or_stack_item(item_id, uint32 stack_count);
	bool					insert_or_stack_item(std::shared_ptr<item>);

	uint32					pull_item_stack(item_id, uint32 stack_count);

	bool					equipe_item(slot_id);
	bool					unequipe_item(slot_id);

	void					send(byte show = 0);
	void					send_item_levels();

	std::shared_ptr<item>	get_inventory_item_by_eid(item_eid, byte remove = 0, uint32 stack_count = 0);
	std::shared_ptr<item>	get_inventory_item_by_id(item_id, byte remove = 0, uint32 stack_count = 0);

	std::shared_ptr<item>   get_item(entityId);

	inventory_slot&			operator[](slot_id);

	void					clear();
	bool					is_full();


	int16					get_empty_slot();
	item_id					get_item(slot_id id);

	bool					move_item(uint32 s_0, uint32 s_1);

	item_id					get_profile_item(slot_id id);
	slot_id					get_profile_item_slot_id_by_eid(item_eid);
	uint8					get_profile_item_enchant_level(slot_id id);
	void					get_profile_passivities(std::vector<const passivity_template*> &);
	void					refresh_enchat_effect();
	void					refresh_items_modifiers();
	bool					add_gold(uint64);

	Stream *				get_raw();

	uint16					slot_count;

	uint32
		profileItemLevel,
		itemLevel;

	uint64					gold;

	inventory_slot			profile_slots[PROFILE_MAX];
	inventory_slot			inventory_slots[SC_INVENTORY_MAX_SLOTS];

	std::shared_ptr<player> parent;
	void							lock();
	void							unlock();
private:

	void recalculate_levels();
	CRITICAL_SECTION		inv_lock;
};



uint32 WINAPI item_stack(std::shared_ptr<item> i, uint32 stack_count, byte mode);
std::shared_ptr<item>  WINAPIinventory_create_item(item_id);
bool WINAPI inventory_build_item(inventory_slot& slot, item_id);
bool WINAPI inventory_new(std::shared_ptr<player> p);
void WINAPI inventory_init(std::shared_ptr<player>, uint16);
void WINAPI iventory_get_item_passivities(std::shared_ptr<item>, std::vector<const passivity_template*> &);

bool WINAPI slot_insert(inventory_slot &, item_id, uint32 stack_count);
void WINAPI slot_wipe(inventory_slot &);
void WINAPI slot_clear(inventory_slot&);

void WINAPI send_item_tooltip(p_ptr, entityId, uint32);
void WINAPI send_item_tooltip(p_ptr, std::shared_ptr<item>, uint32 type = 20);
void WINAPI inventory_interchange_items(inventory_slot &s1, inventory_slot& s2);
static void WINAPI inventory_write_item(inventory_slot *s1, uint32, Stream *);
#endif



