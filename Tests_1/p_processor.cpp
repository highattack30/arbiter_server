#include "p_processor.h"
#include "player.h"
#include "passivitytemplate.h"
#include "inventory.h"
#include "itemtemplate.h"
#include "enchantdatatemplate.h"
#include "equipmentdatatemplate.h"
#include "passivitycategorytemplate.h"

#include <random>

void WINAPI passivity_processor_init() {
	p_funcs[INCREASE_MAX_HP] = p_incrase_max_hp;
	p_funcs[INCREASE_MAX_MP] = p_incrase_max_mp;
	p_funcs[INCREASE_POWER] = p_incrase_power;
	p_funcs[INCREASE_ENDURANCE] = p_incrase_endurance;
	p_funcs[INCREASE_MOVEMENT_SPEED] = p_incrase_movement_speed;
	p_funcs[INCREASE_CRIT_FACTOR] = p_incrase_crit_factor;
	p_funcs[INCREASE_DAMAGE_BY] = p_incrase_damage;
	p_funcs[INCREASE_IMPACT_FACTOR] = p_incrase_impact_factor;
	p_funcs[INCREASE_BALANCE_FACOTR] = p_incrase_balance_factor;
	p_funcs[INCREASE_WEAKENING_APPLY_RATE] = p_incrase_weakening_rate;
	p_funcs[INCREASE_PERIODIC_APPLY_RATE] = p_incrase_poison_rate;
	p_funcs[INCREASE_STUN_APPLY_RATE] = p_incrase_stun_rate;
	p_funcs[INCREASE_WEAKENING_RESISTANCE] = p_incrase_weakening_resist;
	p_funcs[INCREASE_PERIODIC_RESISTANCE] = p_incrase_poison_resist;
	p_funcs[INCREASE_STUN_RESISTANCE] = p_incrase_stun_resist;
	p_funcs[INCREASE_CRIT_POWER] = p_incrase_crit_power;
	p_funcs[SKILLS_PRODUCES_LESS_AGGRO] = p_skill_less_aggro;
	p_funcs[INCREASE_MP_REPLENISHMENT] = p_incrase_mp_hit_regen;
	p_funcs[INCREASE_ATTACK_SPEED] = p_incrase_attack_speed;
	p_funcs[REOVERS_HP_EVERY_5_SECONDS] = p_incrase_five_hp_regen;
	p_funcs[REOVERS_MP_EVERY_5_SECONDS] = p_incrase_five_mp_regen;
	p_funcs[INCREASE_GATHERING_SKILL] = p_incrase_gather_skill;
	p_funcs[INCREASE_PLANT_HARVEST_SPEED] = p_incrase_harvest_skill;
	p_funcs[INCREASE_GATHERING_SPEED_ARUN] = p_incrase_gather_skill;
	p_funcs[INCREASE_ATTACK_SPEED_DECREASE_SKILL_COOLDOWN] = p_incrase_attack_speed_decrease_cooldown;
	p_funcs[INCREASE_KNOCKDOWN_RESISTANCE_WHILE_SKILL] = p_incrase_knockdown_resistance_while_skill;
	p_funcs[REFLECT_DAMAGE_TO_ATTACKER] = p_incrase_damage_reflect;
	p_funcs[INCREASE_CRAFTING_SPEED] = p_incrase_crafting_speed;
	p_funcs[CHANCE_REPLENISH_MP_COMBAT_ENTER] = p_incrase_chance_regen_mp_combat_start;
	p_funcs[DECREASE_DURATION_OF_STUN] = p_decrese_stun_duration;
	p_funcs[INCREASE_PVP_DAMAGE] = p_incrase_pvp_damage;
	p_funcs[DECREASE_PVP_DAMAGE] = p_incrase_pvp_defense;
	p_funcs[DECREASE_DAMAGE_TAKEN] = p_decrese_damage;

	return;
}

void WINAPI passivity_proces(p_ptr p, const passivity_template *t) {
	if (!t || t->type >= P_TYPE_MAX) return;
	if (p_funcs[t->type])p_funcs[t->type](p, t);
	return;
}




void WINAPI passivity_roll_item(std::shared_ptr<item> i) {
	i->passivities.clear();

	const passivity_template * temp = nullptr;

	if (i->item_t->passivityCategory) {
		if ((temp = passivity_category_get_random(i->item_t->passivityCategory)))
			i->passivities.push_back(temp);
	}

	if (i->item_t->enchant && i->item_t->masterpieceEnchant) {
		if (i->isAwakened) {
			for (size_t j = 0; j < i->item_t->masterpieceEnchant->effects.size(); j++) {
					if ((temp = passivity_category_get_random(i->item_t->masterpieceEnchant->effects[j].passivitiesCategory)))
						i->passivities.push_back(temp);
			}
		}
		else if (i->isMasterworked) {
			for (size_t j = 0; j < i->item_t->masterpieceEnchant->effects.size(); j++) {
				if (i->item_t->masterpieceEnchant->effects[j].step <= 12)
					if ((temp = passivity_category_get_random(i->item_t->masterpieceEnchant->effects[j].passivitiesCategory)))
						i->passivities.push_back(temp);
			}
		}
		else {
			for (size_t j = 0; j < i->item_t->enchant->effects.size(); j++) {
				if ((temp = passivity_category_get_random(i->item_t->enchant->effects[j].passivitiesCategory)))
					i->passivities.push_back(temp);
			}
		}
	}

	
	if (i->isMasterworked) {
		for (size_t j = 0; j < i->item_t->masterpiecePassivities.size(); j++)
			i->passivities.push_back(i->item_t->masterpiecePassivities[j]);
	}
	else {
		for (size_t j = 0; j < i->item_t->passivities.size(); j++)
			i->passivities.push_back(i->item_t->passivities[j]);
	}

	return;
}

const passivity_template * WINAPI passivity_category_get_random(passivity_category * c)
{
	if (c->passivities.size() > 1) {
		uint32 rand_seed = (uint32)(rand() % (c->passivities.size() - 1));
		return c->passivities[rand_seed];
	}
	else if (c->passivities.size() == 1) {
		return c->passivities[0];
	}
	return nullptr;
}




void WINAPI p_incrase_max_mp(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_max_hp(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_power(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_endurance(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_movement_speed(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_crit_factor(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_damage(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_impact_factor(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_balance_factor(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_weakening_rate(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_poison_rate(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_stun_rate(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_weakening_resist(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_poison_resist(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI  p_incrase_stun_resist(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_crit_power(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_skill_less_aggro(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_mp_hit_regen(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_attack_speed(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_five_hp_regen(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_five_mp_regen(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_gather_skill(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_attack_speed_decrease_cooldown(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_knockdown_resistance_while_skill(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_damage_reflect(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_crafting_speed(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_chance_regen_mp_combat_start(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_decrese_stun_duration(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_pvp_damage(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_pvp_defense(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_decrese_damage(p_ptr, const passivity_template *)
{
	return;
}

void WINAPI p_incrase_harvest_skill(p_ptr, const passivity_template *)
{
	return;
}
