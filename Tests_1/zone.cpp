#include "zone.h"
#include "player.h"

#include  <iostream>

zone::zone() : bounded(), isSameAsParent(false)
{
}

bool zone::load_from_xml_node(XMLDocumentParser::XMLNode* n)
{
	XMLDocumentParser::XMLNodeArgument* arg = nullptr;

	if (!bounded::load_from_xml_node(n))
		return false;

	for (size_t i = 0; i < n->childNodes.size(); i++)
		if (n->childNodes[i] && n->childNodes[i]->tagName == "Section")
		{
			float f[4];
			if ((arg = n->childNodes[i]->GetArgument("rectangle")))
				sscanf_s(arg->argumentValue.c_str(), "%f,%f;%f,%f", &f[0], &f[1], &f[2], &f[3]);
			else
				return false;

			sections.push_back(std::move(zone::section(f)));
		}

	return true;
}

void zone::collect_visible_players(player_list & p_l, float p_v_r[4], entityId me)
{
	for (size_t i = 0; i < sections.size(); i++)
	{
		if (rectangle_vs_rectangle(p_v_r, sections[i].fences))
			sections[i].players.collect_visible_players(p_l, p_v_r, me);
	}
}


zone::section * zone::get_section(std::shared_ptr<player> p)
{
	for (size_t i = 0; i < sections.size(); i++)
		if (rectangle_vs_point(p, sections[i].fences))
		{
			p->p_c_s.store((uint8)i);
			return &sections[i];
		}

	return nullptr;
}

zone::section::section(float f[4]) :fences{ f[0],f[1],f[2],f[3] }
{
}

zone::section::section(section & s) : fences{ s.fences[0], s.fences[1], s.fences[2], s.fences[3] }
{
}
