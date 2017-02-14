#include "area.h"
#include "player.h"

area::area() :channel_count(0)
{
}

bool area::load_from_xml_node(XMLDocumentParser::XMLNode* n)
{
	XMLDocumentParser::XMLNodeArgument * arg = nullptr;
	if ((arg = n->GetArgument("channels")))
		channel_count = atoi(arg->argumentValue.c_str());
	else return false;

	if ((arg = n->GetArgument("name")))
		strcpy_s(name, arg->argumentValue.c_str());
	else return false;


	if (!bounded::load_from_xml_node(n))
		return false;
	for (uint32 j = 0; j < channel_count; j++)
	{
		for (size_t i = 0; i < n->childNodes.size(); i++)
			if (n->childNodes[i] && n->childNodes[i]->tagName == "Zone")
			{
				std::vector<zone>channel;
				zone z_;
				if (!z_.load_from_xml_node(n->childNodes[i]))
					return false;
				channel.push_back(std::move(z_));
				channels.push_back(std::move(channel));
			}
	}
	return true;
}

zone * area::get_zone(std::shared_ptr<player> p, uint32 channel)
{
	for (size_t i = 0; i < channels[channel].size(); i++)
		if (channels[channel][i].intersects_bounds(p, true))
		{
			p->p_c_z.store((uint8)i);
			p->channel = channel;
			return &channels[channel][i];
		}

	return nullptr;
}

void area::collect_visible_players(player_list & p_l, std::shared_ptr<player> p)
{
	float p_v_r[4] = {
		p->position.x.load() - SC_PLAYER_VISIBLE_RANGE / 2,
		p->position.y.load() - SC_PLAYER_VISIBLE_RANGE / 2,
		SC_PLAYER_VISIBLE_RANGE ,
		SC_PLAYER_VISIBLE_RANGE
	};

	if (channels[p->channel.load()][p->p_c_z.load()].inflates_bounds(p))
	{
		size_t size = channels[p->channel.load()].size();
		for (size_t i = 0; i < size; i++)
		{
			if (channels[p->channel.load()][i].intersects_bounds(p))
				channels[p->channel.load()][i].collect_visible_players(p_l, p_v_r, p->eid);
		}
	}
	else
		channels[p->channel.load()][p->p_c_z.load()].collect_visible_players(p_l, p_v_r, p->eid);
}
