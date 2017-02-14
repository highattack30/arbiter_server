#include "Bounds.h"
#include "player.h"

bool bounded::load_from_xml_node(XMLDocumentParser::XMLNode *n)
{
	XMLDocumentParser::XMLNodeArgument* arg = nullptr;

	for (size_t i = 0, j = 0; j < 4 && i < n->childNodes.size(); i++)
	{
		if (n->childNodes[i] && n->childNodes[i]->tagName == "Bounds")
		{
			if ((arg = n->childNodes[i]->GetArgument("rectangle")))
				sscanf_s(arg->argumentValue.c_str(), "%f,%f;%f,%f", &bounds[0], &bounds[1], &bounds[2], &bounds[3]);
			else
				return false;
		}
	}
	return true;
}

bool bounded::intersects_bounds(std::shared_ptr<player> p, bool point)
{
	if (point)
		return rectangle_vs_point(p, bounds);
	else
	{
		float p_r[4] = { p->position.x.load() - SC_PLAYER_VISIBLE_RANGE / 2, p->position.y.load() - SC_PLAYER_VISIBLE_RANGE / 2, SC_PLAYER_VISIBLE_RANGE ,SC_PLAYER_VISIBLE_RANGE };
		return rectangle_vs_rectangle(bounds, p_r);
	}
}

bool bounded::inflates_bounds(std::shared_ptr<player> p, bool point)
{
	if (point)
		return !rectangle_vs_point(p, bounds);
	else
	{
		float p_r[4] = { p->position.x.load() - SC_PLAYER_VISIBLE_RANGE / 2, p->position.y.load() - SC_PLAYER_VISIBLE_RANGE / 2, SC_PLAYER_VISIBLE_RANGE ,SC_PLAYER_VISIBLE_RANGE };
		return rectangle_inflates_rectangle(bounds, p_r);
	}
}


bool WINAPI rectangle_vs_point(float px, float py, float rectangle[4])
{
	return(((rectangle[0] + rectangle[2]) > px) && ((rectangle[1] + rectangle[3]) > py) && (rectangle[0] < px) && (rectangle[1] < py));
}

bool WINAPI rectangle_vs_point(float * p, float rectangle[4])
{
	return(((rectangle[0] + rectangle[2]) > p[0]) && ((rectangle[1] + rectangle[3]) > p[1]) && (rectangle[0] < p[0]) && (rectangle[1] < p[1]));
}

bool WINAPI rectangle_vs_point(std::shared_ptr<player> p, float rectangle[4])
{
	return(((rectangle[0] + rectangle[2]) > p->position.x.load()) && ((rectangle[1] + rectangle[3]) > p->position.y.load()) && (rectangle[0] < p->position.x.load()) && (rectangle[1] < p->position.y.load()));
}

bool WINAPI rectangle_vs_rectangle(float rectangle1[4], float rectangle2[4])
{
	return ((rectangle1[0] < (rectangle2[0] + rectangle2[2])) && ((rectangle1[0] + rectangle1[2]) > rectangle2[0]) &&
		(rectangle1[1] < (rectangle2[1] + rectangle2[3])) && ((rectangle1[1] + rectangle1[3] > rectangle2[1])));
}

bool WINAPI rectangle_inflates_rectangle(float rectangle1[4], float rectangle2[4])
{
	return
		(
		(((rectangle1[0] + rectangle1[2]) < (rectangle2[0] + rectangle2[2])) && ((rectangle1[0] + rectangle1[2]) > rectangle2[0])) ||
			(((rectangle1[1] + rectangle1[3]) < (rectangle2[1] + rectangle2[3])) && ((rectangle1[1] + rectangle1[3]) > rectangle2[1])) ||
			((rectangle1[0] > rectangle2[0]) && (rectangle1[0] < (rectangle2[0] + rectangle2[2]))) ||
			((rectangle1[1] > rectangle2[1]) && (rectangle1[1] < (rectangle2[1] + rectangle2[3])))
			);
}

float WINAPI distance_point_to_point(float p1x, float p1y, float p2x, float p2y)
{
	float a = p1x - p2x;
	float b = p1y - p2y;
	return sqrt(a * a + b * b);
}
