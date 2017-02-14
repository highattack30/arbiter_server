#include "player.h"
#include "connexion.h"
#include "account.h"
#include "Stream.h"
#include "itemEnums.h"
#include "passivitytemplate.h"
#include "passivityEnums.h"
#include "servertime.h"
#include <sstream>

#include "p_processor.h"

void player::update(double dt, double elapsed)
{
	hp_diff = mp_diff = 0.0;
	if (elapsed - three_regen > 3.0f) {

		if (stats.get_three_hp_regen() > 0) {
			if (a_load(stats.current_hp) < stats.get_max_hp()) {
				stats.current_hp += stats.get_three_hp_regen();
				hp_diff = stats.get_three_hp_regen();
				if (a_load(stats.current_hp) > stats.get_max_hp())
					a_store(stats.current_hp, stats.get_max_hp());
			}
		}

		if (stats.get_three_mp_regen()) {
			if (a_load(stats.current_mp) < stats.get_max_mp()) {
				stats.current_mp += stats.get_three_mp_regen();
				mp_diff += stats.get_three_mp_regen();
				if (a_load(stats.current_mp) > stats.get_max_mp())
					a_store(stats.current_mp, stats.get_max_mp());
			}
		}

		three_regen = elapsed;
	}

	if (elapsed - five_regen > 5.0f) {

		if (stats.get_five_hp_regen() > 0) {
			if (a_load(stats.current_hp) < stats.get_max_hp()) {
				stats.current_hp += stats.get_five_hp_regen();
				hp_diff += stats.get_five_hp_regen();
				if (a_load(stats.current_hp) > stats.get_max_hp())
					a_store(stats.current_hp, stats.get_max_hp());
			}
		}

		if (stats.get_five_mp_regen()) {
			if (a_load(stats.current_mp) < stats.get_max_mp()) {
				stats.current_mp += stats.get_five_mp_regen();
				mp_diff += stats.get_five_mp_regen();
				if (a_load(stats.current_mp) > stats.get_max_mp())
					a_store(stats.current_mp, stats.get_max_mp());
			}
		}

		five_regen = elapsed;
	}

	if (stats.get_special_regen_rate()) {
		if (elapsed - special_regen > stats.get_special_regen_rate()) {

			if (a_load(stats.current_special) < stats.get_special())
			{
				stats.current_special += stats.get_special_regen();

				if (a_load(stats.current_special) > stats.get_special()) {
					a_store(stats.current_special, stats.get_special());
				}

				//do special regen

				data.WriteInt16(0);
				data.WriteInt16(S_PLAYER_STAT_UPDATE);
				data.WriteInt32(a_load(stats.current_hp)); //ACTUAL hp   stats.GetCurrentHp()
				data.WriteInt32(a_load(stats.current_mp)); //ACTUAL mp   stats.GetCurrentMp()
				data.WriteInt32(0);				//unk
				data.WriteInt32(stats.get_max_hp());  //stats.GetHP()  //max
				data.WriteInt32(stats.get_max_mp());  //stats.GetMP()  //max
				data.WriteInt32(stats.get_power());	//base power	[character power + items base power]			 //stats.GetPower()			
				data.WriteInt32(stats.get_endurance());	//base endurance [character endurance + items base endurance]	//	stats.GetEndurance()				 
				data.WriteInt32(stats.get_impact_factor());	//base _impactFactor		
				data.WriteInt32(stats.get_balance_factor());	//base _balanceFactor		
				data.WriteInt16(stats.get_movement_speed(status));	//base movementSpeed	
				data.WriteInt16(0);	//?? 52..							
				data.WriteInt16(stats.get_attack_speed());	//base attackSpeed
				data.WriteFloat(stats.get_crit_rate());	//base critChance
				data.WriteFloat(stats.get_crit_resist());	//base critResistFactor
				data.WriteFloat(stats.get_crit_power());	//base critPower
				data.WriteInt32(stats.get_attack()); 	//base attack range1		
				data.WriteInt32(0);	//base attack range2
				data.WriteInt32(stats.get_defense());	//base defense range1	
				data.WriteInt32(stats.get_impact());	//base impact	
				data.WriteInt32(stats.get_balance());	//base balance	
				data.WriteFloat(stats.get_weakening_resist());	//RESISTANCES	weakening	 stats.GetWeakeningResistance()		
				data.WriteFloat(stats.get_poison_resist());	//RESISTANCES				 stats.GetPeriodicDamageResistance()
				data.WriteFloat(stats.get_stun_resist());	//RESISTANCES	stun		 stats.GetStunResistance()		
				data.WriteInt32(0);	//aditional power [from bonuses and effects]		
				data.WriteInt32(0);	//aditional endurance [from bonuses and effects]	
				data.WriteInt32(0);	//aditional impactFactor							
				data.WriteInt32(0);	//aditional balanceFactor							
				data.WriteInt32(0);	//aditional movementSpeed							
				data.WriteInt32(0);	//aditional attackSpeed								
				data.WriteInt16(0);	//attack ??	
				data.WriteInt16(0);	//attack ??	
				data.WriteInt16(0);	//impact ??						
				data.WriteInt16(0);	//impact ??								
				data.WriteInt16(0);	//??
				data.WriteInt32(0);	//aditional defense range1							
				data.WriteInt32(0);	//aditional defense range2		
				data.WriteInt32(0);	//aditional defense [item defense?]
				data.WriteUInt32(0);	//??
				data.WriteUInt32(0);	//??
				data.WriteUInt32(0);	//??
				data.WriteUInt32(0);	//??
				data.WriteUInt32(0);	//??
				data.WriteUInt32(level);	 //level
				data.WriteUInt8(2);//??
				data.WriteUInt8(2);
				data.WriteUInt8(status);	//?? exp????
				data.WriteUInt8(0);
				data.WriteInt16(0);
				data.WriteInt32(0);
				data.WriteUInt8(0);		//??
				data.WriteUInt32(120);	 //old stamina stuff ??
				data.WriteUInt32(120);	 //old max stamina stuff ??
				data.WriteUInt32(a_load(stats.current_special));	 //RE/WILLPOWER/CHI...CURRENT VALUE	 stats._currentSpecial	
				data.WriteUInt32(stats.get_special());	 //RE/WILLPOWER/CHI...VALUE	 1		 stats._maxSpecial		
				data.WriteUInt32(0);	 //RE/WILLPOWER/CHI...VALUE	 2		 stats._aditionalSpecial
				data.WriteUInt32(stats.infamy);	 //infamy
				data.WriteUInt32(i_.itemLevel);	 //inventory item level
				data.WriteUInt32(i_.profileItemLevel);	 //profile item level
				data.WriteInt32(0);	//warrior stacks
				data.WriteInt32(0);
				data.WriteInt32(8000);	//8000 ???
				data.WriteInt32(123);	//3 energy
				data.WriteInt32(65);	//65 ??
				data.WriteFloat(1000.0f);	//??
				data.WriteFloat(1.0f);		//????
				data.WriteFloat(1.0f);	//scale?
				data.WritePos(0);
				connection_send(con, &data);
				data.Clear();
			}
			special_regen = elapsed;
		}
	}


	if (hp_diff > 0) {
		data.Resize(37);
		data.WriteInt16(37);
		data.WriteInt16(S_CREATURE_CHANGE_HP);
		data.WriteInt32(a_load(stats.current_hp));
		data.WriteInt32(stats.get_max_hp());
		data.WriteInt32(hp_diff);
		data.WriteInt32(3);
		data.WriteUInt64(eid);
		//data.WriteInt64(0);
		//data.WriteByte(0); //crit

		connection_send(con, &data);
		//spawn.bordacast(&data);
		data.Clear();
	}

	if (mp_diff > 0) {
		data.Resize(36);
		data.WriteInt16(36);
		data.WriteInt16(S_PLAYER_CHANGE_MP);
		data.WriteInt32(a_load(stats.current_mp));
		data.WriteInt32(stats.get_max_mp());
		data.WriteInt32(mp_diff);
		data.WriteInt32(2);
		data.WriteUInt64(eid);
		//uint64

		connection_send(con, &data);
		//spawn.bordacast(&data);
		data.Clear();
	}
}

void player::lock_stats()
{
	EnterCriticalSection(&stats_lock);
}

void player::unlock_stats()
{
	LeaveCriticalSection(&stats_lock);
}

void player::save(sql::Connection * sqlCon)
{

	try
	{
		sql::PreparedStatement *ps = sqlCon->prepareStatement("UPDATE players SET x=?, y=?, z=?, h=?, exp=?, restedExp=?, areaId=?, level=?, lastOnlineUTC=?, worldMapGuardId=?, worldMapWorldId=?, worldMapSectionId=?, channel=? WHERE name=?");
		ps->setDouble(1, this->position.x);
		ps->setDouble(2, this->position.y);
		ps->setDouble(3, this->position.z);
		ps->setInt(4, this->position.heading);
		ps->setInt64(5, this->exp);
		ps->setInt64(6, this->restedExp);
		ps->setInt(7, this->position.continent_id);
		ps->setInt(8, this->level);
		ps->setInt64(9, timer_get_current_UTC());
		ps->setInt(10, this->position.worldMapGuardId);
		ps->setInt(11, this->position.worldMapWorldId);
		ps->setInt(12, this->position.worldMapSectionId);
		ps->setInt(13, this->position.channel);
		ps->setString(14, this->name);
		ps->execute();

		ps = sqlCon->prepareStatement("UPDATE player_inventory SET items=?, slotCount=?, gold=? WHERE name=?");
		std::istringstream invBlob = std::istringstream(std::string((const char*)this->i_.get_raw()->_raw, this->i_.get_raw()->_size));
		ps->setBlob(1, &invBlob);
		ps->setInt(2, this->i_.slot_count);
		ps->setInt64(3, this->i_.gold);
		ps->setString(4, this->name);
		ps->execute();
	}
	catch(sql::SQLException &e)
	{
		printf("SQL-ERROR-[%s] FUNC[%s]\n", e.what(), __FUNCTION__);
	}
}


player::player(entityId eid, uint32 db_id, std::shared_ptr<connection> c) :entity(eid), dbid(db_id), i_(inventory())
{
	p_c_a = p_c_c_id = p_c_s = p_c_z = 0;
	channel = 0;
	con = std::move(c);
	exp = restedExp = 0;
	level = 1;
	model = 0;
	status = 0;
	three_regen = five_regen = special_regen = 0.0;
	InitializeCriticalSection(&stats_lock);
}

player::~player()
{
	DeleteCriticalSection(&stats_lock);
}

uint32 WINAPI player_calculate_model(e_player_class pClass, e_player_gender pGender, e_player_race pRace)
{
	return 10000 + ((pRace * 2 + 1 + pGender) * 100) + (pClass + 1);
}

void WINAPI player_load_user_settings(std::shared_ptr<player> p, sql::Connection * con)
{
	Stream data;
	data.WriteInt16(0);
	data.WriteInt16(S_LOAD_CLIENT_USER_SETTING);
	data.WriteInt16(8);

	try
	{
		sql::PreparedStatement *p_s = con->prepareStatement("SELECT * FROM player_settings WHERE username=? AND name=?");
		p_s->setString(1, p->con->_account.username);
		p_s->setString(2, p->name);

		sql::ResultSet* r_s = p_s->executeQuery();
		delete p_s;
		if (r_s && r_s->next())
		{
			std::istream *blob = r_s->getBlob(3);
			if (blob)
			{
				blob->seekg(0, std::istream::end);
				uint32 size = (uint32)blob->tellg();
				blob->seekg(0, std::istream::beg);

				data.Resize(size);
				blob->read((char*)&data._raw[data._pos], size);

				delete blob;
			}
		}
		if (r_s) delete r_s;
	}
	catch (sql::SQLException& e)
	{
		printf("::SQL-ERROR MSG[%s]\n", e.what());
	}

	data.SetEnd();
	data.WritePos(0);
	connection_send(p->con, &data);

	return;
}

bool WINAPI player_send_external_change(std::shared_ptr<player> p, byte broadcast)
{
	Stream data;
	data.Resize(117);
	data.WriteInt16(117);
	data.WriteInt16(S_USER_EXTERNAL_CHANGE);
	data.WriteWorldId(p);
	data.WriteInt32(p->i_.get_profile_item(PROFILE_WEAPON));
	data.WriteInt32(p->i_.get_profile_item(PROFILE_ARMOR));
	data.WriteInt32(p->i_.get_profile_item(PROFILE_GLOVES));
	data.WriteInt32(p->i_.get_profile_item(PROFILE_BOOTS));
	data.WriteInt32(p->i_.get_profile_item(PROFILE_INNERWARE));
	data.WriteInt32(p->i_.get_profile_item(PROFILE_HEAD_ADRONMENT)); //face
	data.WriteInt32(p->i_.get_profile_item(PROFILE_MASK)); //head
	data.WriteUInt64(0);
	data.WriteUInt64(0);
	data.WriteUInt64(0);
	data.WriteUInt64(0);
	data.WriteUInt64(0);
	data.WriteUInt64(0);
	data.WriteInt32(p->i_.get_profile_item_enchant_level(PROFILE_WEAPON));
	data.WriteInt32(p->i_.get_profile_item(PROFILE_SKIN_HEAD));
	data.WriteInt32(p->i_.get_profile_item(PROFILE_SKIN_FACE));
	data.WriteInt32(p->i_.get_profile_item(PROFILE_SKIN_BACK));
	data.WriteInt32(p->i_.get_profile_item(PROFILE_SKIN_WEAPON));
	data.WriteInt32(p->i_.get_profile_item(PROFILE_SKIN_BODY));
	data.WriteUInt32(0); //costume dye
	data.WriteUInt8(1); //enables skin, hair adornment, mask, and costume (back is always on)

	if (broadcast) p->spawn.bordacast(&data);
	return connection_send(p->con, &data);
}

void WINAPI player_write_spawn_packet(std::shared_ptr<player> p, Stream & data_m)
{
	data_m.WriteInt16(0);
	data_m.WriteInt16(S_SPAWN_USER);

	data_m.WriteInt64(0);

	uint16 namePos = data_m.NextPos();
	uint16 guildNamePos = data_m.NextPos();
	uint16 Title = data_m.NextPos();
	uint16 details1Pos = data_m.NextPos();
	data_m.WriteInt16(32);
	uint16 gTitlePos = data_m.NextPos();
	//uint16 gTitleIconPos = data_m.NextPos();
	uint16 details2Pos = data_m.NextPos();
	data_m.WriteInt16(64);

	data_m.WriteSpawnId(p);

	data_m.WriteFloat(p->position.x.load());
	data_m.WriteFloat(p->position.y.load());
	data_m.WriteFloat(p->position.z.load());
	data_m.WriteInt16(p->position.heading.load());

	data_m.WriteInt32(1); //relation ?? enemy / party member ... { 1 = 0 = neutral , 2=party member 3 = enemy, 4 = [orange title?],5 =enemy2?, 6 =[title darker], 7=raid leader? 8=enemye3?,
						  //9 =raid leader?, [light blue] 10=[green title]?, 11=raid leader?, 12 =[DARKx2 title]?, 13=[DARKx2 title]?, 14=1? [...] }
	data_m.WriteInt32(p->model);
	data_m.WriteInt16(0); //unk
	data_m.WriteInt16(52); //unk2 was 70
	data_m.WriteInt16(190); //unk3 was 170
	data_m.WriteInt16(0); //unk4 allways 0?
	data_m.WriteInt16(p->status.load()); //player state { 0 = 1= noncombat ,2 = inCombat ,3 = onMount }

	data_m.WriteUInt8(1); //visible
	data_m.WriteUInt8(1); //alive

	data_m.Write(p->details3, SC_PLAYER_DETAILS_3_BUFFER_SIZE);


	data_m.WriteInt32(p->i_.get_profile_item(PROFILE_WEAPON));
	data_m.WriteInt32(p->i_.get_profile_item(PROFILE_ARMOR));
	data_m.WriteInt32(p->i_.get_profile_item(PROFILE_GLOVES));
	data_m.WriteInt32(p->i_.get_profile_item(PROFILE_BOOTS));
	data_m.WriteInt32(p->i_.get_profile_item(PROFILE_INNERWARE));
	data_m.WriteInt32(p->i_.get_profile_item(PROFILE_HEAD_ADRONMENT)); //face
	data_m.WriteInt32(p->i_.get_profile_item(PROFILE_MASK)); //head

	data_m.WriteInt32(0); //unk 0-1-3 ??
	data_m.WriteInt32(0); //mount...
	data_m.WriteInt32(0); //7 ??? status
	data_m.WriteInt32(1799); // Title id power line

	data_m.WriteUInt64(0);
	data_m.WriteUInt64(0);
	data_m.WriteUInt64(0);
	data_m.WriteUInt64(0);
	data_m.WriteUInt64(0);
	data_m.WriteUInt64(0);
	data_m.WriteUInt64(0);
	data_m.WriteUInt32(0);	  //unk s

	data_m.WriteUInt8(0); //allaways 0?
	data_m.WriteInt16(0);

	data_m.WriteInt32(p->i_.get_profile_item_enchant_level(PROFILE_WEAPON));

	data_m.WriteUInt8(0);//pixie
	data_m.WriteUInt8(0); //second aggro [blue]

	data_m.WriteInt32(p->level);

	data_m.WriteInt32(0);
	data_m.WriteInt32(0);
	data_m.WriteUInt8(1); //unk boolean?

	data_m.WriteInt32(0);
	data_m.WriteInt32(p->i_.get_profile_item(PROFILE_SKIN_HEAD));
	data_m.WriteInt32(p->i_.get_profile_item(PROFILE_SKIN_FACE));
	data_m.WriteInt32(p->i_.get_profile_item(PROFILE_SKIN_BACK));
	data_m.WriteInt32(p->i_.get_profile_item(PROFILE_SKIN_WEAPON));
	data_m.WriteInt32(p->i_.get_profile_item(PROFILE_SKIN_BODY));
	data_m.WriteInt32(0); //costumeDye # ?
	data_m.WriteInt32(0);

	data_m.WriteUInt8(0); //boolean? was 1
	data_m.WriteUInt8(1);
	data_m.WriteUInt32(24);
	data_m.WriteUInt32(0);
	data_m.WriteUInt32(100); //aliance
	data_m.WriteFloat(1.0f); //scale

	data_m.WritePos(namePos);
	data_m.WriteString(p->name);

	data_m.WritePos(guildNamePos);
	data_m.WriteInt32(0);


	data_m.WritePos(Title);
	data_m.WriteInt16(0);

	data_m.WritePos(details1Pos);
	data_m.Write(p->details1, SC_PLAYER_DETAILS_1_BUFFER_SIZE);


	data_m.WritePos(gTitlePos);
	data_m.WriteInt16(0);

	//data_m.WritePos(gTitleIconPos);
	//data_m.WriteInt16(0);

	data_m.WritePos(details2Pos);
	data_m.Write(p->details2, SC_PLAYER_DETAILS_2_BUFFER_SIZE);
	data_m.WritePos(0);

	return;
}

void WINAPI player_send_stats(p_ptr p)
{
	std::unique_ptr<Stream> data = std::make_unique<Stream>();

	p_stats& stats = p->stats;

	data->WriteInt16(0);
	data->WriteInt16(S_PLAYER_STAT_UPDATE);
	data->WriteInt32(a_load(stats.current_hp)); //ACTUAL hp   stats.GetCurrentHp()
	data->WriteInt32(a_load(stats.current_mp)); //ACTUAL mp   stats.GetCurrentMp()
	data->WriteInt32(0);				//unk
	data->WriteInt32(stats.get_max_hp());  //stats.GetHP()  //max
	data->WriteInt32(stats.get_max_mp());  //stats.GetMP()  //max
	data->WriteInt32(stats.get_power());	//base power	[character power + items base power]			 //stats.GetPower()			
	data->WriteInt32(stats.get_endurance());	//base endurance [character endurance + items base endurance]	//	stats.GetEndurance()				 
	data->WriteInt32(stats.get_impact_factor());	//base _impactFactor		
	data->WriteInt32(stats.get_balance_factor());	//base _balanceFactor		
	data->WriteInt16(stats.get_movement_speed(p->status));	//base movementSpeed	
	data->WriteInt16(0);	//?? 52..							
	data->WriteInt16(stats.get_attack_speed());	//base attackSpeed
	data->WriteFloat(stats.get_crit_rate());	//base critChance
	data->WriteFloat(stats.get_crit_resist());	//base critResistFactor
	data->WriteFloat(stats.get_crit_power());	//base critPower
	data->WriteInt32(stats.get_attack()); 	//base attack range1		
	data->WriteInt32(0);	//base attack range2
	data->WriteInt32(stats.get_defense());	//base defense range1	
	data->WriteInt32(stats.get_impact());	//base impact	
	data->WriteInt32(stats.get_balance());	//base balance	
	data->WriteFloat(stats.get_weakening_resist());	//RESISTANCES	weakening	 stats.GetWeakeningResistance()		
	data->WriteFloat(stats.get_poison_resist());	//RESISTANCES				 stats.GetPeriodicDamageResistance()
	data->WriteFloat(stats.get_stun_resist());	//RESISTANCES	stun		 stats.GetStunResistance()		
	data->WriteInt32(0);	//aditional power [from bonuses and effects]		
	data->WriteInt32(0);	//aditional endurance [from bonuses and effects]	
	data->WriteInt32(0);	//aditional impactFactor							
	data->WriteInt32(0);	//aditional balanceFactor							
	data->WriteInt32(0);	//aditional movementSpeed							
	data->WriteInt32(0);	//aditional attackSpeed								
	data->WriteInt16(0);	//attack ??	
	data->WriteInt16(0);	//attack ??	
	data->WriteInt16(0);	//impact ??						
	data->WriteInt16(0);	//impact ??								
	data->WriteInt16(0);	//??
	data->WriteInt32(0);	//aditional defense range1							
	data->WriteInt32(0);	//aditional defense range2		
	data->WriteInt32(0);	//aditional defense [item defense?]
	data->WriteUInt32(0);	//??
	data->WriteUInt32(0);	//??
	data->WriteUInt32(0);	//??
	data->WriteUInt32(0);	//??
	data->WriteUInt32(0);	//??
	data->WriteUInt32(p->level);	 //level
	data->WriteUInt8(2);//??
	data->WriteUInt8(2);
	data->WriteUInt8(p->status);	//?? exp????
	data->WriteUInt8(0);
	data->WriteInt16(0);
	data->WriteInt32(0);
	data->WriteUInt8(0);		//??
	data->WriteUInt32(120);	 //old stamina stuff ??
	data->WriteUInt32(120);	 //old max stamina stuff ??
	data->WriteUInt32(a_load(stats.current_special));	 //RE/WILLPOWER/CHI...CURRENT VALUE	 stats._currentSpecial	
	data->WriteUInt32(stats.get_special());	 //RE/WILLPOWER/CHI...VALUE	 1		 stats._maxSpecial		
	data->WriteUInt32(0);	 //RE/WILLPOWER/CHI...VALUE	 2		 stats._aditionalSpecial
	data->WriteUInt32(p->stats.infamy);	 //infamy
	data->WriteUInt32(p->i_.itemLevel);	 //inventory item level
	data->WriteUInt32(p->i_.profileItemLevel);	 //profile item level
	data->WriteInt32(0);	//warrior stacks
	data->WriteInt32(0);
	data->WriteInt32(8000);	//8000 ???
	data->WriteInt32(123);	//3 energy
	data->WriteInt32(65);	//65 ??
	data->WriteFloat(1000.0f);	//??
	data->WriteFloat(1.0f);		//????
	data->WriteFloat(1.0f);	//scale?
	data->WritePos(0);
	connection_send(p->con, data.get());

	return;
}

void WINAPI player_recalculate_inventory_stats(p_ptr p)
{
	p->i_.refresh_enchat_effect();
	p->i_.refresh_items_modifiers();

	p->stats.clear_bonus();
	std::vector<const passivity_template*> passivities;
	p->lock_stats();
	p->i_.get_profile_passivities(passivities);
	p->unlock_stats();
	for (size_t i = 0; i < passivities.size(); i++) {
		passivity_proces(p, passivities[i]);
	}
	return;
}

void WINAPI player_recalculate_stats(p_ptr p)
{
	s_stats_init_player(p);

	std::vector<const passivity_template*> passivities;
	p->i_.get_profile_passivities(passivities);

	p->lock_stats();
	for (size_t i = 0; i < passivities.size(); i++) {
		passivity_proces(p, passivities[i]);
	}
	p->unlock_stats();

	player_send_stats(p);

	return;
}


e_player_class WINAPI player_get_class(uint32 model)
{
	return (e_player_class)((model % 100) - 1);
}

e_player_race WINAPI player_get_race(uint32 model)
{
	return (e_player_race)((model % 200) + 1);
}

e_player_gender WINAPI player_get_gender(uint32 model)
{
	return (e_player_gender)((((model / 20) % 10) / 5) + 1);
}
