#include "Grid.h"

Grid::Grid(int cont_width, int cont_height, int grid_width, int grid_height)
	: contDims({ cont_width, cont_height })
{
    height = grid_height / cont_height;
	width = grid_width / cont_width;

	containers = new Container<Boid>[width * height];

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			containers[x + y * width] = Container<Boid>(Rect_i(
				{ x * cont_width, y * cont_height },
				{ x * cont_width + cont_width, y * cont_height + cont_height }));
		}
	}
}

Grid::~Grid()
{
	delete[] containers;
}

void Grid::insert(Boid& boid)
{
	boid.assign_container(at_pos(boid));
}

std::vector<Boid> Grid::query(sf::Vector2f pos, float radius)
{
	std::vector<Boid> foundBoids;
	std::unordered_set<Container<Boid>*> cntns;

	cntns.insert(at_pos(sf::Vector2f(pos.x - radius, pos.y - radius)));
	cntns.insert(at_pos(sf::Vector2f(pos.x + radius, pos.y - radius)));
	cntns.insert(at_pos(sf::Vector2f(pos.x - radius, pos.y + radius)));
	cntns.insert(at_pos(sf::Vector2f(pos.x + radius, pos.y + radius)));

	for (const Container<Boid>* c : cntns)
	{
		if (!c) continue;

		std::unordered_set<const Boid*> boids((*c).items);

		for (const Boid* b : boids)
		{
			foundBoids.push_back(*b);
		}
	}

	return foundBoids;
}
