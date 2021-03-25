#include "Grid.h"

Grid::Grid(int cont_width, int cont_height, int width, int height)
	: contDims({ cont_width, cont_height })
{
	const int cHeight = height / cont_height;
	const int cWidth = width / cont_width;

	containers = new Container[cWidth * cHeight];

	for (int y = 0; y < cHeight; ++y)
	{
		for (int x = 0; x < cWidth; ++x)
		{
			containers[x + y * cWidth] = Container(rect_i(
				{ x * cont_width, y * cont_height },
				{ x * cont_width + cont_width, y * cont_height + cont_height }));
		}
	}
}

Grid::~Grid()
{
	delete[] containers;
}

void Grid::insert(const Boid& boid)
{

}
