#include "account.h"
#include "connexion.h"
#include "player.h"
#include "Stream.h"
#include "dataservice.h"
#include "config.h"
#include "skylake_stats.h"
#include "entity_manager.h"

#include <string>
#include <unordered_map>
#include <istream>

bool account_perform_login(std::shared_ptr<connection> c, sql::Connection * conn, byte ticket[33], byte username_cstr[SC_PLAYER_NAME_MAX_LENGTH])
{
	if (!c || !conn || !username_cstr)
		return false;
	sql::PreparedStatement *p = nullptr;

	sql::ResultSet * rs;
	try
	{
		p = conn->prepareStatement("SELECT * FROM accounts WHERE username=? AND password=?");
		if (!p)
			return false;

		p->setString(1, (const char*)username_cstr);
		p->setString(2, (const char*)ticket);

		rs = p->executeQuery();
	}
	catch (sql::SQLException & e)
	{
		printf("::SQL-EX::LN[%d] FN[%s] EX[%s]\n", __LINE__, __FUNCTION__, e.what());

		delete p;
		return false;
	}
	if (!rs)
	{
		delete p;
		return false;
	}

	if (rs->next())
	{
		try
		{
			c->_account.isGm = rs->getBoolean("isGm");
			c->_account.id = rs->getInt("id");
		}
		catch (sql::SQLException & e)
		{
			printf("::SQL-EX::LN[%d] FN[%s] EX[%s]\n", __LINE__, __FUNCTION__, e.what());

			delete rs;
			delete p;
			return false;
		}

		memcpy(c->_account.username, username_cstr, 32);
		memcpy(c->_account.password, ticket, 33);

		delete rs;
		rs = NULL;

		p->close();
		delete p;
		p = NULL;


		p = conn->prepareStatement("SELECT * FROM players WHERE username=?");
		p->setString(1, (const char*)username_cstr);

		sql::ResultSet * rs;
		try
		{
			rs = p->executeQuery();
		}
		catch (sql::SQLException & e)
		{
			printf("::SQL-EX::LN[%d] FN[%s] EX[%s]\n", __LINE__, __FUNCTION__, e.what());

			delete p;
			return false;
		}
		delete p;

		//load players
		for (uint16 i = 0; i < SC_PLAYER_MAX_CHARACTER_COUNT; i++)
		{
			if (rs && rs->next())
			{
				std::shared_ptr<player> newPlayer = entity_manager::create_player(c, rs->getInt(1));
				newPlayer->spawn.init(newPlayer);

				std::string name = rs->getString("name").c_str();
				size_t nameLen = strlen(name.c_str());
				if (nameLen > SC_PLAYER_NAME_MAX_LENGTH)
					nameLen = SC_PLAYER_NAME_MAX_LENGTH;

				if (!memcpy(newPlayer->name, name.c_str(), nameLen + 1))
				{
					continue;
				}

				newPlayer->exp = rs->getInt64("exp");
				newPlayer->restedExp = rs->getInt64("restedExp");

				newPlayer->model = player_calculate_model((e_player_class)rs->getInt("class"), (e_player_gender)rs->getInt("gender"), (e_player_race)rs->getInt("race"));

				newPlayer->level = (uint16)rs->getInt("level");

				newPlayer->position.x.store((float)rs->getDouble("x"));
				newPlayer->position.y.store((float)rs->getDouble("y"));
				newPlayer->position.z.store((float)rs->getDouble("z"));
				newPlayer->position.heading = (int16)rs->getInt("h");

				newPlayer->pRace = (e_player_race)rs->getInt("race");
				newPlayer->pGender = (e_player_gender)rs->getInt("gender");
				newPlayer->pClass = (e_player_class)rs->getInt("class");

				newPlayer->position.channel = (uint32)rs->getInt(25);
				newPlayer->position.continent_id = (uint32)rs->getInt("areaId");
				newPlayer->position.worldMapGuardId = (uint32)rs->getInt(22);
				newPlayer->position.worldMapWorldId = (uint32)rs->getInt(23);
				newPlayer->position.worldMapSectionId = (uint32)rs->getInt(24);

				std::istream * blob = rs->getBlob("details1");
				if (blob)
				{
					blob->read((char*)newPlayer->details1, SC_PLAYER_DETAILS_1_BUFFER_SIZE);
					delete blob;
				}

				blob = rs->getBlob("details2");
				if (blob)
				{
					blob->read((char*)newPlayer->details2, SC_PLAYER_DETAILS_2_BUFFER_SIZE);
					delete blob;
				}

				blob = rs->getBlob("details3");
				if (blob)
				{
					blob->read((char*)newPlayer->details3, SC_PLAYER_DETAILS_3_BUFFER_SIZE);
					delete blob;
				}

				//visited sections

#pragma region inventory
				p = conn->prepareStatement("SELECT * FROM player_inventory WHERE username=? AND name=?");
				p->setString(1, c->_account.username);
				p->setString(2, newPlayer->name);
				sql::ResultSet *rs2 = NULL;
				try
				{
					rs2 = p->executeQuery();
				}
				catch (sql::SQLException & e)
				{
					printf("::SQL-EX::LN[%d] FN[%s] EX[%s]\n", __LINE__, __FUNCTION__, e.what());

					p->close();
					delete p;
					return false;
				}

				if (rs2 && rs2->next())
				{
					inventory_init(newPlayer, (uint16)rs2->getInt(4));
					std::istream * inventory_blob = rs2->getBlob(3);
					if (inventory_blob)
					{
						Stream items = Stream(inventory_blob);

						uint16 profile_next = items.ReadUInt16();
						uint16 profile_item_count = items.ReadUInt16();

						uint16 inventory_next = items.ReadUInt16();
						uint16 inventory_item_count = items.ReadUInt16();

						items._pos = profile_next;
						for (uint32 j = 0; j < profile_item_count; j++)
						{
							slot_id s_id = items.ReadInt32();
							item_id i_id = items.ReadInt32();

							const item_template * i = data_service::data_resolve_item(i_id);
							if (!i) continue;

							newPlayer->i_.profile_slots[s_id]._item = entity_manager::create_item();
							newPlayer->i_.profile_slots[s_id].isEmpty = 0;
							newPlayer->i_.profile_slots[s_id]._item->item_t = i;


							newPlayer->i_.profile_slots[s_id]._item->hasCrystals = items.ReadUInt8();
							for (byte j = 0; j < newPlayer->i_.profile_slots[s_id]._item->hasCrystals; j++)
								newPlayer->i_.profile_slots[s_id]._item->crystals[j] = items.ReadInt32();

							newPlayer->i_.profile_slots[s_id]._item->itemLevel = items.ReadUInt8();
							newPlayer->i_.profile_slots[s_id]._item->isMasterworked = items.ReadUInt8();
							newPlayer->i_.profile_slots[s_id]._item->masterworkRate = items.ReadFloat();
							newPlayer->i_.profile_slots[s_id]._item->isAwakened = items.ReadUInt8();
							newPlayer->i_.profile_slots[s_id]._item->isEnigmatic = items.ReadUInt8();
							newPlayer->i_.profile_slots[s_id]._item->enchantLevel = items.ReadUInt8();
							newPlayer->i_.profile_slots[s_id]._item->isBinded = items.ReadUInt8();
							newPlayer->i_.profile_slots[s_id]._item->binderDBId = items.ReadInt32();
							newPlayer->i_.profile_slots[s_id]._item->isCrafted = items.ReadUInt8();
							newPlayer->i_.profile_slots[s_id]._item->crafterDBId = items.ReadInt32();
							newPlayer->i_.profile_slots[s_id]._item->stackCount = items.ReadInt32();

							uint16 passivitiesCount = items.ReadInt16();
							for (uint16 g = 0; g < passivitiesCount; g++)
							{
								const passivity_template  * ps = data_service::data_resolve_passivity(items.ReadInt32());
								if (ps)
									newPlayer->i_.profile_slots[s_id]._item->passivities.push_back(ps);
							}
						}


						items._pos = inventory_next;
						for (uint32 j = 0; j < inventory_item_count; j++)
						{
							slot_id s_id = items.ReadInt32();
							item_id i_id = items.ReadInt32();

							const item_template * i = data_service::data_resolve_item(i_id);
							if (!i) continue;

							newPlayer->i_.inventory_slots[s_id]._item = entity_manager::create_item();
							newPlayer->i_.inventory_slots[s_id].isEmpty = 0;
							newPlayer->i_.inventory_slots[s_id]._item->item_t = i;


							newPlayer->i_.inventory_slots[s_id]._item->hasCrystals = items.ReadUInt8();
							for (byte j = 0; j < newPlayer->i_.inventory_slots[s_id]._item->hasCrystals; j++)
								newPlayer->i_.inventory_slots[s_id]._item->crystals[j] = items.ReadInt32();

							newPlayer->i_.inventory_slots[s_id]._item->itemLevel = items.ReadUInt8();
							newPlayer->i_.inventory_slots[s_id]._item->isMasterworked = items.ReadUInt8();
							newPlayer->i_.inventory_slots[s_id]._item->masterworkRate = items.ReadFloat();
							newPlayer->i_.inventory_slots[s_id]._item->isAwakened = items.ReadUInt8();
							newPlayer->i_.inventory_slots[s_id]._item->isEnigmatic = items.ReadUInt8();
							newPlayer->i_.inventory_slots[s_id]._item->enchantLevel = items.ReadUInt8();
							newPlayer->i_.inventory_slots[s_id]._item->isBinded = items.ReadUInt8();
							newPlayer->i_.inventory_slots[s_id]._item->binderDBId = items.ReadUInt32();
							newPlayer->i_.inventory_slots[s_id]._item->isCrafted = items.ReadUInt8();
							newPlayer->i_.inventory_slots[s_id]._item->crafterDBId = items.ReadUInt32();
							newPlayer->i_.inventory_slots[s_id]._item->stackCount = items.ReadInt32();

							uint16 passivitiesCount = items.ReadUInt16();
							for (uint16 g = 0; g < passivitiesCount; g++)
							{
								const passivity_template  * ps = data_service::data_resolve_passivity(items.ReadInt32());
								if (ps) newPlayer->i_.inventory_slots[s_id]._item->passivities.push_back(ps);
							}
						}


						delete inventory_blob;
					}

					delete rs2;
				}
				else
				{
					inventory_init(newPlayer, (uint16)config::player.start_inventory_slot_count);
				}
				delete p;
#pragma endregion

				//bank
				//skills
				//settings

				s_stats_init_player(newPlayer);
				newPlayer->c_manager.init(newPlayer);
				c->_players[i] = newPlayer;
			}
			else
				break;
		}

	}
	else
	{
		delete rs;
		delete p;
		return false;
	}

	return true;
}

void WINAPI account_load_client_settings(std::shared_ptr<connection> p, sql::Connection * con)
{
	Stream data;
	data.WriteInt16(0);
	data.WriteInt16(S_LOAD_CLIENT_ACCOUNT_SETTING);
	data.WriteInt16(8);
	try
	{
		sql::PreparedStatement *p_s = con->prepareStatement("SELECT * FROM accounts WHERE username=?");
		p_s->setString(1, p->_account.username);

		sql::ResultSet* r_s = p_s->executeQuery();
		delete p_s;
		if (r_s && r_s->next())
		{

			std::istream *blob = r_s->getBlob(10);
			if (blob)
			{
				blob->seekg(0, std::istream::end);
				uint16 size = (uint16)blob->tellg();
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
	connection_send(p, &data);
}


