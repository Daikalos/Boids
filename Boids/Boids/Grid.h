#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_set>
#include "Container.h"
#include "Boid.h"
#include "Vector2.h"

class Grid
{
public:
	Grid(int cont_width, int cont_height, int grid_width, int grid_height);
	~Grid();

	inline Container<Boid>* at_pos(int x, int y) const
	{
		return &containers[x + y * width];
	}
	inline Container<Boid>* at_pos(const sf::Vector2i& position) const
	{
		return &containers[position.x + position.y * width];
	}
	inline Container<Boid> at_pos(const sf::Vector2f& position) const
	{
		const sf::Vector2i& pos = (sf::Vector2i)position / contDims;

		if (!within_grid(pos))
			return Container<Boid>();

		return containers[pos.x + pos.y * width];
	}
	inline Container<Boid>* at_pos(const Boid& boid) const
	{
		const sf::Vector2i& pos = (sf::Vector2i)boid.get_position() / contDims;

		if (!within_grid(pos))
			return nullptr;

		return &containers[pos.x + pos.y * width];
	}

	inline bool within_grid(const sf::Vector2i& pos) const
	{
		return !(pos.x < 0 || pos.y < 0 || pos.x >= width || pos.y >= height);
	}

	void insert(Boid& boid);
	std::vector<Boid> query(sf::Vector2f pos, float radius);

private:
	Container<Boid>* containers;
	sf::Vector2i contDims;

	int width, height;

private:
	Grid() = delete;
	Grid(const Grid& rhs) = delete;
	Grid& operator=(const Grid& rhs) = delete;
};

