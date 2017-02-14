#ifndef AREA_A_H
#define AREA_A_H

#include "typeDefs.h"
#include "Bounds.h"
#include "zone.h"

#include <vector>

class area :public bounded
{
public:
	area();

	std::vector<std::vector<zone>> channels;
	uint32 channel_count;
	char name[SC_AREA_NAME_MAX_LENGTH];

	bool load_from_xml_node(XMLDocumentParser::XMLNode*);

	zone* get_zone(std::shared_ptr<player>, uint32);

	void collect_visible_players(player_list&, std::shared_ptr<player>);
};


#endif
