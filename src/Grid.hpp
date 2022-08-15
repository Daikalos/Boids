#pragma once

#include <SFML/Graphics.hpp>
#include <execution>

#include "VecUtil.h"
#include "Rectangle.hpp"

class Grid
{
public:
	Grid(RectInt grid, RectInt border, sf::Vector2i cont_dims)
		: grid_rect(grid), border_rect(border), cont_dims(cont_dims)
	{
		width = grid_rect.width() / cont_dims.x;
		height = grid_rect.height() / cont_dims.y;

		border_rect.top_left = (border_rect.top_left / cont_dims);
		border_rect.bot_right = (border_rect.bot_right / cont_dims);

		count = width * height;

		cells_start_indices = (int*)::operator new(count * sizeof(int));
		cells_end_indices = (int*)::operator new(count * sizeof(int));

		reset_buffers();
	}
	~Grid() = default;

	void reset_buffers()
	{
		memset(cells_start_indices, -1, sizeof(int) * count);
		memset(cells_end_indices, -1, sizeof(int) * count);
	}

	sf::Vector2f relative_pos(sf::Vector2f position) const
	{
		return (position - sf::Vector2f(grid_rect.top_left)) / sf::Vector2f(cont_dims);
	}

	int at_pos(sf::Vector2f position) const
	{
		return at_pos(sf::Vector2i(relative_pos(position)));
	}
	int at_pos(sf::Vector2i position) const
	{
		return at_pos(position.x, position.y);
	}
	int at_pos(int x, int y) const
	{
		return x + y * width;
	}

	sf::Vector2i at_pos(int i) const
	{
		return sf::Vector2i(i % width, i / width);
	}

	Grid& operator=(Grid&& rhs) noexcept
	{
		std::swap(cells_start_indices, rhs.cells_start_indices);
		std::swap(cells_end_indices, rhs.cells_end_indices);
		std::swap(width, rhs.width);
		std::swap(height, rhs.height);
		std::swap(count, rhs.count);
		std::swap(cont_dims, rhs.cont_dims);
		std::swap(grid_rect, rhs.grid_rect);
		std::swap(border_rect, rhs.border_rect);

		return *this;
	}

private:
	bool within_grid(int x, int y) const
	{
		return !(x < 0 || y < 0 || x >= width || y >= height);
	}

public:
	RectInt grid_rect;
	RectInt border_rect;
	sf::Vector2i cont_dims;

	int width, height, count, border_count;

	int* cells_start_indices;
	int* cells_end_indices;

private:
	Grid() = delete;
};

