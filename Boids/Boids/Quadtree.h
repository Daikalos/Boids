#pragma once

#include <SFML/Graphics.hpp>
#include "Boid.h"

class Quad
{
public:
	Quad(const sf::Vector2i& t_left, const sf::Vector2i& b_right, int capacity);
	~Quad();

	std::vector<Boid> query(const sf::Vector2i& t_left, const sf::Vector2i& b_right) const;
	
	bool insert(const Boid& boid);

private:
	bool contains(const sf::Vector2f& point) const;
	bool intersects(const sf::Vector2i& t_left, const sf::Vector2i& b_right) const;

	void subdivide();

private:
	const sf::Vector2i top_left;
	const sf::Vector2i bot_right;

	Quad* northWest;
	Quad* northEast;
	Quad* southWest;
	Quad* southEast;

	std::vector<const Boid*> boids;

	int capacity;
	bool divided;

private:
	Quad() = delete;
	Quad(const Quad& rhs) = delete;
	Quad& operator=(const Quad& rhs) = delete;
};

