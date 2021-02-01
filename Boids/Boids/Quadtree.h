#pragma once

#include <SFML/Graphics.hpp>
#include "Boid.h"

#ifndef RECT_I
#define RECT_I

struct rect_i
{
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

class Quadtree
{
public:
	Quadtree(const rect_i& rect, int capacity);
	virtual ~Quadtree();

	std::vector<Boid> query(const rect_i& rect) const;
	
	bool insert(const Boid& boid);

private:
	bool contains(const sf::Vector2f& point) const;
	bool intersects(const rect_i& rect) const;

	void subdivide();

private:
	const rect_i rectangle;

	Quadtree* northWest;
	Quadtree* northEast;
	Quadtree* southWest;
	Quadtree* southEast;

	std::vector<const Boid*> boids;

	int capacity;
	bool divided;

private:
	Quadtree() = delete;
	Quadtree(const Quadtree& rhs) = delete;
	Quadtree& operator=(const Quadtree& rhs) = delete;
};