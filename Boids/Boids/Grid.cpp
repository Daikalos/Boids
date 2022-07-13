#include "Grid.h"

Grid::Grid(int grid_left, int grid_top, int grid_right, int grid_bot, int cont_width, int cont_height)
	: contDims({ cont_width, cont_height }), gridRect(grid_left, grid_top, grid_right, grid_bot)
{
	width = gridRect.width() / cont_width;
    height = gridRect.height() / cont_height;

	containers = new Container<Boid>[width * height];

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			containers[x + y * (long)width] = Container<Boid>(Rect_i(
				 x * cont_width + grid_left,			  y * cont_height + grid_top,
				 x * cont_width + cont_width + grid_left, y * cont_height + cont_height + grid_top));
		}
	}
}

Grid::~Grid()
{
	delete[] containers;
}

void Grid::insert(Boid& item) const
{
	Container<Boid>* newCntn = at_pos(item);
	Container<Boid>* curCntn = item.get_container();

	if (newCntn == nullptr || curCntn == newCntn)
		return;

	if (curCntn != nullptr && curCntn != newCntn)
	{
		curCntn->erase(&item);
	}

	item.set_container(newCntn);
	newCntn->insert(&item);
}

std::vector<const Container<Boid>*> Grid::query_containers(sf::Vector2f pos, float radius) const
{
	std::vector<const Container<Boid>*> cntns;

	const Container<Boid>* cntn = at_pos(pos);

	if (cntn != nullptr && cntn->items.size() > 0)
		cntns.push_back(at_pos(pos));

	for (int x = -radius; x <= (int)radius; x += radius)
		for (int y = -radius; y <= (int)radius; y += radius)
		{
			if (x == 0 && y == 0)
				continue;

			const Container<Boid>* cntn = at_pos(sf::Vector2f(pos.x + x, pos.y + y));

			if (cntn != nullptr && cntn->items.size() > 0)
				cntns.push_back(cntn);
		}

	return cntns;
}
