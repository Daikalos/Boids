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
	std::unordered_set<const Boid*> foundBoids;

	Container<Boid> nwCont = at_pos(sf::Vector2f(pos.x - radius, pos.y - radius));
	Container<Boid> neCont = at_pos(sf::Vector2f(pos.x + radius, pos.y - radius));
	Container<Boid> swCont = at_pos(sf::Vector2f(pos.x - radius, pos.y + radius));
	Container<Boid> seCont = at_pos(sf::Vector2f(pos.x + radius, pos.y + radius));

	for (const Boid* b : nwCont.items)
		foundBoids.insert(b);
	for (const Boid* b : neCont.items)
		foundBoids.insert(b);
	for (const Boid* b : swCont.items)
		foundBoids.insert(b);
	for (const Boid* b : seCont.items)
		foundBoids.insert(b);

	std::vector<Boid> result;

	for (const Boid* b : foundBoids)
		result.push_back(*b);

	return result;
}
