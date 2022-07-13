#pragma once

#include <unordered_set>
#include <vector>

#include "Config.h"

class Boid;

struct Container
{
	Container()
	{
		neighbours.reserve(8);
	}

	bool insert(const Boid* item)
	{
		if (items.size() >= Config::grid_cell_max_boids)
			return false;

		items.insert(item);

		return true;
	}
	bool erase(const Boid* item)
	{
		items.erase(item);
		return true;
	}

	void add_neighbour(const Container* cntn)
	{
		neighbours.push_back(cntn);
	}

	std::unordered_set<const Boid*> items;
	std::vector<const Container*> neighbours;
};