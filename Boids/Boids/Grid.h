#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include "Container.h"
#include "Boid.h"
#include "VecUtil.h"

class Grid
{
public:
	Grid(int grid_left, int grid_top, int grid_right, int grid_bot, int cont_width, int cont_height);
	~Grid();

	inline Container<Boid>* at_pos(const sf::Vector2f& position) const
	{
		const sf::Vector2i& pos = ((sf::Vector2i)position - gridRect.top_left) / contDims;

		if (!within_grid(pos))
			return nullptr;

		return at_pos(pos);
	}
	inline Container<Boid>* at_pos(const Boid& item) const
	{
		const sf::Vector2i& pos = ((sf::Vector2i)item.get_origin() - gridRect.top_left) / contDims;

		if (!within_grid(pos))
			return nullptr;

		return at_pos(pos);
	}

	void insert(Boid& item) const;
	std::vector<const Container<Boid>*> query_containers(sf::Vector2f pos, float radius) const;

private:
	inline Container<Boid>* at_pos(int x, int y) const
	{
		return &containers[x + y * width];
	}
	inline Container<Boid>* at_pos(const sf::Vector2i& position) const
	{
		return &containers[position.x + position.y * width];
	}

	inline bool within_grid(const sf::Vector2i& pos) const
	{
		return !(pos.x < 0 || pos.y < 0 || pos.x >= width || pos.y >= height);
	}

private:
	Container<Boid>* containers;
	sf::Vector2i contDims;

	size_t width, height;
	Rect_i gridRect;

private:
	Grid() = delete;
	Grid(const Grid& rhs) = delete;
	Grid& operator=(const Grid& rhs) = delete;
};

