#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <execution>

#include "VecUtil.h"
#include "Rectangle.h"

class Grid
{
public:
	Grid(int grid_left, int grid_top, int grid_right, int grid_bot, int cont_width, int cont_height)
		: contDims({ cont_width, cont_height }), gridRect(grid_left, grid_top, grid_right, grid_bot)
	{
		width = gridRect.width() / cont_width;
		height = gridRect.height() / cont_height;
		count = width * height;

		cellsStartIndices = (int*)::operator new(count * sizeof(int));
		cellsEndIndices = (int*)::operator new(count * sizeof(int));

		reset_buffers();
	}
	~Grid() 
	{ 

	}

	void reset_buffers()
	{
		memset(cellsStartIndices, -1, sizeof(int) * count);
		memset(cellsEndIndices, -1, sizeof(int) * count);
	}

	sf::Vector2f relative_pos(const sf::Vector2f& position) const
	{
		return (position - sf::Vector2f(gridRect.top_left)) / sf::Vector2f(contDims);
	}

	int at_pos(const sf::Vector2f& position) const
	{
		return at_pos(sf::Vector2i(relative_pos(position)));
	}
	int at_pos(const sf::Vector2i& position) const
	{
		return at_pos(position.x, position.y);
	}
	int at_pos(int x, int y) const
	{
		if (!within_grid(x, y))
			return -1;

		return x + y * width;
	}

	Grid& operator=(Grid&& rhs) noexcept
	{
		std::swap(cellsStartIndices, rhs.cellsStartIndices);
		std::swap(cellsEndIndices, rhs.cellsEndIndices);
		std::swap(width, rhs.width);
		std::swap(height, rhs.height);
		std::swap(count, rhs.count);
		std::swap(contDims, rhs.contDims);
		std::swap(gridRect, rhs.gridRect);

		return *this;
	}

private:
	bool within_grid(int x, int y) const
	{
		return !(x < 0 || y < 0 || x >= width || y >= height);
	}

public:
	int* cellsStartIndices;
	int* cellsEndIndices;

	int width, height, count;

private:
	sf::Vector2i contDims;
	Rect_i gridRect;

private:
	Grid() = delete;
};

