#pragma once

#include <SFML/Graphics.hpp>

#include "../utilities/Rectangle.hpp"
#include "../utilities/VecUtil.h"

#include "Config.h"

class Grid
{
public:
	Grid() = default;

	Grid(Config& config, RectFloat rect, sf::Vector2f cont_dims)
		: _rect(rect), _cont_dims(cont_dims)
	{
		rect.top_left -= sf::Vector2f(config.boid_size_width, config.boid_size_height) / 2.0f;
		rect.bot_right += sf::Vector2f(config.boid_size_width, config.boid_size_height) / 2.0f;

		float a = rect.width() / _cont_dims.x;
		float b = rect.height() / _cont_dims.y;

		_cont_dims.x = rect.width() / std::floorf(a);
		_cont_dims.y = rect.height() / std::floorf(b);

		_width = rect.width() / _cont_dims.x;
		_height = rect.height() / _cont_dims.y;

		_count = _width * _height;

		_cells_start_indices = (int*)::operator new(sizeof(int) * _count);
		_cells_end_indices = (int*)::operator new(sizeof(int) * _count);

		reset_buffers();
	}

	void reset_buffers()
	{
		memset(_cells_start_indices, -1, sizeof(int) * _count);
		memset(_cells_end_indices, -1, sizeof(int) * _count);
	}

	sf::Vector2f relative_pos(sf::Vector2f position) const
	{
		return (position - _rect.top_left) / _cont_dims;
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
		x = util::wrap(x, 0, _width);
		y = util::wrap(y, 0, _height);

		return x + y * _width;
	}

	sf::Vector2i at_pos(int i) const
	{
		return sf::Vector2i(i % _width, i / _width);
	}

	Grid& operator=(Grid&& rhs) noexcept
	{
		std::swap(_rect, rhs._rect);
		std::swap(_cont_dims, rhs._cont_dims);
		std::swap(_width, rhs._width);
		std::swap(_height, rhs._height);
		std::swap(_count, rhs._count);
		std::swap(_cells_start_indices, rhs._cells_start_indices);
		std::swap(_cells_end_indices, rhs._cells_end_indices);

		return *this;
	}

public:
	RectFloat		_rect;
	sf::Vector2f	_cont_dims;

	int*			_cells_start_indices{nullptr};
	int*			_cells_end_indices{nullptr};

private:
	int _width{0}, _height{0}, _count{0};
};

