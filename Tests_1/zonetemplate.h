#ifndef AREATEMPLATE_H
#define AREATEMPLATE_H

#include <vector>

#include "typeDefs.h"

typedef struct zone_template
{
	int32
		continentId,
		worldMapWorldId,
		worldMapGuardId,
		worldMapSectionId,
		huntingZoneId,
		priority,
		campId,
		maxPlayerCount,
		floor;

	float
		substractMinZ,
		addMaxZ;

	std::string name;
	bool
		restExpBonus,
		ride,
		maze,
		vender,
		pk,
		pcMoveCylinder,
		ignoreObstacleShortTel,
		pad;

	//std::vector<float[3]> fences;

}area_section, *lparea_section;

#endif
