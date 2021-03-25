#pragma once

#include <SFML/Graphics.hpp>
#include "Boid.h"

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

	}

	rect_i rect;
	std::vector<const Boid*> boids;
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

	void insert(const Boid& boid);

private:
	Container* containers;
	sf::Vector2i contDims;

private:
	Grid() = delete;
	Grid(const Grid& rhs) = delete;
	Grid& operator=(const Grid& rhs) = delete;
};

