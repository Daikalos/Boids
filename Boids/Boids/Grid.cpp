#include "Grid.h"

Grid::Grid(int grid_left, int grid_top, int grid_right, int grid_bot, int cont_width, int cont_height)
	: contDims({ cont_width, cont_height }), gridRect(grid_left, grid_top, grid_right, grid_bot)
{
	width = gridRect.width() / cont_width;
    height = gridRect.height() / cont_height;

	containers = new Container[width * height];

	std::for_each(
		std::execution::par_unseq,
		containers,
		containers + width * height,
		[&](Container& cntn)
		{
			int i = &cntn - containers;

			int x_pos = i % width;
			int y_pos = i / width;

			cntn.add_neighbour(&cntn);

			for (int x = -1; x <= 1; ++x)
				for (int y = -1; y <= 1; ++y)
				{
					if (x == 0 && y == 0)
						continue;

					const Container* neighbour = at_pos(sf::Vector2i(x_pos + x, y_pos + y));

					if (neighbour != nullptr)
						cntn.add_neighbour(neighbour);
				}
		});
}

Grid::~Grid()
{
	delete[] containers;
}

void Grid::insert(Boid& item) const
{
	Container* newCntn = at_pos(item);
	Container* curCntn = item.get_container();

	if (newCntn == nullptr || curCntn == newCntn)
		return;

	if (curCntn != nullptr && curCntn != newCntn)
	{
		curCntn->erase(&item);
	}

	item.set_container(newCntn);
	newCntn->insert(&item);
}
