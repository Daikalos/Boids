#pragma once

#include <unordered_set>
#include <vector>

#include "Rectangle.h"

class Boid;

struct Container
{
	Container()
	{
		neighbours.reserve(8);
	}

	void insert(const Boid* item)
	{
		items.insert(item);
	}
	void erase(const Boid* item)
	{
		items.erase(item);
	}

	void add_neighbour(const Container* cntn)
	{
		neighbours.push_back(cntn);
	}

	std::unordered_set<const Boid*> items;
	std::vector<const Container*> neighbours;
};