#ifndef SKYLAKE_PLAYER_STATS_H
#define SKYLAKE_PLAYER_STATS_H

#include "typeDefs.h"
#include "win32.h"

struct p_stats
{
	std::atomic_uint32_t
		current_hp,
		current_mp,
		current_special;

	uint32

		//attack_defense_impact_balance 8
		base_attack,
		base_defense,
		base_impact,
		base_balance,

		bonus_attack,
		bonus_defense,
		bonus_impact,
		bonus_balance,

		enchant_attack,
		enchant_defense,
		enchant_impact,
		enchant_balance,

		attack_modifier,
		defense_modifier,
		balance_modifier,
		impact_modifier,
		//---------------

		//pvp_attack_defense 40
		base_pvp_attack,
		base_pvp_defense,

		bonus_pvp_attack,
		bonus_pvp_defense,
		//---------------------

		//regens 56
		base_five_hp_regen,
		base_five_mp_regen,

		base_three_hp_regen,
		base_three_mp_regen,

		bonus_five_hp_regen,
		bonus_five_mp_regen,

		bonus_three_hp_regen,
		bonus_three_mp_regen,
		//---------------

		//hp_mp 88
		base_max_hp,
		base_max_mp,

		bonus_hp,
		bonus_mp,
		//-------------

		//power_endurance
		base_power,
		base_endurance,

		bonus_power,
		bonus_endurance,
		//--------

		//impact_balance_factor
		base_impact_factor,
		base_balance_factor,

		bonus_balance_factor,
		bonus_impact_factor,
		//---------

		//attack_run_movement_speed
		base_attack_speed,
		base_run_speed,
		base_movement_speed,

		bonus_attack_speed,
		bonus_run_speed,
		bonus_movement_speed,
		//-----------



		//gather_mine_harvest_speed
		base_gather_speed,
		base_mine_speed,
		base_harvest_speed,
		base_craft_speed,

		//todo

		//-------


		//special
		base_special,
		bonus_special,

		base_special_regen,
		bonus_special_regen,

		base_special_regen_rate,
		bonus_special_regen_rate,
		//-----------

		infamy;

	float
		//crit_rate_resist_power
		base_crit_rate,
		base_crit_resist,
		base_crit_power,

		bonus_crit_rate,
		bonus_crit_resist,
		bonus_crit_power,
		//

		//stun_poison_weakening_rate
		base_stun_rate,
		base_poison_rate,
		base_weakening_rate,

		bonus_stun_rate,
		bonus_poison_rate,
		bonus_weakening_rate,
		//-------------


		//stun_poison_weakening_resist
		base_weakening_resist,
		base_poison_resist,
		base_stun_resist,

		bonus_weakening_resist,
		bonus_poison_resist,
		bonus_stun_resist;
	//------------




	uint32 get_attack() const;
	uint32 get_defense()const;
	uint32 get_balance() const;
	uint32 get_impact() const;

	uint32 get_max_hp() const;
	uint32 get_max_mp() const;

	uint32 get_five_hp_regen() const;
	uint32 get_three_hp_regen() const;
	uint32 get_five_mp_regen() const;
	uint32 get_three_mp_regen() const;

	uint32 get_power() const;
	uint32 get_endurance() const;
	uint32 get_balance_factor() const;
	uint32 get_impact_factor() const;
	uint32 get_attack_speed() const;
	uint32 get_movement_speed(uint32) const;

	uint32 get_pvp_attack() const;
	uint32 get_pvp_defense() const;

	uint32 get_special() const;
	uint32 get_special_regen() const;
	uint32 get_special_regen_rate() const;


	float get_crit_rate() const;
	float get_crit_resist() const;
	float get_crit_power() const;
	float get_weakening_resist() const;
	float get_poison_resist() const;
	float get_stun_resist() const;
	float get_weakening_rate() const;
	float get_poison_rate() const;
	float get_stun_rate() const;



	void clear_enchat();
	void claer_modifiers();
	void clear_base();
	void clear_bonus();

	bool has_special() const;
};

struct p_skills
{
	std::vector<uint32> active;
	std::vector<uint32> passive;
	std::vector<uint32> get_skills() const;
};


struct s_stats
{
	struct stats {
		uint32
			hp,
			mp,
			power,
			endurance,
			impact,
			balance,
			attack_speed,
			run_speed,
			movement_speed,
			attack,
			defense,
			pvp_attack,
			pvp_defense,
			special,
			three_hp_regen,
			five_hp_regen,
			three_mp_regen,
			five_mp_regen,
			special_regen,
			special_regen_rate,
			gather_speed,
			mine_speed,
			harvest_speed,
			craft_speed;

		float
			crit_rate,
			crit_resist,
			crit_power,
			stun_rate,
			poison_rate,
			weakening_rate,
			weakening_resist,
			poison_resist,
			stun_resist;
	};

	struct skill
	{
		std::vector<uint32> active;
		std::vector<uint32> passive;
	};

	struct c_progress
	{
		uint32 level;
		stats class_progress_stats[13];
	};

	struct r_progress
	{
		uint32 level;
		stats race_progress_stats[8];
	};

	stats class_base_stats[13];
	stats race_base_stats[8];
	skill class_base_skills[13];
	skill race_base_skills[8];

	std::vector<r_progress> race_progress;
	std::vector<c_progress> class_progress;

} static skylake_stats;

struct e_stats
{
	struct item_e
	{
		item_e();
		uint32 id;
		uint32 id_max;
		uint32 rate;
		bool is_range;
	};

	//------------------ENCHANT-----------------
	uint32
			enchant_rates[15],
			enchant_pool_range;
	double	feedstock_multiplier;
	std::vector<item_e> enchant_material_rates;

	//------------------ENIGMATIC-----------------
	uint32
		mPullRange,
		sp_cost;
	std::vector<uint32> spellbind_ids;
	std::vector<item_e> enigmatic_material_rates;

	//------------------AWAKEN-----------------
	std::vector<item_e> awaken_material_cost;

}static skylake_e_stats;


bool WINAPI e_stats_calculate_enchant(uint32 target_enchant_level, uint32 material_item_id, uint32 feed_stock_count);
bool WINAPI e_can_use_spellbind(uint32);
uint32 WINAPI e_get_spellbind_id(p_ptr);
uint32 WINAPI e_get_sp_cost();
uint32 WINAPI e_get_awaken_material_cost(uint32);
uint32 WINAPI e_get_scroll_rate(uint32);
uint32 WINAPI e_get_masterworked(uint32);

void WINAPI s_stats_get_progress(p_ptr);
void WINAPI s_stats_get_base_stats(p_ptr);
void WINAPI s_stats_init_player(p_ptr);
void WINAPI s_stats_process_progress(p_ptr, uint16);
void WINAPI s_skills_get_base_skills(p_ptr);
static void WINAPI s_stats_add_to_base(s_stats::stats&from, p_stats& to);
static void WINAPI s_skills_add_to_base(s_stats::skill&from, p_skills& to);

bool WINAPI s_stats_load_scripts();
#endif
