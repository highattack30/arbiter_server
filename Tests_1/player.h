#ifndef PLAYER_H
#define PLAYER_H

#include "serverEnums.h"
#include "typeDefs.h"
#include "playerEnums.h"
#include "inventory.h"
#include "worldposition.h"
#include "entity.h"
#include "p_spawn.h"
#include "skylake_stats.h"
#include "contract_manager.h"

#include "MySqlDriver.h"
#include "win32.h"


class connection;
class player : public entity
{
private:
	double	three_regen, five_regen, special_regen;
	uint32 hp_diff, mp_diff;
	Stream data;
public:
	world_position					position;
	std::shared_ptr<connection>		con;

	p_spawn							spawn;
	std::atomic_uint8_t				p_c_s;		//player current section
	std::atomic_uint8_t				p_c_z;		//player current zone
	std::atomic_uint8_t				p_c_a;		//player current area
	std::atomic_uint8_t				p_c_c_id;	//player current contientn id
	std::atomic_uint8_t				channel;
	p_stats							stats;
	p_skills						skills;
	inventory						i_;

	std::atomic_uint8_t				status;

	std::atomic_uint64_t
		exp,
		restedExp;
	contract_manager				c_manager;

	uint16							level;
	uint32							model;
	e_player_class                  pClass;
	e_player_race					pRace;
	e_player_gender					pGender;

	byte							details1[SC_PLAYER_DETAILS_1_BUFFER_SIZE];
	byte							details2[SC_PLAYER_DETAILS_2_BUFFER_SIZE];
	byte							details3[SC_PLAYER_DETAILS_3_BUFFER_SIZE];

	uint32							dbid;
	char							name[SC_PLAYER_NAME_MAX_LENGTH];

	void							update(double dt, double elapse);
	void							lock_stats();
	void							unlock_stats();
	void							save(sql::Connection*);

	player(entityId, uint32 db_id, std::shared_ptr<connection>);
	~player();

private:
	CRITICAL_SECTION					stats_lock;
};

uint32 WINAPI				player_calculate_model(e_player_class, e_player_gender, e_player_race);
void WINAPI					player_load_user_settings(std::shared_ptr<player>, sql::Connection*);
bool WINAPI					player_send_external_change(std::shared_ptr<player>, byte broadcast = 0x00);
void WINAPI					player_write_spawn_packet(std::shared_ptr<player>, Stream&);
void WINAPI					player_send_stats(p_ptr);
void WINAPI					player_recalculate_inventory_stats(p_ptr);
void WINAPI					player_recalculate_stats(p_ptr);

e_player_class WINAPI		player_get_class(uint32 model);
e_player_race WINAPI		player_get_race(uint32 model);
e_player_gender WINAPI		player_get_gender(uint32 model);


#endif
