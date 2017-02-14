#ifndef ENCHANT_CONTRACT_H
#define ENCHANT_CONTRACT_H

#include "contract.h"

struct item;
struct enchant_contract : public contract
{
	struct enchant_action{
		enchant_action();

		item_eid	i_eid;

		item_eid	material_eid_1;
		uint32		m_1_count;
		item_eid	material_eid_2;
		uint32		m_2_count;
	};

	enchant_contract(const uint32 id, byte type = 0);

	virtual bool internal_init() override;

	void try_enchant();
	void set_byte(byte type);
	void cancel_temper();

	void cancel();

	bool add_item_to_enchant(int32 unk1, item_eid i_eid, int32 unk2, int32 unk3);
	bool add_material_to_enchant_process(item_eid i_eid, int32 count, uint32 slot);

	std::shared_ptr<item> i_;

private:
	bool enchant_write_materials();

	byte type_unk;
	std::unique_ptr<enchant_action> action;
};

#endif
