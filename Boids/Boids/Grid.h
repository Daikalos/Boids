#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <execution>

#include "Container.h"
#include "Boid.h"
#include "VecUtil.h"

class Grid
{
public:
	Grid(int grid_left, int grid_top, int grid_right, int grid_bot, int cont_width, int cont_height);
	~Grid();

	inline Container* at_pos(const sf::Vector2f& position) const
	{
		return at_pos(((sf::Vector2i)position - gridRect.top_left) / contDims);
	}
	inline Container* at_pos(const Boid& item) const
	{
		return at_pos(((sf::Vector2i)item.get_origin() - gridRect.top_left) / contDims);
	}

	void insert(Boid& item) const;

private:
	inline Container* at_pos(int x, int y) const
	{
		if (!within_grid(x, y))
			return nullptr;

		return &containers[x + y * width];
	}
	inline Container* at_pos(const sf::Vector2i& position) const
	{
		if (!within_grid(position))
			return nullptr;

		return &containers[position.x + position.y * width];
	}

	inline bool within_grid(int x, int y) const
	{
		return !(x < 0 || y < 0 || x >= width || y >= height);
	}
	inline bool within_grid(const sf::Vector2i& pos) const
	{
		return within_grid(pos.x, pos.y);
	}

private:
	Container* containers;
	sf::Vector2i contDims;

	size_t width, height;
	Rect_i gridRect;

private:
	Grid() = delete;
	Grid(const Grid& rhs) = delete;
	Grid& operator=(const Grid& rhs) = delete;
};

