#include "Grid.h"

Grid::Grid(int grid_left, int grid_top, int grid_right, int grid_bot, int cont_width, int cont_height)
	: contDims({ cont_width, cont_height }), gridRect(grid_left, grid_top, grid_right, grid_bot)
{
	width = gridRect.width() / cont_width;
    height = gridRect.height() / cont_height;
	count = width * height;

	cellsStartIndices = (int*)::operator new(count * sizeof(int));
	cellsEndIndices = (int*)::operator new(count * sizeof(int));

	reset_buffers();
}

Grid::~Grid()
{

}
