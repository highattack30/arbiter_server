#ifndef AREA_H
#define AREA_H

#include "typeDefs.h"
#include "mathUtils.h"
#include "win32.h"

class w_player; class player;

#include "area.h"

typedef std::vector<std::shared_ptr<w_player>> player_collection;

class continent :public bounded
{
public:
	std::vector<area>		areas;
	uint16					visible_range;
	uint32					identity[6];
	int32					index;
	char					name[CONTINENT_NAME_MAX];


	bool					load_from_xml_node(XMLDocumentParser::XMLNode*) override;

	area* get_area(std::shared_ptr<player>);
	continent();
};



#endif