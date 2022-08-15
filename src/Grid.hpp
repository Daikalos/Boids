#pragma once

#include <SFML/Graphics.hpp>
#include <execution>

#include "VecUtil.h"
#include "Rectangle.hpp"

class Grid
{
public:
	Grid(Config& config, RectFloat grid, sf::Vector2f container)
		: config(&config), grid_rect(grid), cont_dims(container)
	{
		grid_rect.top_left -= sf::Vector2f(config.boid_size_width, config.boid_size_height);
		grid_rect.bot_right += sf::Vector2f(config.boid_size_width, config.boid_size_height);

		float a = grid_rect.width() / this->cont_dims.x;
		float b = grid_rect.height() / this->cont_dims.y;

		cont_dims.x = grid_rect.width() / std::floorf(a);
		cont_dims.y = grid_rect.height() / std::floorf(b);

		width = grid_rect.width() / cont_dims.x;
		height = grid_rect.height() / cont_dims.y;

		count = width * height;

		cells_start_indices = (int*)::operator new(sizeof(int) * count);
		cells_end_indices = (int*)::operator new(sizeof(int) * count);

		reset_buffers();
	}

	void reset_buffers()
	{
		memset(cells_start_indices, -1, sizeof(int) * count);
		memset(cells_end_indices, -1, sizeof(int) * count);
	}

	sf::Vector2f relative_pos(sf::Vector2f position) const
	{
		return (position - grid_rect.top_left) / cont_dims;
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
		x = util::wrap(x, 0, width);
		y = util::wrap(y, 0, height);

		return x + y * width;
	}

	sf::Vector2i at_pos(int i) const
	{
		return sf::Vector2i(i % width, i / width);
	}

	Grid& operator=(Grid&& rhs) noexcept
	{
		std::swap(config, rhs.config);
		std::swap(grid_rect, rhs.grid_rect);
		std::swap(cont_dims, rhs.cont_dims);
		std::swap(width, rhs.width);
		std::swap(height, rhs.height);
		std::swap(count, rhs.count);
		std::swap(cells_start_indices, rhs.cells_start_indices);
		std::swap(cells_end_indices, rhs.cells_end_indices);

		return *this;
	}

public:
	Config* config;

	RectFloat grid_rect;
	sf::Vector2f cont_dims;

	int width, height, count, border_count;

	int* cells_start_indices;
	int* cells_end_indices;

private:
	Grid() = delete;
};

