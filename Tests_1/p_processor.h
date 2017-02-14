#ifndef PASSIVITY_PROCESSOR_H
#define PASSIVITY_PROCESSOR_H

#include "typeDefs.h"
#include "win32.h"
#include "passivityEnums.h"

struct item;
struct passivity_template;
struct passivity_category;
typedef void(WINAPI *p_fun_ptr)(p_ptr, const passivity_template*);

static p_fun_ptr p_funcs[P_TYPE_MAX];

void WINAPI passivity_processor_init();
void WINAPI passivity_proces(p_ptr, const passivity_template*);

void WINAPI passivity_roll_item(std::shared_ptr<item>);
const passivity_template* WINAPI passivity_category_get_random(passivity_category*);


static void WINAPI p_incrase_max_mp(p_ptr, const passivity_template*);
static void WINAPI p_incrase_max_hp(p_ptr, const passivity_template*);
static void WINAPI p_incrase_power(p_ptr, const passivity_template*);
static void WINAPI p_incrase_endurance(p_ptr, const passivity_template*);
static void WINAPI p_incrase_movement_speed(p_ptr, const passivity_template*);
static void WINAPI p_incrase_crit_factor(p_ptr, const passivity_template*);
static void WINAPI p_incrase_damage(p_ptr, const passivity_template*);
static void WINAPI p_incrase_impact_factor(p_ptr, const passivity_template*);
static void WINAPI p_incrase_balance_factor(p_ptr, const passivity_template*);
static void WINAPI p_incrase_weakening_rate(p_ptr, const passivity_template*);
static void WINAPI p_incrase_poison_rate(p_ptr, const passivity_template*);
static void WINAPI p_incrase_stun_rate(p_ptr, const passivity_template*);
static void WINAPI p_incrase_weakening_resist(p_ptr, const passivity_template*);
static void WINAPI p_incrase_poison_resist(p_ptr, const passivity_template*);
static void WINAPI p_incrase_stun_resist(p_ptr, const passivity_template*);
static void WINAPI p_incrase_crit_power(p_ptr, const passivity_template*);
static void WINAPI p_skill_less_aggro(p_ptr, const passivity_template*);
static void WINAPI p_incrase_mp_hit_regen(p_ptr, const passivity_template*);
static void WINAPI p_incrase_attack_speed(p_ptr, const passivity_template*);
static void WINAPI p_incrase_five_hp_regen(p_ptr, const passivity_template*);
static void WINAPI p_incrase_five_mp_regen(p_ptr, const passivity_template*);
static void WINAPI p_incrase_gather_skill(p_ptr, const passivity_template*);
static void WINAPI p_incrase_harvest_skill(p_ptr, const passivity_template*);
static void WINAPI p_incrase_gather_skill(p_ptr, const passivity_template*);
static void WINAPI p_incrase_attack_speed_decrease_cooldown(p_ptr, const passivity_template*);
static void WINAPI p_incrase_knockdown_resistance_while_skill(p_ptr, const passivity_template*);
static void WINAPI p_incrase_damage_reflect(p_ptr, const passivity_template*);
static void WINAPI p_incrase_crafting_speed(p_ptr, const passivity_template*);
static void WINAPI p_incrase_chance_regen_mp_combat_start(p_ptr, const passivity_template*);
static void WINAPI p_decrese_stun_duration(p_ptr, const passivity_template*);
static void WINAPI p_incrase_pvp_damage(p_ptr, const passivity_template*);
static void WINAPI p_incrase_pvp_defense(p_ptr, const passivity_template*);
static void WINAPI p_decrese_damage(p_ptr, const passivity_template*);

#endif

