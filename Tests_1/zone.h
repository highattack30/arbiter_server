#ifndef AREA_B_H
#define AREA_B_H

#include "Bounds.h"
#include "typeDefs.h"
#include "pms.h"

#include "player_rcu.h"

class w_player;
class zone : public bounded
{
public:
	bool isSameAsParent;

	zone();
	bool load_from_xml_node(XMLDocumentParser::XMLNode*);

	struct section{

		section(float[4]);
		section(section&);
		float							fences[4];
		player_rcu						players;


		//std::vector<creature>			creatures;
		//std::vector<creature>			npcs;
		//std::vector<creature>			objects;
	};

	std::vector<zone::section> sections;


	void collect_visible_players(player_list& p_l, float[4], entityId);

	section* get_section(std::shared_ptr<player>);
};


#endif