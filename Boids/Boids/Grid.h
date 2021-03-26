#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_set>
#include "Boid.h"
#include "Vector2.h"

#ifndef RECT_I
#define RECT_I

struct rect_i
{
	rect_i() 
	{ 
		top_left = bot_right = top_right = bot_left = {};
	}
	rect_i(sf::Vector2i top_left, sf::Vector2i bot_right)
		: top_left(top_left), bot_right(bot_right)
	{
		top_right = { bot_right.x, top_left.y };
		bot_left = { top_left.x, bot_right.y };
	};

	inline int width() const { return (right - left); }
	inline int height() const { return (bot - top); }

	union
	{
		sf::Vector2i top_left;
		struct { int left, top; };
	};
	union
	{
		sf::Vector2i bot_right;
		struct { int right, bot; };
	};

	sf::Vector2i
		top_right,
		bot_left;
};

#endif

struct Container
{
	Container() { }
	Container(rect_i rect) : rect(rect) { }

	void insert(const Boid& boid)
	{
		if (boids.find(&boid) != boids.end())
			return;

		boids.insert(&boid);
	}
	void erase(const Boid& boid)
	{
		if (boids.find(&boid) == boids.end())
			return;

		boids.erase(&boid);
	}

	rect_i rect;
	std::unordered_set<const Boid*> boids;
};

class Grid
{
public:
	Grid(int cont_width, int cont_height, int width, int height);
	~Grid();

	inline Container* at_pos(int x, int y) const
	{
		return &containers[x + y * contDims.x];
	}
	inline Container* at_pos(const sf::Vector2i& position) const
	{
		return &containers[position.x + position.y * contDims.x];
	}
	inline Container* at_pos(const sf::Vector2f& position) const
	{
		const sf::Vector2i& pos = (sf::Vector2i)position / contDims;
		return &containers[pos.x + pos.y * contDims.x];
	}
	inline Container* at_pos(const Boid& boid) const
	{
		const sf::Vector2i& pos = (sf::Vector2i)boid.get_position() / contDims;
		return &containers[pos.x + pos.y * contDims.x];
	}

	void insert(const Boid& boid);

private:
	Container* containers;
	sf::Vector2i contDims;

private:
	Grid() = delete;
	Grid(const Grid& rhs) = delete;
	Grid& operator=(const Grid& rhs) = delete;
};

