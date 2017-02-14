#include "continent.h"
#include "player.h"
#include "job.h"
#include "arbiter_server.h"
#include "world_server.h"
#include "jobEnums.h"
#include "dataservice.h"
#include "worldposition.h"
#include "XMLDocumentParser.h"

#include <atomic>
#include <memory>

area * continent::get_area(std::shared_ptr<player> p)
{
	for (size_t i = 0; i < areas.size(); i++)
	{
		if (areas[i].intersects_bounds(p, true))
		{
			p->p_c_a.store((uint8)i);
			return &areas[i];
		}
	}
	return false;
}

continent::continent() : index(-1), visible_range(0), identity{ 0, 0, 0, 0, 0, 0 } { }

bool continent::load_from_xml_node(XMLDocumentParser::XMLNode* n)
{
	if (!n) return false;

	XMLDocumentParser::XMLNodeArgument * arg = nullptr;

	if ((arg = n->GetArgument("identity")))
	{
		std::string chuck = "";

		std::vector<uint32> _identity;
		if (arg->argumentValue.size() > 1)
			for (size_t i = 0; i < arg->argumentValue.size(); i++)
			{
				if (arg->argumentValue[i] == ';')
				{
					_identity.push_back(atoi(chuck.c_str()));
					chuck = "";
					continue;
				}
				else if (i == arg->argumentValue.size() - 1)
				{
					_identity.push_back(atoi(chuck.c_str()));
					break;
				}

				chuck += arg->argumentValue[i];
			}
		else
			_identity.push_back(atoi(arg->argumentValue.c_str()));

		for (size_t j = 0; j < _identity.size(); j++)
		{
			identity[j] = _identity[j];
		}
	}
	else return false;

	if ((arg = n->GetArgument("name")))
		strcpy_s(name, arg->argumentValue.c_str());
	else return false;

	if (!bounded::load_from_xml_node(n)) return false;

	for (size_t i = 0; i < n->childNodes.size(); i++)
		if (n->childNodes[i] && n->childNodes[i]->tagName == "Area")
		{
			area a;
			if (!a.load_from_xml_node(n->childNodes[i])) return false;
			areas.push_back(std::move(a));
		}

	return true;
}
