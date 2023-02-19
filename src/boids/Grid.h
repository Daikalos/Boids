#pragma once

#include <SFML/Graphics.hpp>

#include "../utilities/Rectangle.hpp"
#include "../utilities/VectorUtilities.h"

#include "Config.h"

class Grid
{
public:
	Grid() = default;
	Grid(const RectFloat& rect, const sf::Vector2f& cont_dims);

public:
	void reset_buffers();

	[[nodiscard]] constexpr sf::Vector2f relative_pos(const sf::Vector2f& position) const;
	[[nodiscard]] constexpr int at_pos(const sf::Vector2f& position) const;
	[[nodiscard]] constexpr int at_pos(const sf::Vector2i& position) const noexcept;
	[[nodiscard]] constexpr int at_pos(int x, int y) const noexcept;
	[[nodiscard]] constexpr sf::Vector2i at_pos(const int i) const;

public:
	RectFloat		_rect;
	sf::Vector2f	_cont_dims;

	std::vector<int> _cells_start_indices;
	std::vector<int> _cells_end_indices;

private:
	int _width{0}, _height{0}, _count{0};
};

constexpr sf::Vector2f Grid::relative_pos(const sf::Vector2f& position) const
{
	return (position - _rect.top_left) / _cont_dims;
}
constexpr int Grid::at_pos(const sf::Vector2f& position) const
{
	return at_pos(sf::Vector2i(relative_pos(position)));
}
constexpr int Grid::at_pos(const sf::Vector2i& position) const noexcept
{
	return at_pos(position.x, position.y);
}
constexpr int Grid::at_pos(int x, int y) const noexcept
{
	x = util::wrap(x, 0, _width);
	y = util::wrap(y, 0, _height);

	return x + y * _width;
}
constexpr sf::Vector2i Grid::at_pos(const int i) const
{
	return sf::Vector2i(i % _width, i / _width);
}

