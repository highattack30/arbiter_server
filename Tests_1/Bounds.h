#ifndef BOUNDS_H
#define BOUNDS_H

#include "typeDefs.h"
#include "serverEnums.h"
#include "XMLDocumentParser.h"

#include<memory>
#include <vector>

class player; class XMLNode;

bool WINAPI rectangle_vs_point(float px, float py, float rectangle[4]);
bool WINAPI rectangle_vs_point(float *p, float rectangle[4]);
bool WINAPI rectangle_vs_rectangle(float rectangle1[4], float rectangle2[4]);
bool WINAPI rectangle_inflates_rectangle(float rectangle1[4] , float rectangle2[4]);
float WINAPI distance_point_to_point(float p1x, float p1y, float p2x, float p2y);
bool WINAPI rectangle_vs_point(std::shared_ptr<player> p, float rectangle[4]);
class bounded
{
protected:
	virtual bool load_from_xml_node(XMLDocumentParser::XMLNode*);

	float bounds[4]; //x,y ; w,h
public:
	bool intersects_bounds(std::shared_ptr<player> p, bool point = false);
	bool inflates_bounds(std::shared_ptr<player> p, bool point = false);
};


#endif
